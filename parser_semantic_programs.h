#ifndef PARSER_SEMANTIC_PROGRAMS_H_INCLUDED
#define PARSER_SEMANTIC_PROGRAMS_H_INCLUDED

// „тобы не плодить таблицы имен, адресов на каждый тип, будем использовать массив
// таких структур дл€ представлени€ переменных

struct Variable{
    char* name;
    void* address;
    int type;
};
// –азмер таблицы идентификаторов.
// ¬сего в одну таблицу может быть записано на один меньше.
// ѕоследний Variable - ссылка на следующую таблицу идентификаторов,
// —оздаваемую, если не хватает существующей
#define MAX_ID_PER_TABLE 512
extern struct Variable * VariableTable;

void VariableTable_init();
void VariableTable_extend();
struct Variable* VariableTable_find(char* name);
int VariableTable_add(char* name, void* address, int type);
struct Variable * VariableTable_at(int ind);
void VariableTable_print();
#endif // PARSER_SEMANTIC_PROGRAMS_H_INCLUDED
