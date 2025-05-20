#ifndef TOKENIZER_SEMANTIC_PROGRAMS_H_INCLUDED
#define TOKENIZER_SEMANTIC_PROGRAMS_H_INCLUDED

// ��� ��� ����������� �� pr_resolver ������ ������������� ������������ (��� �� ������ � ���� ��������� �������, ������� �� ������ ������ �� ����� ����� �������)
// � �������� ���������� �������� ������� ������ (����� ��� ���������)
typedef void (*semanticpr)(char c);

void allocate_resources();
void free_resources();
semanticpr pr_resolver(int pr_ind);

#endif // TOKENIZER_SEMANTIC_PROGRAMS_H_INCLUDED
