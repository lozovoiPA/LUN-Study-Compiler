#include "interpreter_.h"
#include "parser_semantic_programs.h"
#include "compiler_globals.h"

#define MAX_FILE_SIZE 1024 * 1024 // ������������ ������ ����� (1 ��)

int main()
{
    system("chcp 1251>nul");
    // �������� ����� ��� ������
    char filename[512];
    gets (filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("������ �������� �����");
        return EXIT_FAILURE;
    }

    char *buffer = malloc(MAX_FILE_SIZE);
    if (buffer == NULL) {
        perror("������ ��������� ������");
        fclose(file);
        return EXIT_FAILURE;
    }

    size_t bytesRead = fread(buffer, sizeof(char), MAX_FILE_SIZE - 1, file);
    buffer[bytesRead] = '\0'; // ���������� ������ ������� ��������

    printf("%s\n\n", buffer);
    interpret(buffer);

    // �������� �����
    fclose(file);
    free(buffer);
    return 0;
}

