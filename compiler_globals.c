#include "compiler_globals.h"

enum ERR_CODES err_no = 0;
int line_no = 1; int char_no = 0;

int n; double x, d; char *name;
char _out_st; int _out_tk_no;

# define KWRD_AMOUNT 14
# define OPER_AMOUNT 4
// Номер, с которого начинаются ключевые слова в списке лексем
const int KWRD_OFFSET = 19;
// Ключевые слова
const char* keywords[KWRD_AMOUNT] =
{
    "==", "<=", ">=", "!=",
    "if", "else",
    "read", "write",
    "sqrt", "exp", "log",
    "real", "new",
    "while"
};
const char* lexemes = " ak+-*/[](){},;z=<>";
const char* operations[OPER_AMOUNT] = { // Операции ОПС, которые обозначаются не лексемами
    "unary-", "j", "jf", "ind"
};

// список зарезервированных (ключевых) слов
int kwrd_resolver(char* tk){
    for(int i = 0; i < KWRD_AMOUNT; i++){
        if(!strcmp(tk, keywords[i])){
            return (i + KWRD_OFFSET);
        }
    }
    return 1; // tk не в списке зарезервированных слов
}

// Конвертация символической лексемы в ее номер
int tk_resolver(char* tk){
    int tk_len = strlen(tk);
    if(tk_len == 0){
        return 15;
    }
    if(tk_len == 1){
        switch(tk[0]){
            case 'a':   return 1;
            case 'k':   return 2;
            case '+':   return 3;
            case '-':   return 4;
            case '*':   return 5;
            case '/':   return 6;
            case '[':   return 7;
            case ']':   return 8;
            case '(':   return 9;
            case ')':   return 10;
            case '{':   return 11;
            case '}':   return 12;
            case ',':   return 13;
            case ';':   return 14;

            case '=':   return 16;
            case '<':   return 17;
            case '>':   return 18;
        }
    }
    tk_len = kwrd_resolver(tk);
    if(tk_len == 1){
        return 0;
    }
    return tk_len;
}

int oper_resolver(char* tk){
    int tk_no = tk_resolver(tk);
    if(!tk_no){
        for(int i = 0; i < OPER_AMOUNT; i++){
            if(!strcmp(tk, operations[i])){
                return (i + KWRD_OFFSET + KWRD_AMOUNT);
            }
        }
    }
    return tk_no;
}

// Конвертация номера в лексему (для выводов)
const char* tk_no_resolver(int tk_no){
    if(tk_no <= 0) return "\0";
    if(tk_no < KWRD_OFFSET){
        char* str = (char*)malloc(sizeof(char) * 2);
        str[0] = lexemes[tk_no];
        str[1] = 0;
        return str;
    }
    else if(tk_no < KWRD_OFFSET + KWRD_AMOUNT){
        return keywords[tk_no-KWRD_OFFSET];
    }
    return "\0";
}

const char* oper_no_resolver(int tk_no){
    const char* str = tk_no_resolver(tk_no);
    if(!strlen(str) && (tk_no > 0 && tk_no < (KWRD_OFFSET + KWRD_AMOUNT + OPER_AMOUNT))){
        return operations[tk_no - KWRD_OFFSET - KWRD_AMOUNT];
    }
    return str;
}

void err_throw(){
    _out_tk_no = 0;
    if(err_no == 0){
        err_no = ERR_UNKNOWN;
    }
}

char* IntToString(int num){
    int length = snprintf(NULL, 0,"%d",num);
    char* str = (char*)malloc(sizeof(char) * length);
    sprintf(str, "%d", num);
    return str;
}

int StringToInt(char* str){
    return (int) strtol(str, (char **)NULL, 10);
}

struct List* NewList() {
    struct TypedData tdata;
    tdata.data = NULL;
    tdata.type = -1;

    struct List* list = (struct List*)malloc(sizeof(struct List));
    list->tdata = tdata;
    list->next = NULL;
    list->prev = NULL;
    return list;
};
struct Stack* NewStack(){
    struct List* list = NewList();

    struct Stack* stack = (struct Stack*)malloc(sizeof(struct Stack));
    stack->top = list;

    return stack;
};
struct Queue* NewQueue(){
    struct List* list = NewList();
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->front = list;

    return queue;
}

void ListDispose(struct List* list){
    void* nextEl;
    while(list->next != NULL){
        nextEl = list->next;
        free(list->tdata.data);
        free(list);
    }
    free(nextEl);
}
void StackDispose(struct Stack* stack){
    ListDispose(stack->top);
    free(stack);
}
void QueueDispose(struct Queue* queue){
    ListDispose(queue->front);
    free(queue);
}

int IsEmpty(struct List list){
    //printf("%d %d\n", list.tdata.data, list.tdata.type);
    if ((list.tdata.data == NULL) && (list.tdata.type == -1) && (list.next == NULL) && (list.prev == NULL)){
        return 1;
    }
    return 0;
}
struct List* Append(struct List *list, struct TypedData tdata){
    if(IsEmpty(*list)){
        list->tdata = tdata;
        return list;
    }
    else{
        while(list->next != NULL){
            list = list->next;
        }
        struct List* newList = NewList();
        list->next = newList;
        list->next->tdata = tdata;
        list->next->prev = list;


        return list->next;
    }
}
struct TypedData RemoveLast(struct List* list){
    struct TypedData tdata;
    tdata.data = NULL; tdata.type = -1;
    if(!IsEmpty(*list)){
        if(list->next == NULL && list->prev == NULL){
            tdata = list->tdata;
            list->tdata.data = NULL;
            list->tdata.type = -1;
        }
        else{
            list = Tail(list);
            tdata = list->tdata;
            list->prev->next = NULL;
            free(list);
        }
    }
    return tdata;
};
struct TypedData RemoveFirst(struct List* list){
    struct TypedData tdata;
    tdata.data = NULL; tdata.type = -1;
    if(!IsEmpty(*list)){
        if(list->next == NULL && list->prev == NULL){
            tdata = list->tdata;
            list->tdata.data = NULL;
            list->tdata.type = -1;
        }
        else{
            tdata = list->tdata;
            list->next->prev = NULL;
            free(list);
        }
    }
    return tdata;
}

struct List* Tail(struct List* list){
    while(list->next != NULL){
        list = list->next;
    }
    return list;
};
void PrintList(struct List list){
    if(IsEmpty(list)){
        printf("list is empty\n");
    }
    else{
        printf("(%p, %d)\n", list.tdata.data, list.tdata.type);
        while(list.next != NULL){
            list = *(list.next);
            printf("(%p, %d)\n", list.tdata.data, list.tdata.type);
        }
    }
}

void PrintListBackwards(struct List list){
    if(IsEmpty(list)){
        printf("list is empty\n");
    }
    else{
        printf("(%p, %d)\n", list.tdata.data, list.tdata.type);
        while(list.prev != NULL){
            list = *(list.prev);
            printf("(%p, %d)\n", list.tdata.data, list.tdata.type);
        }
    }
}

struct Stack* Push(struct Stack *stack, struct TypedData tdata){
    stack->top = Append(stack->top, tdata);
    return stack;
}

struct TypedData Pop(struct Stack *stack){
    struct List* removeAt = stack->top;
    if(stack->top->prev != NULL){
        stack->top = stack->top->prev;
    }
    return RemoveLast(removeAt);
};

struct Queue* QueuePush(struct Queue* queue, struct TypedData tdata){
    Append(queue->front, tdata);
    return queue;
}

struct TypedData QueuePop(struct Queue* queue){
    struct List* removeAt = queue->front;
    if(queue->front->next != NULL){
        queue->front = queue->front->next;
    }
    return RemoveFirst(removeAt);
}
