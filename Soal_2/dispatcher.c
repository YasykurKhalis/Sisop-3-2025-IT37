#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_LINE 1024
#define MAX_DEST 100

void trim_newline(char *str) {
    str[strcspn(str, "\r\n")] = 0;
}

void ensure_directory(const char *dirname) {
    struct stat st = {0};
    if (stat(dirname, &st) == -1) {
        mkdir(dirname, 0755);
    }
}

int main() {
    FILE *fp = fopen("delivery_order.csv", "r");
    if (fp == NULL) {
        perror("Gagal membuka delivery_order.csv");
        return 1;
    }

    char line[MAX_LINE];
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        trim_newline(line);

        char *id = strtok(line, ",");
        char *destination = strtok(NULL, ",");

        if (id == NULL || destination == NULL) continue;

        ensure_directory(destination);

        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/order_%s.csv", destination, destination);

        FILE *out = fopen(filepath, "a");
        if (out == NULL) {
            perror("Gagal membuat file output");
            continue;
        }

        fprintf(out, "%s,%s\n", id, destination);
        fclose(out);
    }

    fclose(fp);
    return 0;
}
