#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "parser_.h"
#include "compiler_globals.h"

#define STACK_SIZE 100
#define MEM_SIZE 3000

//эпсилон значение для вычисление математических функций
const double eps = 1e-9;

//Макросы определяют размеры стека и области памяти.

typedef struct {
    int value;
    int type; 
    // 0 - операция
    // 1 - ссылка на статический массив
    // 2 - ссылка на динамический массив
    // 3 - ссылка на переменную real
    // 5 - константа real
    // 6 - константа integer
} OpsStackItem;

int memory[MEM_SIZE]; // Память интерпретатора
OpsStackItem stack[STACK_SIZE]; // Стек
int sp = -1;  // Указатель вершины стека

//кладет элемент в стек.
void push(int value, int type) {
    if (sp >= STACK_SIZE - 1) {
        printf("Ошибка: стек переполнен\n");
        //exit(1);
    }
    stack[++sp] = (OpsStackItem){value, type};
}

//извлекает элемент из стека.
OpsStackItem pop() {
    if (sp < 0) {
        printf("Ошибка: стек пуст\n");
        exit(1);
    }
    return stack[sp--];
}

//Модуль числа
double my_abs(double x) {
    return (x < 0) ? -x : x;
}

//Экспонента по ряду Макларена
double taylor_exp(double x) {
    double term = 1.0;
    double result = 1.0;
    int i = 1;
    while (my_abs(term) > eps) {
        term *= x / i++;
        result += term;
    }
    return result;
}

//Логарифм по ряду арксинуса
double taylor_ln(double x) {
    if (x <= 0) {
        printf("Ошибка: log от неположительного числа\n");
        exit(1);
    }
    if (x == 1.0) return 0.0;

    double y = (x - 1) / (x + 1);
    double y2 = y * y;
    double term = y;
    double result = 0;
    int n = 1;

    while (my_abs(term / n) > eps) {
        result += term / n;
        term *= y2;
        n += 2;
    }

    return 2 * result;
}

//Корень по методу Ньютона (итераций)
double taylor_sqrt(double x) {
    if (x < 0) {
        printf("Ошибка: sqrt от отрицательного числа\n");
        exit(1);
    }
    if (x == 0) return 0;

    double guess = x / 2.0;
    double next = 0.5 * (guess + x / guess);

    while (my_abs(next - guess) > eps) {
        guess = next;
        next = 0.5 * (guess + x / guess);
    }

    return next;
}

//возвращает реальное значение (если ссылка — берет из memory, иначе возвращает значение напрямую).
double resolve(OpsStackItem s) {
    if (s.type == 1 || s.type == 2 || s.type == 3) return memory[s.value]; // ссылки
    if (s.type == 5 || s.type == 6) return s.value; // константы
    printf("Ошибка: недопустимый тип для resolve: %d\n", s.type);
    exit(1);
}

//выполняет одну из операций, в зависимости от кода op
void exec_op(int op, int *pc, OpsItem *ops) {
    OpsStackItem a, b;
    switch (op) {
        case 2: // =
            b = pop();
            a = pop();
            if (a.type != 0) {
                printf("Ошибка: присваивание требует ссылку\n");
                exit(1);
            }
            memory[a.value] = resolve(b);
            break;
        case 3: // read
            a = pop();
            if (a.type != 0) {
                printf("Ошибка: read требует ссылку\n");
                exit(1);
            }
            printf("Введите значение для переменной: ");
            scanf("%d", &memory[a.value]);
            break;
        case 4: // write
            a = pop();
            printf("%d\n", resolve(a));
            break;
        case 5: // ind (индексация массива)
            b = pop(); // индекс
            a = pop(); // массив
            
            // Проверка типов
            if (b.type != 6) {
                printf("Ошибка: индекс должен быть целым числом, получено тип %d\n", b.type);
                exit(1);
            }
            if (a.type != 1 && a.type != 2) {
                printf("Ошибка: индексация возможна только для массивов, получено тип %d\n", a.type);
                exit(1);
            }
            push(a.value + resolve(b), 1); // считаем адрес и возвращаем ссылку
            break;
        case 6: // +
            b = pop(); a = pop();
            push(resolve(a) + resolve(b), 5); // предполагаем, что результат — real
            break;

        case 7: // -
            b = pop(); a = pop();
            push(resolve(a) - resolve(b), 5);
            break;

        case 8: // *
            b = pop(); a = pop();
            push(resolve(a) * resolve(b), 5);
            break;

        case 9: // /
            b = pop(); a = pop();
            if (resolve(b) == 0) {
                printf("Ошибка: деление на ноль\n");
                exit(1);
            }
            push(resolve(a) / resolve(b), 5);
            break;

        case 10: // унарный минус
            a = pop();
            push(-resolve(a), a.type);
            break;
        case 11: // sqrt
            a = pop();
            if (resolve(a) < 0) {
                printf("Ошибка: sqrt от отрицательного числа\n");
                exit(1);
            }
            push(taylor_sqrt(resolve(a)), 5); // возвращаем real
            break;

        case 12: // exp
            a = pop();
            push(taylor_exp(resolve(a)), 5);
            break;

        case 13: // log_b(a)
            b = pop(); // основание
            a = pop(); // аргумент
            if (resolve(a) <= 0 || resolve(b) <= 0 || resolve(b) == 1.0) {
                printf("Ошибка: некорректные аргументы для log_b(a): a=%f, b=%f\n", resolve(a), resolve(b));
                exit(1);
            }
            push(taylor_ln(resolve(a)) / taylor_ln(resolve(b)), 5);
            break;

        case 18: // >=
            b = pop(); a = pop();
            push(resolve(a) >= resolve(b), 6); // логическое значение — целое
            break;
        case 20: // jf
            a = pop();
            if (!resolve(a)) *pc = ops[*pc + 1].value - 1;
            (*pc)++; // пропустить адрес
            break;
        case 21: // j
            *pc = ops[*pc + 1].value - 1;
            (*pc)++; // пропустить адрес
            break;
        default:
            printf("Неизвестная операция: %d\n", op);
            exit(1);
    }
}

void interpret(char *program) {
    OpsItem * ops = parse(program);

    // Предположим, parse возвращает массив и в конце маркер с типом -1
    int pc = 0;
    while (ops[pc].type != -1) {
        OpsItem item = ops[pc];
        if (item.type >= 1 && item.type <= 6) {
            push(item.value, item.type);
        } else if (item.type == 0) { // операция
            exec_op(item.value, &pc, ops);
        }
        pc++;
    }
}