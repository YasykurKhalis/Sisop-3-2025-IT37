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
#define BUFFER_SIZE 1048576  // 1MB buffer
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
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket error: %s\n", strerror(errno));
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection failed: %s\n", strerror(errno));
        return -1;
    }

    return sock;
}

void list_local_files() {
    DIR *dir;
    struct dirent *ent;
    printf("\nLocal text files:\n");
    
    if ((dir = opendir(SECRETS_DIR)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".txt")) {
                printf("- %s\n", ent->d_name);
            }
        }
        closedir(dir);
    } else {
        printf("Cannot open secrets directory\n");
    }
}

void decrypt_file(int sock) {
    list_local_files();
    
    char filename[256];
    printf("\nEnter filename: ");
    scanf("%255s", filename);
    
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", SECRETS_DIR, filename);
    
    FILE *file = fopen(path, "r");
    if (!file) {
        printf("Error opening file: %s\n", strerror(errno));
        return;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *content = malloc(size + 1);
    if (!content) {
        fclose(file);
        printf("Memory error\n");
        return;
    }
    
    fread(content, 1, size, file);
    content[size] = '\0';
    fclose(file);
    
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "DECRYPT|%s|%s", filename, content);
    free(content);
    
    if (send(sock, command, strlen(command), 0) < 0) {
        printf("Send error\n");
        return;
    }
    
    char response[BUFFER_SIZE] = {0};
    ssize_t bytes = read(sock, response, BUFFER_SIZE - 1);
    if (bytes <= 0) {
        printf("No response from server\n");
        return;
    }
    response[bytes] = '\0';
    
    char *status = strtok(response, "|");
    char *info = strtok(NULL, "|");
    
    if (status && info) {
        if (strcmp(status, "OK") == 0) {
            printf("Success! Saved as: %s\n", info);
        } else {
            printf("Error: %s\n", info);
        }
    } else {
        printf("Invalid response\n");
    }
}

void list_server_files(int sock) {
    if (send(sock, "LIST", 4, 0) < 0) {
        printf("Send error\n");
        return;
    }
    
    char list[BUFFER_SIZE] = {0};
    ssize_t bytes = read(sock, list, BUFFER_SIZE - 1);
    if (bytes <= 0) {
        printf("No response\n");
        return;
    }
    list[bytes] = '\0';
    
    printf("\nAvailable JPEG files on server:\n%s", list);
}

void download_file(int sock) {
    list_server_files(sock);
    
    char filename[256];
    printf("\nEnter filename to download: ");
    scanf("%255s", filename);
    
    char command[512];
    snprintf(command, sizeof(command), "DOWNLOAD|%s", filename);
    
    if (send(sock, command, strlen(command), 0) < 0) {
        printf("Send error\n");
        return;
    }
    
    // First read the response with file size
    char size_response[256] = {0};
    ssize_t bytes = read(sock, size_response, 255);
    if (bytes <= 0) {
        printf("No response\n");
        return;
    }
    size_response[bytes] = '\0';
    
    char *status = strtok(size_response, "|");
    char *size_str = strtok(NULL, "|");
    
    if (!status || !size_str || strcmp(status, "OK") != 0) {
        printf("Error: %s\n", size_str ? size_str : "Invalid response");
        return;
    }
    
    long file_size = atol(size_str);
    if (file_size <= 0) {
        printf("Invalid file size\n");
        return;
    }
    
    // Now read the actual file data
    char *file_data = malloc(file_size);
    if (!file_data) {
        printf("Memory error\n");
        return;
    }
    
    long received = 0;
    while (received < file_size) {
        bytes = read(sock, file_data + received, file_size - received);
        if (bytes <= 0) {
            free(file_data);
            printf("Download interrupted\n");
            return;
        }
        received += bytes;
    }
    
    char output_path[512];
    snprintf(output_path, sizeof(output_path), "%s/%s", OUTPUT_DIR, filename);
    
    FILE *file = fopen(output_path, "wb");
    if (!file) {
        free(file_data);
        printf("Cannot create file\n");
        return;
    }
    
    fwrite(file_data, 1, file_size, file);
    fclose(file);
    free(file_data);
    
    printf("Success! Downloaded to: %s\n", output_path);
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
                printf("Please start the server first\n");
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
                send(sock, "EXIT", 4, 0);
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
