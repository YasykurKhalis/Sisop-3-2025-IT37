#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>

#define PORT 8080
#define BUFFER_SIZE 1048576  // 1MB buffer for large files
#define DATABASE_DIR "database"
#define LOG_FILE "server.log"

void log_action(const char *source, const char *action, const char *info) {
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    
    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
        fclose(log);
    }
}

int hex_to_bin(const char *hex, unsigned char *bin, size_t len) {
    for (size_t i = 0; i < len; i += 2) {
        if (!isxdigit(hex[i]) || !isxdigit(hex[i+1])) {
            return -1;  // Invalid hex digit
        }
        sscanf(hex + i, "%2hhx", &bin[i/2]);
    }
    return 0;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    ssize_t valread;
    
    valread = read(client_socket, buffer, BUFFER_SIZE - 1);
    if (valread <= 0) {
        log_action("Server", "ERROR", "Failed to read from client");
        return;
    }
    buffer[valread] = '\0';
    
    char *command = strtok(buffer, "|");
    char *filename = strtok(NULL, "|");
    char *data = strtok(NULL, "|");
    
    if (!command || !filename || !data) {
        strcpy(response, "ERROR|Invalid command format");
        send(client_socket, response, strlen(response), 0);
        return;
    }

    if (strcmp(command, "DECRYPT") == 0) {
        log_action("Client", "DECRYPT", filename);
        
        // Reverse the hex string
        size_t len = strlen(data);
        for (size_t i = 0; i < len / 2; i++) {
            char temp = data[i];
            data[i] = data[len - i - 1];
            data[len - i - 1] = temp;
        }
        
        // Convert hex to binary
        size_t bin_len = len / 2;
        unsigned char *bin_data = malloc(bin_len);
        if (!bin_data || hex_to_bin(data, bin_data, len) != 0) {
            strcpy(response, "ERROR|Invalid hex data");
            log_action("Server", "ERROR", "Invalid hex data");
            free(bin_data);
            send(client_socket, response, strlen(response), 0);
            return;
        }
        
        // Save as JPEG
        time_t timestamp = time(NULL);
        char jpeg_filename[256];
        snprintf(jpeg_filename, sizeof(jpeg_filename), "%s/%ld.jpeg", DATABASE_DIR, timestamp);
        
        FILE *file = fopen(jpeg_filename, "wb");
        if (file) {
            size_t written = fwrite(bin_data, 1, bin_len, file);
            fclose(file);
            
            if (written == bin_len) {
                snprintf(response, sizeof(response), "OK|%ld.jpeg", timestamp);
                log_action("Server", "SAVE", jpeg_filename);
            } else {
                strcpy(response, "ERROR|Failed to write file");
                log_action("Server", "ERROR", "Failed to write file");
                remove(jpeg_filename);
            }
        } else {
            strcpy(response, "ERROR|Failed to create file");
            log_action("Server", "ERROR", "Failed to create file");
        }
        
        free(bin_data);
    }
    else if (strcmp(command, "DOWNLOAD") == 0) {
        log_action("Client", "DOWNLOAD", filename);
        
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/%s", DATABASE_DIR, filename);
        
        FILE *file = fopen(filepath, "rb");
        if (file) {
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            char *file_data = malloc(file_size);
            if (file_data) {
                size_t items_read = fread(file_data, 1, file_size, file);
                fclose(file);
                
                if (items_read == file_size) {
                    snprintf(response, sizeof(response), "OK|%ld", file_size);
                    send(client_socket, response, strlen(response), 0);
                    send(client_socket, file_data, file_size, 0);
                    log_action("Server", "UPLOAD", filename);
                } else {
                    strcpy(response, "ERROR|Failed to read file");
                    send(client_socket, response, strlen(response), 0);
                    log_action("Server", "ERROR", "Failed to read file");
                }
                free(file_data);
                return;
            }
            fclose(file);
        }
        strcpy(response, "ERROR|File not found");
        log_action("Server", "ERROR", "File not found");
    }
    else if (strcmp(command, "EXIT") == 0) {
        log_action("Client", "EXIT", "Client requested to exit");
        strcpy(response, "OK|Goodbye");
    }
    else {
        strcpy(response, "ERROR|Unknown command");
        log_action("Server", "ERROR", "Unknown command");
    }
    
    send(client_socket, response, strlen(response), 0);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    // Daemonize
    pid_t pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    umask(0);
    
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            log_action("Server", "ERROR", "Accept failed");
            continue;
        }
        
        pid_t child_pid = fork();
        if (child_pid == 0) {
            close(server_fd);
            handle_client(new_socket);
            close(new_socket);
            exit(0);
        } else if (child_pid > 0) {
            close(new_socket);
        } else {
            log_action("Server", "ERROR", "Fork failed");
        }
    }
    
    return 0;
}
