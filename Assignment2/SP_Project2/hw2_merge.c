#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    FILE* merged_file = fopen(argv[1], "wb");

    int key = atoi(argv[2]);

    printf("encode with key 0x%x\n", key);

    for (int i = 3; i < argc; ++i) {
        FILE* source_file = fopen(argv[i], "rb");

        if (merged_file == NULL || source_file == NULL) {
            printf("\nFile Open Error\n");
            return 0;
        }

        fputs(argv[i], merged_file);

        fseek(source_file, 0, SEEK_END);
        int file_size = ftell(source_file);
        rewind(source_file);

        char buffer[100];
        sprintf(buffer, " %d\n", file_size);
        fputs(buffer, merged_file);

        for (int i = 0; i < file_size; ++i) {
            fputc(fgetc(source_file) ^ key, merged_file);
        }

        printf("merge %s\n", argv[i]);

        fclose(source_file);
    }

    printf("done\n");

    fclose(merged_file);

    return 0;
}