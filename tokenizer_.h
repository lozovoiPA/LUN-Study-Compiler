#ifndef TOKENIZER__H_INCLUDED
#define TOKENIZER__H_INCLUDED

// функции для инициализации массивов переходов
void tokenizer_init();
// Функция лексического анализатора
int tokenizer(char* str, int i);
// Функция для вывода результатов лекс. анализатора
void tokenizer_output();
void tokenizer_dispose();

#endif // TOKENIZER__H_INCLUDED
