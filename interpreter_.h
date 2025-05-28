#ifndef INTERPRETER__H_INCLUDED
#define INTERPRETER__H_INCLUDED

//P.S. ѕо сути OPSItem и StackItem можно объединить в одну структуру, так как обе содержат одно и тоже, но было решено использовать всЄ же разное
// 1. type у OPSItem: 0 Ч ссылка, 1 Ч значение, 2 Ч операци€, а у StackItem 0 Ч ссылка, 1 Ч значение
// 2. »з-за семантических различий, ради избежании путаницы при чтени€ кода.

void interpret(char * program);

#endif // INTERPRETER__H_INCLUDED
