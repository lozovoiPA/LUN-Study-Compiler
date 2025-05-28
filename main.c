#include "parser_.h"

int main()
{
    // тестовые строки

    char str12[] =
    "real a, b = 12.4 + 1, c = 10;\na = b - c;\nwrite(a);"; // объявление real
    char str13[] =
    "real[4] a, b, c;\na[0] = 10;\nwrite(a[0]);"; // объявление real[E]
    char str14[] =
    "real[*] a, b = new[5], c;\nb[0] = 10;\nwrite(b[0]);"; // объявление real[*]

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
    "real[4] a, b, c;\na(0) = 10;\nwrrtweke(a[0]);"; // более сложные ошибки зависят от пары терминал-нетерминал

    // поменяйте указатель на какую-либо из тестовых строк, чтобы ее проанализировать
    char* str = str18;
    printf("%s\n", str);

    parse(str);

    return 0;
}

