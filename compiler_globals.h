#ifndef COMPILER_GLOBAL_H_INCLUDED
#define COMPILER_GLOBAL_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Задаем размер таблицы ASCII
#define ASCII_W 128

// Сокращение имени функции
#define token tk_resolver

// Возможные коды ошибок
enum ERR_CODES
{
    ERR_UNKNOWN = -1,
    ERR_ID_TOO_LARGE = 1,
    ERR_UNKNOWN_ID = 2,
    ERR_EXISTING_ID = 3,
    ERR_IN_VARIABLES_STACK = 4,
    ERR_INDEX_NOT_INTEGER = 5,
    ERR_LEXEME_EXPECTED = 6,
    ERR_UNKNOWN_LEXEME = 7
};

// Максимальная длина идентификатора
#define MAX_ID_LEN 64
extern enum ERR_CODES err_no;
// Номера строки и символа в ней
extern int line_no; extern int char_no;

// Переменные, использующиеся при вычислении чисел/идентификаторов
extern int n; extern double x, d;
extern char *name;

// Вывод лексического анализатора записывается в глобальные переменные (чтобы к ним имели доступ все подпрограммы)
extern char _out_st; extern int _out_tk_no;
extern int _error_lexeme;

void err_codes_resolver();

// Структуры данных
// смысл типа данных зависит от того, где используется стек
struct TypedData {
    void* data;
    int type;
};

struct List{
    struct TypedData tdata;
    struct List* next;
    struct List* prev;
};

struct Stack{
    struct List* top;
};

// Элемент ОПС
typedef struct {
    int type;   // 0 - ссылка на память, 1 - константа, 2 - операция
    int value;  // значение или код операции
} OpsItem;

// Функции для работы со структурами данных
struct List* NewList();
struct Stack* NewStack();
void ListDispose(struct List*);
void StackDispose(struct Stack*);

int IsEmpty(struct List);
struct List* Append(struct List*, struct TypedData);
struct TypedData RemoveLast(struct List*);
struct TypedData RemoveFirst(struct List*);
struct List* Tail(struct List*);
void PrintList(struct List);
void PrintListBackwards(struct List);

struct Stack* Push(struct Stack*, struct TypedData);
struct TypedData Pop(struct Stack*);

char* IntToString(int num);
int StringToInt(char* str);

const char* tk_no_resolver(int tk_no);
const char* oper_no_resolver(int tk_no);
int kwrd_resolver(char*);
int tk_resolver(char*);
int oper_resolver(char*);
void err_throw();



#endif // COMPILER_GLOBAL_H_INCLUDED
