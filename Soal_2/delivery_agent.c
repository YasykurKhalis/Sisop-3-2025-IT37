#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_DEST 100

int main() {
    FILE *fp = fopen("delivery_order.csv", "r");
    if (!fp) {
        perror("Gagal membuka delivery_order.csv");
        return 1;
    }

    char line[1024];
    char *destinations[MAX_DEST];
    int num_dest = 0;

    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        char *token = strtok(line, ",");      
        char *destination = strtok(NULL, ","); 
        if (destination) {
            destination[strcspn(destination, "\r\n")] = 0; 

            int found = 0;
            for (int i = 0; i < num_dest; i++) {
                if (strcmp(destinations[i], destination) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                destinations[num_dest] = strdup(destination);
                num_dest++;
            }
        }
    }
    fclose(fp);

    for (int i = 0; i < num_dest; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            execl("./worker", "worker", destinations[i], NULL);
            perror("exec gagal");
            exit(1);
        }
    }

    for (int i = 0; i < num_dest; i++) {
        wait(NULL);
    }

    printf("Semua order selesai diproses.\n");

    for (int i = 0; i < num_dest; i++) {
        free(destinations[i]);
    }

    return 0;
}
