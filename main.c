#include "interpreter_.h"
#include "parser_semantic_programs.h"
#include "compiler_globals.h"

#define MAX_FILE_SIZE 1024 * 1024 // Максимальный размер файла (1 МБ)

int main()
{
    system("chcp 1251>nul");
    // Открытие файла для чтения
    char filename[512];
    gets (filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Ошибка открытия файла");
        return EXIT_FAILURE;
    }

    char *buffer = malloc(MAX_FILE_SIZE);
    if (buffer == NULL) {
        perror("Ошибка выделения памяти");
        fclose(file);
        return EXIT_FAILURE;
    }

    size_t bytesRead = fread(buffer, sizeof(char), MAX_FILE_SIZE - 1, file);
    buffer[bytesRead] = '\0'; // Завершение строки нулевым символом

    printf("%s\n\n", buffer);
    interpret(buffer);

    // Закрытие файла
    fclose(file);
    free(buffer);
    return 0;
}

