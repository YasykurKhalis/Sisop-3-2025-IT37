#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1048576  // 1MB buffer for large files
#define SECRETS_DIR "secrets"
#define OUTPUT_DIR "."

void display_menu() {
    printf("\n=== The Legend of Rootkids ===\n");
    printf("1. Decrypt text file\n");
    printf("2. Download JPEG file\n");
    printf("3. Exit\n");
    printf("============================\n");
    printf("Choose option: ");
}

int connect_to_server() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error: %s\n", strerror(errno));
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported\n");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed: %s\n", strerror(errno));
        return -1;
    }
    
    return sock;
}

void list_text_files() {
    DIR *d;
    struct dirent *dir;
    int count = 0;
    
    printf("\nAvailable text files:\n");
    d = opendir(SECRETS_DIR);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strstr(dir->d_name, ".txt")) {
                printf("%d. %s\n", ++count, dir->d_name);
            }
        }
        closedir(d);
    }
    
    if (count == 0) {
        printf("No text files found in %s directory\n", SECRETS_DIR);
    }
}

void decrypt_file(int sock) {
    list_text_files();
    
    char filename[256];
    printf("\nEnter filename (e.g., input_1.txt): ");
    if (scanf("%255s", filename) != 1) {
        printf("Invalid input\n");
        while (getchar() != '\n');
        return;
    }
    
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", SECRETS_DIR, filename);
    
    FILE *file = fopen(filepath, "r");
    if (!file) {
        printf("Error: Cannot open file %s: %s\n", filepath, strerror(errno));
        return;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *file_data = malloc(file_size + 1);
    if (!file_data) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return;
    }
    
    size_t items_read = fread(file_data, 1, file_size, file);
    fclose(file);
    
    if (items_read != file_size) {
        printf("Error: Failed to read file completely\n");
        free(file_data);
        return;
    }
    file_data[file_size] = '\0';
    
    // Prepare command
    char *command = malloc(strlen("DECRYPT|") + strlen(filename) + 1 + file_size + 1);
    if (!command) {
        printf("Error: Memory allocation failed\n");
        free(file_data);
        return;
    }
    sprintf(command, "DECRYPT|%s|%s", filename, file_data);
    free(file_data);
    
    if (send(sock, command, strlen(command), 0) < 0) {
        printf("Error: Failed to send command: %s\n", strerror(errno));
        free(command);
        return;
    }
    free(command);
    
    char response[BUFFER_SIZE] = {0};
    ssize_t n = read(sock, response, BUFFER_SIZE - 1);
    if (n <= 0) {
        printf("Error: Failed to get server response\n");
        return;
    }
    response[n] = '\0';
    
    char *status = strtok(response, "|");
    char *info = strtok(NULL, "|");
    
    if (status && info) {
        if (strcmp(status, "OK") == 0) {
            printf("Success! File saved as: %s\n", info);
        } else {
            printf("Error: %s\n", info);
        }
    } else {
        printf("Error: Invalid server response\n");
    }
}

void download_file(int sock) {
    char filename[256];
    printf("\nEnter filename to download (e.g., 1744401282.jpeg): ");
    if (scanf("%255s", filename) != 1) {
        printf("Invalid input\n");
        while (getchar() != '\n');
        return;
    }
    
    char command[512];
    snprintf(command, sizeof(command), "DOWNLOAD|%s", filename);
    
    if (send(sock, command, strlen(command), 0) < 0) {
        printf("Error: Failed to send command: %s\n", strerror(errno));
        return;
    }
    
    char response[BUFFER_SIZE] = {0};
    ssize_t n = read(sock, response, BUFFER_SIZE - 1);
    if (n <= 0) {
        printf("Error: Failed to get server response\n");
        return;
    }
    response[n] = '\0';
    
    char *status = strtok(response, "|");
    char *size_str = strtok(NULL, "|");
    
    if (status && size_str && strcmp(status, "OK") == 0) {
        long file_size = atol(size_str);
        if (file_size <= 0) {
            printf("Error: Invalid file size\n");
            return;
        }
        
        char *file_data = malloc(file_size);
        if (!file_data) {
            printf("Error: Memory allocation failed\n");
            return;
        }
        
        long bytes_received = 0;
        while (bytes_received < file_size) {
            n = read(sock, file_data + bytes_received, file_size - bytes_received);
            if (n <= 0) {
                printf("Error: Failed to receive file data\n");
                free(file_data);
                return;
            }
            bytes_received += n;
        }
        
        char output_path[512];
        snprintf(output_path, sizeof(output_path), "%s/%s", OUTPUT_DIR, filename);
        
        FILE *output = fopen(output_path, "wb");
        if (output) {
            size_t written = fwrite(file_data, 1, file_size, output);
            fclose(output);
            
            if (written == file_size) {
                printf("Success! File downloaded to: %s\n", output_path);
            } else {
                printf("Error: Failed to write complete file\n");
                remove(output_path);
            }
        } else {
            printf("Error: Cannot create file %s: %s\n", output_path, strerror(errno));
        }
        
        free(file_data);
    } else {
        printf("Error: %s\n", size_str ? size_str : "Invalid server response");
    }
}

int main() {
    int option;
    
    while (1) {
        display_menu();
        if (scanf("%d", &option) != 1) {
            printf("Invalid input\n");
            while (getchar() != '\n');
            continue;
        }
        
        int sock = connect_to_server();
        if (sock < 0) {
            if (option != 3) {
                printf("Please make sure the server is running\n");
            }
            if (option == 3) break;
            continue;
        }
        
        switch (option) {
            case 1:
                decrypt_file(sock);
                break;
            case 2:
                download_file(sock);
                break;
            case 3:
                send(sock, "EXIT", strlen("EXIT"), 0);
                printf("Goodbye!\n");
                close(sock);
                return 0;
            default:
                printf("Invalid option\n");
        }
        
        close(sock);
    }
    
    return 0;
}
