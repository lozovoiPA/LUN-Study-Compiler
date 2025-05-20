#include "parser_.h"
#include "compiler_globals.h"

#include "parser_semantic_programs.h"
int main()
{
    // тестовые строки

    char str12[] =
    "real a, b = 12.4 + 1, c = 10;\na = b - c;\nwrite(a);"; // объ€вление real
    char str13[] =
    "real[4] a, b, c;\na[0] = 10;\nwrite(a[0]);"; // объ€вление real[E]
    char str14[] =
    "real[*] a, b = new[5], c;\nb[0] = 10;\nwrite(b[0]);"; // объ€вление real[*]

    char str15[] =
    "if(6) {\nwrite(10);\n} \nwrite(100);"; // простое условие
    char str16[] =
    "if(6 < 12) {\nwrite(10);\n} \nelse { \nwrite(10 + 2); \n} \nwrite(100);"; // if-else
    char str17[] =
    "if(6 < 12 + 343) {\nwrite(10);\n} \nelse if(6 + 3 != 25) { \nwrite(10 + 2); \n} \nwrite(100);"; // if-else if
    char str18[] =
    "if(6 < 12 + 343) {\nwrite(10);\n} \nelse if(6 + 3 != 25) { \nwrite(10 + 2); \n} \nelse { \nwrite(10 + 2); \n} \nwrite(100);"; // if-elseif-else

    char str19[] =
    "real i = 10;\nwhile(i > 0) {\nwrite(78 - i); \ni = i - 1; \n} \nwrite(i); \ni = 10; "; // while

    char str20[] =
    "if 10 \n i = 25;"; // error
    char str21[] =
    "real[4.5] a, b, c;\na[0] = 10;\nwrite(a[0]);"; // error
    char str22[] =
    "real[4] a, b, c;\na[0] = 10;\nwrrtweke(a[0]);"; // error
    char str23[] =
    "real[4] a, b, c;\na(0) = 10;\nwrrtweke(a[0]);"; // более сложные ошибки завис€т от пары терминал-нетерминал

    // помен€йте указатель на какую-либо из тестовых строк, чтобы ее проанализировать
    char* str = str19;
    printf("%s\n", str);
    int str_len = strlen(str)+1;

    /*
    char test_ops1[4096];
    char* write_to;
    write_to = test_ops1;
    for(int i = 0; i<10; i++){
        write_to += sprintf(write_to, "%d ", i) * sizeof(char);
    }
    printf("%s\n", test_ops1);

    VariableTable_init();

    VariableTable_add("test_ops1", test_ops1, 1);
    VariableTable_print();
    printf("\n\n%s", (char*)(VariableTable_find("test_ops1")->address));*/

    /*
    struct Queue* q = NewQueue();
    struct TypedData td;
    for(int i = 0; i < 10; i++){
        td.data = malloc(sizeof(int));
        *(int*)td.data = i;
        td.type = 0;
        QueuePush(q, td);
        PrintList(*q->front);
        printf("\n");
    }
    printf("\n============\n");
    for(int i = 0; i < 10; i++){
        td = QueuePop(q);
        printf("%d\n\n", *(int*)td.data);
        PrintList(*q->front);
        printf("\n");
    }*/

    parser(str, str_len);
    return 0;
}

