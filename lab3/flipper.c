#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

// Zapytać czy dałby rade jakiś statyczny BUFFER_SIZE = 1024

void reverse(char *line)
{
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n')
        len--; // ignore newline \n
    for (size_t i = 0; i < len / 2; i++){
        char temp = line[i];
        line[i] = line[len - 1 - i];
        line[len - 1 - i] = temp;
    }
}

void file_create(const char *src_path, const char *dest_path)
{
    FILE *src_file = fopen(src_path, "r");
    if (!src_file){
        perror("Cant open source file");
        return;
    }

    FILE *dest_file = fopen(dest_path, "w");
    if (!dest_file){
        perror("Cant open result file");
        fclose(src_file);
        return;
    }

    char* buffer = NULL;
    size_t n = 0;
    int line_length = 0;

    while ((line_length = getline(&buffer, &n, src_file)) != -1){
        reverse(buffer);
        fputs(buffer, dest_file);
    }

    free(buffer);
    fclose(src_file);
    fclose(dest_file);
}

int main(int argc, char *argv[])
{
    if (argc != 3){
        fprintf(stderr, "Usage: %s <source_directory> <end_directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* src_dir = argv[1];
    char* dest_dir = argv[2];

    DIR *dir = opendir(src_dir);
    if (!dir){
        perror("Cannot open source directory");
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    struct stat file_stat;

    mkdir(dest_dir, 0777); // Create end directory if does not exist

    while ((entry = readdir(dir))){
        size_t src_size = strlen(src_dir) + strlen(entry->d_name) + 2; // + 2 for artifacts
        size_t dest_size = strlen(dest_dir) + strlen(entry->d_name) + 2;
        char *src_path = malloc(src_size);
        char *dest_path = malloc(dest_size);

        // Check for system memory allocation failure
        if (!src_path || !dest_path) {
            perror("Memory allocation failure");
            free(src_path);
            free(dest_path);
            closedir(dir);
            return EXIT_FAILURE;
        }

        snprintf(src_path, src_size, "%s/%s", src_dir, entry->d_name);
        snprintf(dest_path, dest_size, "%s/%s", dest_dir, entry->d_name);

        if (stat(src_path, &file_stat) == 0 && S_ISREG(file_stat.st_mode)){
            if (strlen(entry->d_name) > 4 && strcmp(entry->d_name + strlen(entry->d_name) - 4, ".txt") == 0){
                file_create(src_path, dest_path);
            }
            
        }
        free(src_path);
        free(dest_path);
    }
    closedir(dir);
    return EXIT_SUCCESS;
}
