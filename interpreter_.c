#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "parser_.h"
#include "compiler_globals.h"

#define STACK_SIZE 100
#define MEM_SIZE 3000

//������� ���������� ������� ����� � ������� ������.

typedef struct {
    int value;
    int type; // 0 - ������, 1 - ��������
} OpsStackItem;

int memory[MEM_SIZE]; // ������ ��������������
OpsStackItem stack[STACK_SIZE]; // ����
int sp = -1;  // ��������� ������� �����

//������ ������� � ����.
void push(int value, int type) {
    if (sp >= STACK_SIZE - 1) {
        printf("������: ���� ����������\n");
        //exit(1);
    }
    stack[++sp] = (OpsStackItem){value, type};
}

//��������� ������� �� �����.
OpsStackItem pop() {
    if (sp < 0) {
        printf("������: ���� ����\n");
        exit(1);
    }
    return stack[sp--];
}

//���������� �������� �������� (���� ������ � ����� �� memory, ����� ���������� �������� ��������).
int resolve(OpsStackItem s) {
    return (s.type == 0) ? memory[s.value] : s.value;
}

//��������� ���� �� ��������, � ����������� �� ���� op
void exec_op(int op, int *pc, OpsItem *ops) {
    OpsStackItem a, b;
    switch (op) {
        case 2: // =
            b = pop();
            a = pop();
            if (a.type != 0) {
                printf("������: ������������ ������� ������\n");
                exit(1);
            }
            memory[a.value] = resolve(b);
            break;
        case 3: // read
            a = pop();
            if (a.type != 0) {
                printf("������: read ������� ������\n");
                exit(1);
            }
            printf("������� �������� ��� ����������: ");
            scanf("%d", &memory[a.value]);
            break;
        case 4: // write
            a = pop();
            printf("%d\n", resolve(a));
            break;
        case 5: // ind (���������� �������)
            b = pop(); // ������
            a = pop(); // ������� �����
            push(a.value + resolve(b), 0);
            break;
        case 6: b = pop(); a = pop(); push(resolve(a) + resolve(b), 1); break;
        case 7: b = pop(); a = pop(); push(resolve(a) - resolve(b), 1); break;
        case 8: b = pop(); a = pop(); push(resolve(a) * resolve(b), 1); break;
        case 9:
        b = pop();
        a = pop();
        if (resolve(b) == 0) {
            printf("������: ������� �� ����\n");
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
