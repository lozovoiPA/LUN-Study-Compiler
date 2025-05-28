#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "parser_.h"
#include "compiler_globals.h"

#define STACK_SIZE 100
#define MEM_SIZE 3000

//������� �������� ��� ���������� �������������� �������
const double eps = 1e-9;

//������� ���������� ������� ����� � ������� ������.

OpsItem stack[STACK_SIZE]; // ����
int sp = -1;  // ��������� ������� �����

//������ ������� � ����.
void push(OpsItem item) {
    if (sp >= STACK_SIZE - 1) {
        err_no = ERR_STACK_OVERFLOW;
    }
    stack[++sp] = item;
}

//��������� ������� �� �����.
OpsItem pop() {
    if (sp < 0) {
        printf("������: ���� ����\n");
        exit(1);
    }
    return stack[sp--];
}

//������ �����
double my_abs(double x) {
    return (x < 0) ? -x : x;
}

//���������� �� ���� ���������
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

//�������� �� ���� ���������
double taylor_ln(double x) {
    if (x <= 0) {
        printf("������: log �� ���������������� �����\n");
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

//������ �� ������ ������� (��������)
double taylor_sqrt(double x) {
    if (x < 0) {
        printf("������: sqrt �� �������������� �����\n");
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

//���������� �������� �������� (���� ������ � ����� �� memory, ����� ���������� �������� ��������).
double resolve(OpsItem s) {
    switch (s.type) {
        case 1: case 2: case 3:
            //return memory[s.value];
            return 0;
        case 5: case 6:
            //return s.value;
            return 0;
        default:
            printf("������: ������������ ��� ��� resolve: %d\n", s.type);
            exit(1);
    }
}
//��������� ���� �� ��������, � ����������� �� ���� op
void exec_op(int op, int *pc, OpsItem *ops) {
    OpsItem a, b;
    switch (op) {
        case 2: // =
            b = pop();
            a = pop();
            if (a.type != 0) {
                printf("������: ������������ ������� ������\n");
                exit(1);
            }
            //memory[a.value] = resolve(b);
            break;
        case 3: // read
            a = pop();
            if (a.type != 0) {
                printf("������: read ������� ������\n");
                exit(1);
            }
            printf("������� �������� ��� ����������: ");
            //scanf("%d", &memory[a.value]);
            break;
        case 4: // write
            a = pop();
            printf("%d\n", resolve(a));
            break;
        case 5: // ind (���������� �������)
            b = pop(); // ������
            a = pop(); // ������

            // �������� ���� �������
            if (a.type != 1 && a.type != 2) {
                printf("������: ���������� �������� ������ ��� ��������, �������� ��� %d\n", a.type);
                exit(1);
            }

            // ��������, ��� ������ � ����� �����
            double index_val = resolve(b);
            if (floor(index_val) != index_val) {
                printf("������: ������ ������ ���� ����� ������, �������� %f\n", index_val);
                exit(1);
            }

            //push(a.value + (int)index_val, 1); // ������� ����� � ���������� ������
            break;
        case 6: // +
            b = pop(); a = pop();
            //push(resolve(a) + resolve(b), 5); // ������������, ��� ��������� � real
            break;

        case 7: // -
            b = pop(); a = pop();
            //push(resolve(a) - resolve(b), 5);
            break;

        case 8: // *
            b = pop(); a = pop();
            //push(resolve(a) * resolve(b), 5);
            break;

        case 9: // /
            b = pop(); a = pop();
            if (resolve(b) == 0) {
                printf("������: ������� �� ����\n");
                exit(1);
            }
            //push(resolve(a) / resolve(b), 5);
            break;

        case 10: // ������� �����
            a = pop();
            //push(-resolve(a), a.type);
            break;
        case 11: // sqrt
            a = pop();
            if (resolve(a) < 0) {
                printf("������: sqrt �� �������������� �����\n");
                exit(1);
            }
            //push(taylor_sqrt(resolve(a)), 5); // ���������� real
            break;

        case 12: // exp
            a = pop();
            //push(taylor_exp(resolve(a)), 5);
            break;

        case 13: // log_b(a)
            b = pop(); // ���������
            a = pop(); // ��������
            if (resolve(a) <= 0 || resolve(b) <= 0 || resolve(b) == 1.0) {
                printf("������: ������������ ��������� ��� log_b(a): a=%f, b=%f\n", resolve(a), resolve(b));
                exit(1);
            }
            //push(taylor_ln(resolve(a)) / taylor_ln(resolve(b)), 5);
            break;

        case 18: // >=
            b = pop(); a = pop();
            //push(resolve(a) >= resolve(b), 6); // ���������� �������� � �����
            break;
        case 20: // jf
            a = pop();
            if (!resolve(a)) *pc = ops[*pc + 1].value - 1;
            (*pc)++; // ���������� �����
            break;
        case 21: // j
            *pc = ops[*pc + 1].value - 1;
            (*pc)++; // ���������� �����
            break;
        default:
            printf("����������� ��������: %d\n", op);
            exit(1);
    }
}

void err_print(){
    printf("\nERR %d at line %d: ", err_no, line_no);
    err_codes_resolver();
}

void interpret(char *program) {
    OpsItem * ops = parse(program);
    if(ops == NULL){
        err_print();
        return;
    }

    // parse ���������� ������ � � ����� ������ � ����� -1
    int pc = 0;
    while (ops[pc].type != -1) {
        OpsItem item = ops[pc];
        if (item.type >= 1 && item.type <= 6) {
            push(item);
        } else if (item.type == 0) { // ��������
            exec_op(*(int*)item.value, &pc, ops);
        }
        pc++;
    }
}
