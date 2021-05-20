#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
    FILE* merged_file = fopen(argv[1], "rb");

    if (merged_file == NULL) {
        printf("\nFile Open Error\n");
        return 0;
    }

    int key = atoi(argv[2]);

    printf("decode with key 0x%x\n", key);

    mkdir(argv[3], 0777);

    printf("make directory \"%s\"\n", argv[3]);

    fseek(merged_file, 0, SEEK_END);
    int merged_file_size = ftell(merged_file);
    rewind(merged_file);

    while(1){
        char file_mata[100];
        char* file_name;
        int file_size;

        fgets(file_mata, 100, merged_file);

        file_name = strtok(file_mata, " ");
        sscanf(strtok(NULL, " "), "%d", &file_size);
        
        char output_dir[100] = "";
        strcat(output_dir, argv[3]);
        strcat(output_dir, "/");
        strcat(output_dir, file_name);

        FILE* source_file = fopen(output_dir, "wb");

        for (int i = 0; i < file_size; ++i) {
            fputc(fgetc(merged_file) ^ key, source_file);
        }

        printf("split %s, size %d\n", file_name, file_size);

        fclose(source_file);
        
        if(ftell(merged_file) == merged_file_size){
            break;
        }
    }

    printf("done\n");

    fclose(merged_file);

    return 0;
}