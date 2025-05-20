#include "compiler_globals.h"
#include "tokenizer_semantic_programs.h"

void pr_err_throw(char c){
    if(err_no == 0){
        err_no = ERR_UNKNOWN_LEXEME;
    }
    err_throw();
}

void allocate_resources(){
    name = malloc(MAX_ID_LEN);
}

void free_resources(){
    free(name);
}

void pr_name_init(char c) {
    free(name);
    name = malloc(MAX_ID_LEN);

    name[0] = c;
    name[1] = '\0';
}

void pr_name_add(char c) {
    int len = strlen(name);
    if(len >= MAX_ID_LEN-1){
        err_no = ERR_ID_TOO_LARGE;
        pr_err_throw(c);
        return;
    }
    name[len] = c;
    name[len + 1] = '\0';
}

void pr_name_end(char c){
    _out_tk_no = kwrd_resolver(name);
}

void pr_n_init(char c) {
    n = c - '0';
}

void pr_n_add(char c) {
    n = n * 10 + (c - '0');
}

void pr_n_end(char c){
    _out_tk_no = 2;
}

void pr_real_init(char c) {
    d = 1.0;
    x = n;
}

void pr_real_add(char c) {
    d *= 0.1;
    x += (c - '0') * d;
}

void pr_op_plus(char c) {
    _out_tk_no = 3;
}

void pr_op_minus(char c) {
    _out_tk_no = 4;
}

void pr_op_mul(char c) {
    _out_tk_no = 5;
}

void pr_op_div(char c) {
    _out_tk_no = 6;
}

void pr_br_lsquare(char c) {
    _out_tk_no = 7;
}

void pr_br_rsquare(char c) {
    _out_tk_no = 8;
}

void pr_br_lparen(char c) {
    _out_tk_no = 9;
}

void pr_br_rparen(char c) {
    _out_tk_no = 10;
}

void pr_br_lcurly(char c) {
    _out_tk_no = 11;
}

void pr_br_rcurly(char c) {
    _out_tk_no = 12;
}

void pr_do_nothing(char c) {
    // ничего не делаем
}

void pr_change_line(char c){
    line_no++;
    char_no = 0;
}

void pr_comp_end_single(char c){
    if(name[0] == '='){
        _out_tk_no = 16;
        return;
    }
    if(name[0] == '<'){
        _out_tk_no = 17;
        return;
    }
    if(name[0] == '>'){
        _out_tk_no = 18;
        return;
    }
    err_no = ERR_UNKNOWN;
    pr_err_throw(c);
}

void pr_comp_end_compound(char c){
    pr_name_add(c);
    if(!strcmp(name, "==")){
        _out_tk_no = 19;
        return;
    }
    if(!strcmp(name, "<=")){
        _out_tk_no = 20;
        return;
    }
    if(!strcmp(name, ">=")){
        _out_tk_no = 21;
        return;
    }
    if(!strcmp(name, "!=")){
        _out_tk_no = 22;
        return;
    }
    err_no = ERR_UNKNOWN;
    pr_err_throw(c);
}

void pr_comma(char c){
    _out_tk_no = 13;
}

void pr_line_terminal(char c){
    _out_tk_no = 14;
}

void pr_program_end(char c){
    _out_tk_no = 15;
}

// Функция перекодировки функций семантических подпрограмм в адреса
semanticpr pr_resolver(int pr_ind) {
    switch(pr_ind) {
        case 1:  return pr_name_init;               // name := C[i]
        case 2:  return pr_n_init;                  // n := ord(C[i]) – ord('0')

        case 3:  return pr_op_plus;                 // распознано "+"
        case 4:  return pr_op_minus;                // распознано "-"
        case 5:  return pr_op_mul;                  // распознано "*"
        case 6:  return pr_op_div;                  // распознано "/"
        case 7:  return pr_br_lsquare;              // распознана "["
        case 8:  return pr_br_rsquare;              // распознана "]"
        case 9:  return pr_br_lparen;               // распознана "("
        case 10: return pr_br_rparen;               // распознана ")"
        case 11: return pr_br_lcurly;               // распознан "{"
        case 12: return pr_br_rcurly;               // распознан "}"

        case 13: return pr_do_nothing;              // пробел
        case 14: return pr_change_line;             // перенос строки
        case 15: return pr_line_terminal;           // распознано ";"

        case 16: return pr_comp_end_single;         // распознан оператор сравнения или присвоения
        case 17: return pr_program_end;             // распознан конец программы
        case 18: return pr_name_add;                // name += C[i]
        case 19: return pr_name_end;                // завершение лексемы, поиск ключевого слова
        case 20: return pr_n_add;                   // n := n * 10 + ord(C[i]) - ord('0')
        case 21: return pr_real_init;               // d := 1; x := n
        case 22: return pr_n_end;                   // распознано число
        case 23: return pr_real_add;                // d := d * 0.1; x += (ord(C[i]) - ord('0')) * d

        case 26: return pr_comp_end_compound;       // name += C[i]; поиск лексемы

        case 28: return pr_comma;                   // распознана ","
        default: return pr_err_throw;
    }
}
