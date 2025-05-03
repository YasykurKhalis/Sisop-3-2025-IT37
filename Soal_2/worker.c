#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <destination>\n", argv[0]);
        return 1;
    }

    char *destination = argv[1];
    char input_file[256], output_file[256];
    snprintf(input_file, sizeof(input_file), "%s/order_%s.csv", destination, destination);
    snprintf(output_file, sizeof(output_file), "%s/success_%s.txt", destination, destination);

    FILE *in = fopen(input_file, "r");
    if (!in) {
        perror("Gagal membuka file input");
        return 1;
    }

    int count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), in)) {
        count++;
    }
    fclose(in);

    FILE *out = fopen(output_file, "w");
    if (!out) {
        perror("Gagal membuat file output");
        return 1;
    }
    fprintf(out, "Total order di %s: %d\n", destination, count);
    fclose(out);

    printf("Worker %s selesai memproses %d order.\n", destination, count);
    return 0;
}
