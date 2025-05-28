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

OpsItem stack[STACK_SIZE]; // Стек
int sp = -1;  // Указатель вершины стека

//кладет элемент в стек.
void push(OpsItem item) {
    if (sp >= STACK_SIZE - 1) {
        err_no = ERR_STACK_OVERFLOW;
    }
    stack[++sp] = item;
}

//извлекает элемент из стека.
OpsItem pop() {
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

//выполняет одну из операций, в зависимости от кода op
void exec_op(int op, int *pc, OpsItem *ops) {
    OpsItem a, b, newitem;
    double ad, bd;
    int ai, bi;
    switch (op) {
        case 16: // =
            b = pop();
            a = pop();
            if (a.type <= 1 || a.type > 3) {
                printf("Ошибка: присваивание требует ссылку");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            if(a.type == 2){ // присвоение памяти динамическому массиву
                if(b.type != 4){
                    printf("Ошибка: только область памяти может быть присвоена массиву");
                    printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                    exit(1);
                }
                if(a.value != NULL){
                    free((void*)(long)(((int*)a.value)[0]));
                }
                int size = *(int*)b.value;
                ((int*)a.value)[0] = (int)(long)malloc(sizeof(double) * size);
                ((int*)a.value)[1] = sizeof(double);
                ((int*)a.value)[2] = size;

                double* init = (double*)(long)(((int*)a.value)[0]);
                for(int i = 0; i < size; i++){
                    init[i] = 0;
                }

            }
            else{
                if(b.type == 4){
                    printf("Ошибка: только переменные или константы могут быть присвоены переменной");
                    printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                    exit(1);
                }
                if(b.type == 6){
                    bd = (double)*(int*)b.value;
                }
                else{
                    bd = *(double*)b.value;
                }
                *(double*)a.value = bd;
            }


            break;
        case 25: // read
            a = pop();
            if (a.type != 3) {
                printf("Ошибка: read требует ссылку на переменную");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }
            printf("Введите значение для переменной: ");
            scanf("%lf", (double*)a.value);
            break;
        case 26: // write
            a = pop();
            if (a.type == 1 || a.type == 2 || a.type == 4) {
                printf("Ошибка: невозможно выполнить write с массивом");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }
            if(a.type == 6){
                ai = *(int*)a.value;
                printf("%d\n", ai);
            }
            else{
                ad = *(double*)a.value;
                printf("%g\n", ad);
            }

            break;
        case 36: // ind (индексация массива)
            b = pop(); // индекс
            a = pop(); // массив

            // Проверка типа массива
            if (a.type >= 3) {
                printf("Ошибка: индексация возможна только для массивов");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }
            // Проверка, что динамический массив инициализирован
            if (a.type == 2 && a.value == NULL) {
                printf("Ошибка: массив не инициализирован");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }
            // Проверка, что индекс — целое число
            if(b.type <= 2 || b.type == 4){
                printf("Ошибка: индекс должен быть целым числом");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            bi = floor(bd);
            if(bd - bi != 0){
                printf("Ошибка: индекс должен быть целым числом");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }
            // Проверка, что не происходит выхода за границу массива
            if(bi >= ((int*)a.value)[2] || bi < 0){
                printf("Ошибка: индекс за границей массива");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = ((int*)a.value)[1] * (bi) + ((int*)a.value)[0];
            newitem.type = 3;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);
            break;
        case 3: // +
            b = pop(); a = pop();
            // Проверка что a и b - переменные или константы
            if(a.type < 3 || b.type < 3 || a.type == 4 || b.type == 4){
                printf("Ошибка: операнды должны быть переменными или константами");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(double));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            *(double*)newitem.value = ad + bd;
            newitem.type = 5;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;

        case 4: // -
            b = pop(); a = pop();
            // Проверка что a и b - переменные или константы
            if(a.type < 3 || b.type < 3 || a.type == 4 || b.type == 4){
                printf("Ошибка: операнды должны быть переменными или константами");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(double));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            *(double*)newitem.value = ad - bd;
            newitem.type = 5;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);
            break;

        case 5: // *
            b = pop(); a = pop();
            // Проверка что a и b - переменные или константы
            if(a.type < 3 || b.type < 3 || a.type == 4 || b.type == 4){
                printf("Ошибка: операнды должны быть переменными или константами");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(double));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            *(double*)newitem.value = ad * bd;
            newitem.type = 5;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;

        case 6: // /
            b = pop(); a = pop();
            // Проверка что a и b - переменные или константы
            if(a.type < 3 || b.type < 3 || a.type == 4 || b.type == 4){
                printf("Ошибка: операнды должны быть переменными или константами");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(double));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            if (bd == 0) {
                printf("Ошибка: деление на ноль");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }
            *(double*)newitem.value = ad / bd;
            newitem.type = 5;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;

        case 33: // унарный минус
            a = pop();
            // Проверка что a - переменная или константа
            if(a.type < 3 || a.type == 4){
                printf("Ошибка: операнд должен быть переменной или константой");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(double));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            *(double*)newitem.value = -ad;
            newitem.type = 5;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;
        case 27: // sqrt
            a = pop();
            // Проверка что a - переменная или константа
            if(a.type < 3 || a.type == 4){
                printf("Ошибка: операнд должен быть переменной или константой");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(double));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if (ad < 0) {
                printf("Ошибка: sqrt от отрицательного числа");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }
            *(double*)newitem.value = taylor_sqrt(ad);
            newitem.type = 5;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);
            break;

        case 28: // exp
            a = pop();
            // Проверка что a - переменная или константа
            if(a.type < 3 || a.type == 4){
                printf("Ошибка: операнд должен быть переменной или константой");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(double));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            *(double*)newitem.value = taylor_exp(ad);
            newitem.type = 5;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;

        case 29: // log_b(a)
            b = pop(); a = pop();
            // Проверка что a и b - переменные или константы
            if(a.type < 3 || b.type < 3 || a.type == 4 || b.type == 4){
                printf("Ошибка: операнды должны быть переменными или константами");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(double));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            if (ad <= 0 || bd <= 0 || bd == 1.0) {
                printf("Ошибка: некорректные аргументы для log_b(a)");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }
            *(double*)newitem.value = taylor_ln(ad) / taylor_ln(bd);
            newitem.type = 5;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;

        case 21: // >=
            b = pop(); a = pop();
            // Проверка что a и b - переменные или константы
            if(a.type < 3 || b.type < 3 || a.type == 4 || b.type == 4){
                printf("Ошибка: операнды должны быть переменными или константами");
                printf("в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(int));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            *(int*)newitem.value = (ad >= bd);
            newitem.type = 6;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;

        case 17: // <
            b = pop(); a = pop();
            // Проверка что a и b - переменные или константы
            if(a.type < 3 || b.type < 3 || a.type == 4 || b.type == 4){
                printf("Ошибка: операнды должны быть переменными или константами");
                printf("в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(int));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            *(int*)newitem.value = (ad < bd);
            newitem.type = 6;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;

        case 18: // >
            b = pop(); a = pop();
            // Проверка что a и b - переменные или константы
            if(a.type < 3 || b.type < 3 || a.type == 4 || b.type == 4){
                printf("Ошибка: операнды должны быть переменными или константами");
                printf("в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(int));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            *(int*)newitem.value = (ad > bd);
            newitem.type = 6;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;
        case 19: // ==
            b = pop(); a = pop();
            // Проверка что a и b - переменные или константы
            if(a.type < 3 || b.type < 3 || a.type == 4 || b.type == 4){
                printf("Ошибка: операнды должны быть переменными или константами");
                printf("в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(int));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            *(int*)newitem.value = (ad == bd);
            newitem.type = 6;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;

        case 20: // <=
            b = pop(); a = pop();
            // Проверка что a и b - переменные или константы
            if(a.type < 3 || b.type < 3 || a.type == 4 || b.type == 4){
                printf("Ошибка: операнды должны быть переменными или константами");
                printf("в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(int));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            *(int*)newitem.value = (ad <= bd);
            newitem.type = 6;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;
        case 22: // !=
            b = pop(); a = pop();
            // Проверка что a и b - переменные или константы
            if(a.type < 3 || b.type < 3 || a.type == 4 || b.type == 4){
                printf("Ошибка: операнды должны быть переменными или константами");
                printf("в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            newitem.value = malloc(sizeof(int));
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            if(b.type == 6){
                bd = (double)*(int*)b.value;
            }
            else{
                bd = *(double*)b.value;
            }
            *(int*)newitem.value = (ad != bd);
            newitem.type = 6;
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;
        case 35: // jf
            b = pop(); a = pop();
            // Проверка, что b - целочисленная константа (метка перехода), a - переменная или константа
            if(b.type != 6 || a.type < 3 || a.type == 4){
                printf("Ошибка в интерпретации перехода");
                printf("в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            bi = *(int*)b.value;
            if (ad == 0){
                *pc = bi - 1;
            }
            break;

        case 34: // j
            a = pop();
            // Проверка, что a - целочисленная константа (метка перехода)
            if(a.type != 6){
                printf("Ошибка в интерпретации перехода");
                printf("в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }

            ai = *(int*)a.value;
            *pc = ai - 1;
            break;

        case 31: // new
            a = pop();

            // Проверка, что индекс — целое число
            if(a.type !=6 && a.type != 3){
                printf("Ошибка: индекс должен быть целым числом");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }
            if(a.type == 6){
                ad = (double)*(int*)a.value;
            }
            else{
                ad = *(double*)a.value;
            }
            ai = floor(ad);
            if(ad - ai != 0){
                printf("Ошибка: индекс должен быть целым числом");
                printf(" в строке %d, символ %d\n", a.line_no, a.char_no);
                exit(1);
            }
            newitem.value = malloc(sizeof(int));
            *(int*)newitem.value = ai;
            newitem.type = 4; // - память
            newitem.line_no = a.line_no;
            newitem.char_no = a.char_no;
            push(newitem);

            break;
        default:
            printf("Неизвестная операция: %d\n", op);
            exit(1);
    }
}

void err_print(){
    printf("\nERR %d at line %d char %d: ", err_no, line_no, char_no-_lexeme_length);
    err_codes_resolver();
}

void interpret(char *program) {
    OpsItem * ops = parse(program);
    if(ops == NULL){
        err_print();
        return;
    }

    // parse возвращает массив и в конце маркер с типом -1
    int pc = 0;
    while (ops[pc].type != -1) {
        OpsItem item = ops[pc];
        if (item.type >= 1 && item.type <= 6) {
            push(item);
        } else if (item.type == 0) { // операция
            exec_op(*(int*)item.value, &pc, ops);
        }
        pc++;
    }
}
