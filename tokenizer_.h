#ifndef TOKENIZER__H_INCLUDED
#define TOKENIZER__H_INCLUDED

// ������� ��� ������������� �������� ���������
void tokenizer_init();
// ������� ������������ �����������
int tokenizer(char* str, int i);
// ������� ��� ������ ����������� ����. �����������
void tokenizer_output();
void tokenizer_dispose();

#endif // TOKENIZER__H_INCLUDED
