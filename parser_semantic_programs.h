#ifndef PARSER_SEMANTIC_PROGRAMS_H_INCLUDED
#define PARSER_SEMANTIC_PROGRAMS_H_INCLUDED

// ����� �� ������� ������� ����, ������� �� ������ ���, ����� ������������ ������
// ����� �������� ��� ������������� ����������

struct Variable{
    char* name;
    void* address;
    int type;
};
// ������ ������� ���������������.
// ����� � ���� ������� ����� ���� �������� �� ���� ������.
// ��������� Variable - ������ �� ��������� ������� ���������������,
// �����������, ���� �� ������� ������������
#define MAX_ID_PER_TABLE 512
extern struct Variable * VariableTable;

void VariableTable_init();
void VariableTable_extend();
struct Variable* VariableTable_find(char* name);
int VariableTable_add(char* name, void* address, int type);
struct Variable * VariableTable_at(int ind);
void VariableTable_print();
#endif // PARSER_SEMANTIC_PROGRAMS_H_INCLUDED
