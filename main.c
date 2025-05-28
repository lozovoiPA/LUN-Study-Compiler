#include "parser_.h"
#include "parser_semantic_programs.h"

#define MAX_FILE_SIZE 1024 * 1024 // Максимальный размер файла (1 МБ)

int main()
{
    // Открытие файла для чтения
    FILE *file = fopen("input.txt", "r");
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

    parse(buffer);
    // Закрытие файла
    fclose(file);
    free(buffer);
    return 0;
}

