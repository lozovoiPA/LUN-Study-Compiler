#ifndef TOKENIZER_SEMANTIC_PROGRAMS_H_INCLUDED
#define TOKENIZER_SEMANTIC_PROGRAMS_H_INCLUDED

// Тип для возвращения из pr_resolver адреса семантической подпрограммы (все СП заданы в виде отдельной функции, поэтому по такому адресу их сразу можно вызвать)
// в качестве аргументов передаем текущий символ (нужен для рассчетов)
typedef void (*semanticpr)(char c);

void allocate_resources();
void free_resources();
semanticpr pr_resolver(int pr_ind);

#endif // TOKENIZER_SEMANTIC_PROGRAMS_H_INCLUDED
