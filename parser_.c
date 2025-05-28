#include "tokenizer_.h"
#include "compiler_globals.h"
#include "parser_semantic_programs.h"

#define lexeme NewLexeme
#define nterm NewNonterminal
struct TypedData NewLexeme(char* lx);
struct TypedData NewNonterminal(char st, int ind);

int parser_nt_resolver(char nt_ch, int ind);
void print_rule(struct List* list);
void print_rule_bk(struct List* list);
void print_action(struct List* list);
void print_action_bk(struct List* list);
void print_tables();
void parser_init();
int use_rule(int lx_no);
int use_action(struct TypedData tdata);
void fill_gen(int nt_no, char* rule[], char* act[]);
struct List* gen_symbols(char* rule[], int len);
struct List* gen_actions(char* act[], int len);
void parser_dispose();

void print_variable(struct TypedData);
void print_variables();
void push_variable();

#define RULES_W 32 // Число лексем (столбцов в таблице LL(1)-анализатора)
#define RULES_H 20 // Число состояний (строк в таблице LL(1)-анализатора)
struct Stack* magazine = NULL; // магазин анализатора
struct Stack* act_magazine = NULL; // магазин генератора (действий по генерации ОПС)
struct Stack* variables = NULL; // стек для хранения констант/переменных (при генерации ОПС, т.к. name, n, x лекс. анализатора затираются)
struct List* lambda; // лямбда - пустой список (инициалицируется в parse_init(), также см. функцию IsEmpty в compiler_global.c), ошибка (пустая клетка в таблице LL(1)-анализатора - NULL)
//таблицы для генерации символов (терминалов и нетерминалов) в магазин анализатора
// и действий в магазин генератора
struct List* rules_gen_symbols[RULES_H][RULES_W];
struct List* rules_gen_actions[RULES_H][RULES_W];

char parser_sts[] = "P  JO XREUTVG MLHCQZ";

char test_ops[4096];
char* write_to;

struct List* ops = NULL; // ОПС в виде списка
int open_table_type = 0; // Тип объявляемых переменных (см. else в use_action())
struct Stack* labels_ops = NULL; // стек меток
struct Stack* labels_test_ops = NULL; // стек меток для тестовой ОПС (выводится в консоль в виде строки)
int ops_els = 0; // Число элементов ОПС

OpsItem* parse(char* str){
    parser_init();
    //print_tables();

    int str_len = strlen(str)+1;

    int res;
    for(int i = 0; (i < str_len) && !err_no;){
        i = tokenizer(str, i); // Получить лексему
        if(!err_no){
            if(_out_tk_no == 2 || _out_tk_no == 1){
                push_variable(); // Запомнить имя переменой или значение константы
            }
            res = use_rule(_out_tk_no); // Применить правило (генерация символов в оба стека здесь!)
        }
    }
    if(err_no){
        printf("\nERR %d at line %d: ", err_no, line_no);
        err_codes_resolver();
    } else{
        //printf("\n\n%s", test_ops);
        PrintList(*ops);
    }
    //VariableTable_print();
    parser_dispose();
}

void use_rule_print(int lx_no){
    printf("%s\t", tk_no_resolver(lx_no));
    print_rule_bk(magazine->top);
    printf("  |  ");
    print_variables();
    printf("\n\t");
    print_action_bk(act_magazine->top);
    //printf("\n");
}
// Функция, применяющая правила (рекурсивно, пока не будет удалена лексема или не установлена ошибка)
// Она же проверяет на ошибки.
int use_rule(int lx_no){
    // Выводы для тестов
    use_rule_print(lx_no);

    // Берем символ из магазина
    struct TypedData tdata = Pop(magazine);
    struct TypedData act_tdata = Pop(act_magazine);

    int dt = *(int*) tdata.data;
    free(tdata.data);
    if(!use_action(act_tdata)){ //Выполняется действие из стека генератора
        err_throw();

        free(act_tdata.data);

        return 0;
    }
    free(act_tdata.data);

    printf("  |  %s", test_ops);
    printf("\n\n");
    if(tdata.type == 0){ // Лексема
        if(dt != lx_no){
            err_no = ERR_LEXEME_EXPECTED;
            _error_lexeme = dt;
            err_throw();
            return 0;
        }
        else{
            return 1;
        }
    }
    else{ // Нетерминал
        struct List* gen_s = rules_gen_symbols[dt][lx_no-1];
        if(gen_s == NULL){ // Ошибка
            err_throw();
            return 0;
        }
        else if(gen_s == lambda){ // Если лямбда, то в стек ничего не добавляется
            return use_rule(lx_no);
        }
        else{ // Иначе добавляем правую часть (в обратном порядке!) в магазины
            struct TypedData new_data;

            gen_s = Tail(gen_s);

            new_data.data = malloc(sizeof(int));
            *(int*)new_data.data = *(int*)gen_s->tdata.data;
            new_data.type = gen_s->tdata.type;
            Push(magazine, new_data);
            while(gen_s->prev != NULL){
                gen_s = gen_s->prev;

                new_data.data = malloc(sizeof(int));
                *(int*)new_data.data = *(int*)gen_s->tdata.data;
                new_data.type = gen_s->tdata.type;
                Push(magazine, new_data);
            }

            gen_s = Tail(rules_gen_actions[dt][lx_no-1]);
            new_data.data = malloc(sizeof(int));
            *(int*)new_data.data = *(int*)gen_s->tdata.data;
            new_data.type = gen_s->tdata.type;
            Push(act_magazine, new_data);
            while(gen_s->prev != NULL){
                gen_s = gen_s->prev;

                new_data.data = malloc(sizeof(int));
                *(int*)new_data.data = *(int*)gen_s->tdata.data;
                new_data.type = gen_s->tdata.type;
                Push(act_magazine, new_data);
            }
            return use_rule(lx_no);
        }
    }
}

int use_action(struct TypedData tdata){
    if(tdata.type == 1){ // Действие - внести ссылку в ОПС. Необходимо найти ее по имени переменной
        struct TypedData tdata = Pop(variables);
        struct Variable* vt = (struct Variable*)tdata.data; // Получаем идентификатор из стека
        if(vt->type != 0){
            err_no = ERR_IN_VARIABLES_STACK;
            err_throw();
            return 0;
        }
        struct Variable* v = VariableTable_find(vt->name);
        free(vt);
        if(v == NULL){
            err_no = ERR_UNKNOWN_ID;
            err_throw();
            return 0;
        }
        write_to += sprintf(write_to, "%s ", v->name) * sizeof(char);

        struct TypedData td;
        td.data = v->address;
        td.type = v->type;
        ListAppend(ops, td);
        //printf("%d\n", (int)(v->address));

        //QueuePush(ops, td);

        ops_els++;
        return 1;
    }
    else if(tdata.type == 2){ // Действие - внести константу в ОПС
        struct TypedData tdata = Pop(variables);
        struct Variable* vt = (struct Variable*)tdata.data; // Получаем константу из стека
        if(vt->type < 5){ // если в стеке была не константа
            err_no = ERR_IN_VARIABLES_STACK;
            err_throw();
            return 0;
        }
        struct TypedData td;
        if(vt->type == 5){ // real
            write_to += sprintf(write_to, "%g ", *(double*)vt->address) * sizeof(char);

            td.data = malloc(sizeof(double));
            *(double*)td.data = *(double*)vt->address;
            td.type = vt->type;

        }
        else{ // int
            write_to += sprintf(write_to, "%d ", *(int*)vt->address) * sizeof(char);


            td.data = malloc(sizeof(int));
            *(int*)td.data = *(int*)vt->address;
            td.type = vt->type;

        }
        ListAppend(ops, td);


        ops_els++;
        return 1;
    }
    else if(tdata.type == 3){
        write_to += sprintf(write_to, "%s ", oper_no_resolver(*(int*)tdata.data)) * sizeof(char);

        struct TypedData td;
        td.data = malloc(sizeof(int));
        *(int*)td.data = *(int*)tdata.data;
        td.type = 0;
        ListAppend(ops, td);
        ops_els++;
        return 1;
    }
    else{ // Действие - семантическая подпрограмма
        int dt = *(int*)tdata.data;
        switch(dt){
            case 1: // Встретилось real[E], объявляем статические массивы
                open_table_type = 1; // Тип: ссылка на real
            break;
            case 2: // Встретилось real[*], объявляем динамические массивы
                open_table_type = 2;
            break;
            case 3: // Встретилось real (без квадратных скобок), объявляем real (если не объявляем массивы)
                if(open_table_type == 0){
                    open_table_type = 3;
                }
            break;
            case 4: // Внести идентификатор в таблицу, выделить память под переменную
                {
                    struct TypedData tdata = Pop(variables);
                    struct Variable* v = (struct Variable*)tdata.data; // Получаем значение из стека
                    switch(open_table_type){ // выделение памяти
                        case 1: // выделение памяти для статического массива. v - идентификатор массива
                            {
                                if(v->type != 0){
                                    err_no = ERR_IN_VARIABLES_STACK;
                                    err_throw();
                                    free(v->address);
                                    return 0;
                                }
                                struct TypedData td = Pop(variables);
                                struct Variable *vt = (struct Variable*)td.data; // vt - размер массива (очередное значение из стека)
                                if(vt->type != 6){ // если vt не const int
                                    err_no = ERR_INDEX_NOT_INTEGER;
                                    err_throw();
                                    free(v->address);
                                    return 0;
                                }
                                int size = *(int*)vt->address;

                                v->address = malloc(sizeof(int) * 3); // 0 - ссылка на первый элемент, 1 - размер элемента в байтах, 2 - число элементов
                                (((int*)v->address)[0]) = (int)malloc(sizeof(double) * size);
                                (((int*)v->address)[1]) = sizeof(double);
                                (((int*)v->address)[2]) = size;
                                Push(variables, td);
                            }
                        break;
                        case 2: // выделение памяти для паспорта динамического массива. v - идентификатор массива
                            {
                                if(v->type != 0){
                                    err_no = ERR_IN_VARIABLES_STACK;
                                    err_throw();
                                    return 0;
                                }
                                v->address = malloc(sizeof(int) * 3);
                                (((int*)v->address)[0]) = (int)NULL;
                            }
                        break;
                        case 3:
                            if(v->type != 0){
                                err_no = ERR_IN_VARIABLES_STACK;
                                err_throw();
                                free(v->address);
                                return 0;
                            }
                            v->address = malloc(sizeof(double));
                        break;
                    }
                    if (!VariableTable_add(v->name, v->address, open_table_type)){
                        free(v->address);
                        free(tdata.data);
                        return 0;
                    }
                    Push(variables, tdata);
                }
            break;
            case 5: // Закрываем таблицу, очищаем стек инициализации
                open_table_type = 0;
                if (!IsEmpty(*variables->top)){
                    StackDispose(variables);
                }
                variables = NewStack();
            break;
            case 6: // Убрать из стека инициализации первое значение
                Pop(variables);
            break;

            case 10: // 10 и далее: работа с метками для генерации условных/безусловных переходов
                {
                    // Добавление метки в стек
                    struct TypedData tdata;
                    //tdata.data = malloc(sizeof(void*));
                    tdata.data = write_to;
                    tdata.type = 0;
                    Push(labels_test_ops, tdata);

                    struct TypedData td;
                    td.data = malloc(sizeof(int));
                    *(int*)td.data = ops_els;
                    td.type = 0;
                    Push(labels_ops, td);

                    // Добавление в ОПС пустого символа
                    write_to += sprintf(write_to, "     ") * sizeof(char);
                    //=====
                    struct TypedData ops_el;
                    ops_el.data = malloc(sizeof(int));
                    *(int*)ops_el.data = -1;
                    ops_el.type = -1;
                    ListAppend(ops, ops_el);

                    //ops_els++;

                    // Добавление в ОПС jf
                    write_to += sprintf(write_to, "jf ") * sizeof(char);
                    //=====
                    ops_el.data = malloc(sizeof(int));
                    *(int*)ops_el.data = oper_resolver("jf");
                    ops_el.type = 0;
                    ListAppend(ops, ops_el);

                    ops_els+=2;
                    //printf("%p=========%p", labels_test_ops->top->tdata.data, write_to);
                }
            break;
            case 11:
                {
                    // Запись значения метки в пустой элемент по метке
                    struct TypedData tdata = Pop(labels_test_ops);
                    struct TypedData tdata2 = Pop(labels_test_ops);

                    struct TypedData td = Pop(labels_ops);
                    struct TypedData td2 = Pop(labels_ops);
                    int len = sprintf((char*)tdata2.data, "%d", *(int*)td.data);
                    *((char*)tdata2.data + sizeof(char)*len) = ' ';
                    //=====
                    struct TypedData ops_el = ListGetItem(*ops, *(int*)td2.data);
                    if(ops_el.data != NULL){
                            printf("SOMETHING WENT right (STILL WRONG)!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                        *(int*)ops_el.data = *(int*)td.data;
                        ops_el.type = 6;
                    }
                    else{
                        printf("SOMETHING WENT WRONG!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                    }


                    //free((char*)tdata2.data); //освобождаем указатель на часть какой-то строки. из-за этого и были ошибки кучи
                    free(td2.data);
                    //free((char*)tdata.data);
                    free(td.data);

                    // Добавление метки в стек
                    tdata.data = write_to;
                    tdata.type = 0;
                    Push(labels_test_ops, tdata);

                    td.data = malloc(sizeof(int));
                    *(int*)td.data = ops_els;
                    td.type = 0;
                    Push(labels_ops, td);

                    // Добавление в ОПС пустого символа
                    write_to += sprintf(write_to, "     ") * sizeof(char);
                    //=====
                    ops_el.data = malloc(sizeof(int));
                    *(int*)ops_el.data = -1;
                    ops_el.type = -1;
                    ListAppend(ops, ops_el);

                    // Добавление в ОПС jf
                    write_to += sprintf(write_to, "jf ") * sizeof(char);
                    //=====
                    ops_el.data = malloc(sizeof(int));
                    *(int*)ops_el.data = oper_resolver("jf");
                    ops_el.type = 0;
                    ListAppend(ops, ops_el);

                    ops_els+=2;
                }
            break;
            case 12:
                {
                    // Запись значения в пустой элемент по метке
                    if(!IsEmpty(*labels_test_ops->top)){
                        //printf("%p=========%p", labels_test_ops->top->tdata.data, write_to);
                        struct TypedData tdata = Pop(labels_test_ops);
                        int len = sprintf((char*)tdata.data, "%d", ops_els);
                        *((char*)tdata.data + sizeof(char)*len) = ' ';
                    }
                }
            break;
            case 13:
                {
                    // Добавление метки в стек
                    struct TypedData tdata;
                    tdata.data = write_to;
                    tdata.type = 0;
                    Push(labels_test_ops, tdata);

                    struct TypedData td;
                    td.data = malloc(sizeof(int));
                    *(int*)td.data = ops_els;
                    td.type = 0;
                    Push(labels_ops, td);
                }
            break;
            case 14:
                {
                    // Запись значения в пустой элемент по метке
                    struct TypedData tdata = Pop(labels_test_ops);
                    struct TypedData td = Pop(labels_ops);
                    int len = sprintf((char*)tdata.data, "%d", ops_els+2);
                    *((char*)tdata.data + sizeof(char)*len) = ' ';

                    //free((char*)tdata.data);
                    free(td.data);
                    tdata = Pop(labels_test_ops);
                    td = Pop(labels_ops);

                    // Добавление метки в ОПС
                    write_to += sprintf(write_to, "%d ", *(int*)td.data) * sizeof(char);

                    // Добавление в ОПС j
                    write_to += sprintf(write_to, "j ") * sizeof(char);
                    ops_els+=2;
                }
            break;
        }
        return 1;
    }
}

void parser_init(){
    tokenizer_init();
    VariableTable_init();
    write_to = test_ops;

    magazine = NewStack();
    Push(magazine, lexeme("\0"));
    Push(magazine, nterm('P', 0));

    act_magazine = NewStack();
    struct TypedData tdata;
    tdata.data = malloc(sizeof(int)); *(int*)tdata.data = 0; tdata.type = 0;
    Push(act_magazine, tdata);
    tdata.data = malloc(sizeof(int)); *(int*)tdata.data = 0; tdata.type = 0;
    Push(act_magazine, tdata);

    variables = NewStack();
    ops = NewList();
    labels_ops = NewStack();
    labels_test_ops = NewStack();

    lambda = NewList();
    struct List* assign = NULL;

    // Инициализация строк
    for(int i = 0; i < RULES_H; i++){
        switch(parser_sts[i]){
            case 'P':
            case 'J':
            case 'O':
            case 'X':
            case 'U':
            case 'V':
            case 'M':
            case 'H':
            case 'Q':
            case 'Z':
                assign = lambda;
                break;
            default:
                assign = NULL;
                break;
        }
        for(int j = 0; j < RULES_W; j++){
            rules_gen_symbols[i][j] = assign;
            rules_gen_actions[i][j] = NULL;
        }
    }
    // Заполняем правила (вручную, т.к. лексемы/нетерминалы могут состоять из более чем одного символа)
    int ind1, ind2;
    { ind1 = parser_nt_resolver('P',0);
        {
            char* rule[] = {"a", "\\P1", ";", "\\P", "\0"};
            char* act[] = {"a", "0", "=", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"if", "\\C", "{", "\\P", "}", "\\X", "\\Z", "\\P", "\0"};
            char* act[] = {"0", "0", "10", "0", "0", "0", "12", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"read", "(", "a", "\\H", ")", ";", "\\P", "\0"};
            char* act[] = {"0", "0", "a", "0", "read", "0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"write", "(", "\\E", ")", ";", "\\P", "\0"};
            char* act[] = {"0", "0", "0", "write", "0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"real", "\\M", "\\Z", "a", "\\O", "\\J", ";", "\\P", "\0"};
            char* act[] = {"0", "0", "3", "4", "0", "0", "5", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"while", "\\C", "{", "\\P", "}", "\\P", "\0"};
            char* act[] = {"13", "0", "10", "0", "0", "14"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('P',1);
        {
            char* rule[] = {"[", "\\E", "]", "=", "\\E", "\0"};
            char* act[] = {"0", "0", "ind", "0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"=", "\\P2", "\0"};
            char* act[] = {"0", "0"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('P',2); //также: O1, E
        {
            char* rule[] = {"a", "\\H", "\\V", "\\U", "\0"};
            char* act[] = {"a", "0", "0", "0"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('O',1), rule, act);
            fill_gen(parser_nt_resolver('E',0), rule, act);
        }
        {
            char* rule[] = {"k", "\\V", "\\U", "\0"};
            char* act[] = {"k", "0", "0"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('O',1), rule, act);
            fill_gen(parser_nt_resolver('E',0), rule, act);
        }
        {
            char* rule[] = {"(", "\\E", ")", "\\V", "\\U", "\0"};
            char* act[] = {"0", "0", "0", "0", "0"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('O',1), rule, act);
            fill_gen(parser_nt_resolver('E',0), rule, act);
        }
        {
            char* rule[] = {"sqrt", "(", "\\E", ")", "\\V", "\\U", "\0"};
            char* act[] = {"0", "0", "0", "sqrt", "0", "0"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('O',1), rule, act);
            fill_gen(parser_nt_resolver('E',0), rule, act);
        }
        {
            char* rule[] = {"exp", "(", "\\E", ")", "\\V", "\\U", "\0"};
            char* act[] = {"0", "0", "0", "exp", "0", "0"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('O',1), rule, act);
            fill_gen(parser_nt_resolver('E',0), rule, act);
        }
        {
            char* rule[] = {"log", "(", "\\E", ",", "\\E", ")", "\\V", "\\U", "\0"};
            char* act[] = {"0", "0", "0", "0", "0", "log", "0", "0"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('O',1), rule, act);
            fill_gen(parser_nt_resolver('E',0), rule, act);
        }
        {
            char* rule[] = {"new", "[", "\\E", "]", "\0"};
            char* act[] = {"0", "0", "0", "new"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('O',1), rule, act);
        }
    }
    { ind1 = parser_nt_resolver('J',0);
        {
            char* rule[] = {",", "a", "\\O", "\\J", "\0"};
            char* act[] = {"6", "4", "0", "0"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('O',0);
        {
            char* rule[] = {"=", "\\O1", "\\Z", "\0"};
            char* act[] = {"a", "0", "="};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('X',0);
        {
            char* rule[] = {"else", "\\R", "\0"};
            char* act[] = {"0", "0"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('R',0);
        {
            char* rule[] = {"if", "\\C", "{", "\\P", "}", "\\X", "\0"};
            char* act[] = {"13", "0", "11", "0", "0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"{", "\\P", "}", "\0"};
            char* act[] = {"12", "0", "0"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('U',0);
        {
            char* rule[] = {"+", "\\T", "\\U", "\0"};
            char* act[] = {"0", "0", "+"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"-", "\\T", "\\U", "\0"};
            char* act[] = {"0", "0", "-"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('T',0);
        {
            char* rule[] = {"a", "\\H", "\\V", "\0"};
            char* act[] = {"a", "0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"k", "\\V", "\0"};
            char* act[] = {"k", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"(", "\\E", ")", "\\V", "\0"};
            char* act[] = {"0", "0", "0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"sqrt", "(", "\\E", ")", "\\V", "\0"};
            char* act[] = {"0", "0", "0", "sqrt", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"exp", "(", "\\E", ")", "\\V", "\0"};
            char* act[] = {"0", "0", "0", "exp", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"log", "(", "\\E", ",", "\\E", ")", "\\V", "\0"};
            char* act[] = {"0", "0", "0", "0", "0", "log", "0"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('V',0);
        {
            char* rule[] = {"*", "\\G", "\\V", "\0"};
            char* act[] = {"0", "0", "*"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"/", "\\G", "\\V", "\0"};
            char* act[] = {"0", "0", "/"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('G',0); // Также: G1
        {
            char* rule[] = {"a", "\\H", "\0"};
            char* act[] = {"a", "0"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('G',1), rule, act);
        }
        {
            char* rule[] = {"k", "\0"};
            char* act[] = {"k"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('G',1), rule, act);
        }
        {
            char* rule[] = {"+", "\\G1", "\0"};
            char* act[] = {"0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"-", "\\G1", "\\Z", "\0"};
            char* act[] = {"0", "0", "unary-"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"(", "\\E", ")", "\0"};
            char* act[] = {"0", "0", "0"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('G',1), rule, act);
        }
        {
            char* rule[] = {"sqrt", "(", "\\E", ")", "\0"};
            char* act[] = {"0", "0", "0", "sqrt"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('G',1), rule, act);
        }
        {
            char* rule[] = {"exp", "(", "\\E", ")", "\0"};
            char* act[] = {"0", "0", "0", "exp"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('G',1), rule, act);
        }
        {
            char* rule[] = {"log", "(", "\\E", ",", "\\E", ")", "\0"};
            char* act[] = {"0", "0", "0", "0", "0", "log"};
            fill_gen(ind1, rule, act);
            fill_gen(parser_nt_resolver('G',1), rule, act);
        }
    }
    { ind1 = parser_nt_resolver('M',0);
        {
            char* rule[] = {"[", "\\L", "\0"};
            char* act[] = {"0", "0"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('L',0);
        {
            char* rule[] = {"a", "\\H", "\\V", "\\U", "]", "\0"};
            char* act[] = {"0", "0", "0", "0", "1"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"k", "\\V", "\\U", "]", "\0"};
            char* act[] = {"0", "0", "0", "1"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"*", "]", "\0"};
            char* act[] = {"0", "2"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"(", "\\E", ")", "\\V", "\\U", "]", "\0"};
            char* act[] = {"0", "0", "0", "0", "0", "1"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"sqrt", "(", "\\E", ")", "\\V", "\\U", "]", "\0"};
            char* act[] = {"0", "0", "0", "sqrt", "0", "0", "1"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"exp", "(", "\\E", ")", "\\V", "\\U", "]", "\0"};
            char* act[] = {"0", "0", "0", "exp", "0", "0", "1"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"log", "(", "\\E", ",", "\\E", ")", "\\V", "\\U", "]", "\0"};
            char* act[] = {"0", "0", "0", "0", "0", "log", "0", "0", "1"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('H',0);
        {
            char* rule[] = {"[", "\\E", "]", "\0"};
            char* act[] = {"0", "0", "ind"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('C',0);
        {
            char* rule[] = {"a", "\\H", "\\V", "\\U", "\\Q", "\0"};
            char* act[] = {"a", "0", "0", "0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"k", "\\V", "\\U", "\\Q", "\0"};
            char* act[] = {"k", "0", "0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"(", "\\C", ")", "\0"};
            char* act[] = {"0", "0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"sqrt", "(", "\\E", ")", "\\V", "\\U", "\\Q", "\0"};
            char* act[] = {"0", "0", "0", "sqrt", "0", "0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"exp", "(", "\\E", ")", "\\V", "\\U", "\\Q", "\0"};
            char* act[] = {"0", "0", "0", "exp", "0", "0", "0"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"log", "(", "\\E", ",", "\\E", ")", "\\V", "\\U", "\\Q", "\0"};
            char* act[] = {"0", "0", "0", "0", "0", "log", "0", "0", "0"};
            fill_gen(ind1, rule, act);
        }
    }
    { ind1 = parser_nt_resolver('Q',0);
        {
            char* rule[] = {"<", "\\E", "\\Z", "\0"};
            char* act[] = {"0", "0", "<"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {">", "\\E", "\\Z", "\0"};
            char* act[] = {"0", "0", ">"};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"<=", "\\E", "\\Z", "\0"};
            char* act[] = {"0", "0", "<="};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {">=", "\\E", "\\Z", "\0"};
            char* act[] = {"0", "0", ">="};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"==", "\\E", "\\Z", "\0"};
            char* act[] = {"0", "0", "=="};
            fill_gen(ind1, rule, act);
        }
        {
            char* rule[] = {"!=", "\\E", "\\Z", "\0"};
            char* act[] = {"0", "0", "!="};
            fill_gen(ind1, rule, act);
        }
    }
}

void fill_gen(int nt_no, char* rule[], char* act[]){
    int len = 0;
    while(strlen(rule[len])){
        len++;
    }
    int ind2 = tk_resolver(rule[0])-1;
    rules_gen_symbols[nt_no][ind2] = gen_symbols(rule, len);
    rules_gen_actions[nt_no][ind2] = gen_actions(act, len);
}

struct List* gen_symbols(char* rule[], int len){
    struct List* assign = NewList();
    for(int i = 0; i < len; i++){
        if(rule[i][0]=='\\'){
            ListAppend(assign, nterm(rule[i][1], rule[i][2] - '0'));
        }
        else{
            ListAppend(assign, lexeme(rule[i]));
        }
    }
    return assign;
}

struct List* gen_actions(char* act[], int len){
    struct List* assign = NewList();
    int tk_no;
    struct TypedData tdata;
    for(int i = 0; i < len; i++){
        tk_no = oper_resolver(act[i]);

        if(tk_no){
            if(tk_no > 2){ // Действие: внести в ОПС операцию
                tdata.data = malloc(sizeof(int));
                *(int*)(tdata.data) = tk_no;
                tdata.type = 3;
            }
            else{ // Действие: внести в ОПС константу (необходимо перед этим получить ее значение и тип) или переменную (необходимо разыменовать, получить значение ссылки и тип)
                tdata.data = malloc(sizeof(int));
                *(int*)(tdata.data) = -1;
                tdata.type = tk_no;
            }
        }
        else{ // Пустое действие или семантическая подпрограмма
            tdata.data = malloc(sizeof(int));
            *(int*)(tdata.data) = StringToInt(act[i]);
            tdata.type = 0;
        }
        ListAppend(assign, tdata);
    }
    return assign;
};

// Для тестов правильности заполнения таблиц
void print_tables(){
    struct List* out = NULL;
    for(int i = 0; i < RULES_H; i++){
        printf("\n%c:", parser_sts[i]);
        for(int j = 0; j < RULES_W; j++){
            if(rules_gen_symbols[i][j] != NULL && !IsEmpty(*rules_gen_symbols[i][j])){
                printf("\n\t");
                printf("%s", tk_no_resolver(j+1));
                printf(":\t");
                print_rule(rules_gen_symbols[i][j]);

                if(rules_gen_actions[i][j] != NULL){
                    printf("\n\t\t");
                    print_action(rules_gen_actions[i][j]);
                }
            }
            else{
                out = rules_gen_symbols[i][j];
            }
        }
        printf("\n\tOthers: "); print_rule(out);
    }
    printf("\n");
}

void parser_dispose(){
    StackDispose(magazine);
    StackDispose(act_magazine);
    StackDispose(variables);
    ListDispose(ops);
    StackDispose(labels_ops);
    StackDispose(labels_test_ops);
    tokenizer_dispose();
}

struct TypedData NewLexeme(char* lx){
    struct TypedData lexeme;
    lexeme.data = malloc(sizeof(int));
    *(int*)(lexeme.data) = tk_resolver(lx);
    lexeme.type = 0;

    return lexeme;
};

struct TypedData NewNonterminal(char nt, int ind){
    struct TypedData nterm;
    nterm.data = malloc(sizeof(int));
    *(int*) nterm.data = parser_nt_resolver(nt, ind);
    nterm.type = 1;

    return nterm;
};

int parser_nt_resolver(char nt_ch, int ind){
    switch (nt_ch) {
        case 'P':
            if(ind == 1)
                return 1;
            if(ind == 2)
                return 2;
            return 0;
        case 'J':   return 3;
        case 'O':
            if(ind == 1)
                return 5;
            return 4;
        case 'X':   return 6;
        case 'R':   return 7;
        case 'E':   return 8;
        case 'U':   return 9;
        case 'T':   return 10;
        case 'V':   return 11;
        case 'G':
            if(ind == 1)
                return 13;
            return 12;
        case 'M':   return 14;
        case 'L':   return 15;
        case 'H':   return 16;
        case 'C':   return 17;
        case 'Q':   return 18;
        case 'Z':   return 19;
        default:  return -1;
    }
}

// Только для тестов, вывод содержимого стека/списков таблиц
void print_symbol(struct TypedData tdata){
    int dt = *(int*)tdata.data;
    if(tdata.type == 0){
        printf("%s", tk_no_resolver(dt));
    }
    else{
        char st = parser_sts[dt];
        int ind = 0;
        if(st == ' '){
            if(dt == 2){
                ind = 2;
                st = 'P';
            }
            else{
                ind = 1;
                st = parser_sts[dt-1];
            }
        }
        if(ind){
            printf("%c%d", st, ind);
        }
        else{
            printf("%c", st);
        }
    }
}
void print_action_symbol(struct TypedData tdata){
    int dt;
    if(tdata.type == 0){
        dt = *(int*)tdata.data;
        if(dt){
            printf("%s", IntToString(dt));
        }
        else{
            printf("'");
        }
    }
    else if(tdata.type == 3){
        dt = *(int*)tdata.data;
        printf("%s", oper_no_resolver(dt));
    }
    else{
        printf("%s", tk_no_resolver(tdata.type));
    }
}

void print_rule(struct List* list){
    if(list != NULL){
        if(!IsEmpty(*list)){
            struct TypedData tdata = list->tdata;
            while(list->next != NULL){
                print_symbol(tdata);
                list = list->next;
                tdata = list->tdata;
            }
            print_symbol(tdata);
        }
        else{
            printf("l");
        }
    }
    else{
        printf("err");
    }
}
void print_rule_bk(struct List* list){
    if(list != NULL){
        if(!IsEmpty(*list)){
            struct TypedData tdata = list->tdata;
            while(list->prev != NULL){
                print_symbol(tdata);
                list = list->prev;
                tdata = list->tdata;
            }
            print_symbol(tdata);
        }
        else{
            printf("l");
        }
    }
    else{
        printf("err");
    }
}

void print_action(struct List* list){
    if(list != NULL){
        if(!IsEmpty(*list)){
            struct TypedData tdata = list->tdata;
            while(list->next != NULL){
                print_action_symbol(tdata);
                list = list->next;
                tdata = list->tdata;
            }
            print_action_symbol(tdata);
        }
        else{
            printf("l");
        }
    }
    else{
        printf("err");
    }
}
void print_action_bk(struct List* list){
    if(list != NULL){
        if(!IsEmpty(*list)){
            struct TypedData tdata = list->tdata;
            while(list->prev != NULL){
                print_action_symbol(tdata);
                list = list->prev;
                tdata = list->tdata;
            }
            print_action_symbol(tdata);
        }
        else{
            printf("l");
        }
    }
    else{
        printf("err");
    }
}

void print_variable(struct TypedData tdata){
    struct Variable * v = (struct Variable *)tdata.data;
    switch(v->type){
    case 5:
        printf("%g ", *(double*)v->address);
        break;
    case 6:
        printf("%d ", *(int*)v->address);
        break;
    default:
        printf("%s ", (char*)v->name);
        break;
    }
}

void print_variables(){
    struct List* list = variables->top;
    if(list != NULL){
        if(!IsEmpty(*list)){
            struct TypedData tdata = list->tdata;
            while(list->prev != NULL){
                print_variable(tdata);
                list = list->prev;
                tdata = list->tdata;
            }
            print_variable(tdata);
        }
        else{
            printf("List is empty");
        }
    }
}

void push_variable(){
    struct TypedData td;
    td.data = (struct Variable*) malloc(sizeof(struct Variable));
    struct Variable * v = (struct Variable*)td.data;
    if(_out_tk_no == 1){
        v->name = strdup(name);
        v->address = NULL;
        v->type = 0;
    }
    else{
        v->name = NULL;
        if(_out_st == 'C'){
            v->address = malloc(sizeof(int));
            *(int*)v->address = n;
            v->type = 6; // const int
        }
        else{
            v->address = malloc(sizeof(double));
            *(double*)v->address = x;
            v->type = 5; // const real
        }
    }
    Push(variables, td);
}

