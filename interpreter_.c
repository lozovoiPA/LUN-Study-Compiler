#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "parser_.h"
#include "compiler_globals.h"

#define STACK_SIZE 100
#define MEM_SIZE 3000

//Макросы определяют размеры стека и области памяти.

typedef struct {
    int value;
    int type; // 0 - ссылка, 1 - значение
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

//возвращает реальное значение (если ссылка — берет из memory, иначе возвращает значение напрямую).
int resolve(OpsStackItem s) {
    return (s.type == 0) ? memory[s.value] : s.value;
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
            a = pop(); // базовый адрес
            push(a.value + resolve(b), 0);
            break;
        case 6: b = pop(); a = pop(); push(resolve(a) + resolve(b), 1); break;
        case 7: b = pop(); a = pop(); push(resolve(a) - resolve(b), 1); break;
        case 8: b = pop(); a = pop(); push(resolve(a) * resolve(b), 1); break;
        case 9:
        b = pop();
        a = pop();
        if (resolve(b) == 0) {
            printf("Ошибка: деление на ноль\n");
            exit(1);
        }
        push(resolve(a) / resolve(b), 1);
        break;
        case 10: a = pop(); push(-resolve(a), 1); break;
        case 13: b = pop(); a = pop(); push((int)(log(resolve(a)) / log(resolve(b))), 1); break;
        case 18: b = pop(); a = pop(); push(resolve(a) >= resolve(b), 1); break;
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

    int len = sizeof(ops)/sizeof(ops[0]);
    for (int pc = 0; pc < len; ++pc) {
        OpsItem item = ops[pc];
        if (item.type == 0 || item.type == 1) {
            push(item.value, item.type);
        } else if (item.type == 2) {
            exec_op(item.value, &pc, ops);
        }
    }
}
