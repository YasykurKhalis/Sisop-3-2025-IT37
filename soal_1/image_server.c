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
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1048576  // 1MB buffer
#define DATABASE_DIR "database"
#define LOG_FILE "server.log"

void log_action(const char *source, const char *action, const char *info) {
    time_t now = time(NULL);
    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        fprintf(log, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
        fclose(log);
    }
}

void hex_to_bin(const char *hex, unsigned char *bin, size_t len) {
    for (size_t i = 0; i < len; i += 2) {
        sscanf(hex + i, "%2hhx", &bin[i/2]);
    }
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    
    // Read client command
    ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        log_action("Server", "ERROR", "Read failed");
        return;
    }
    buffer[bytes_read] = '\0';

    char *command = strtok(buffer, "|");
    char *filename = strtok(NULL, "|");
    char *data = strtok(NULL, "");  // Get remaining data

    if (!command) {
        send(client_socket, "ERROR|Invalid command", 21, 0);
        return;
    }

    if (strcmp(command, "DECRYPT") == 0 && filename && data) {
        log_action("Client", "DECRYPT", filename);
        
        // Remove newlines from data if any
        char *p = data;
        while (*p) {
            if (*p == '\n' || *p == '\r') *p = ' ';
            p++;
        }

        // Reverse the hex string
        size_t len = strlen(data);
        for (size_t i = 0; i < len/2; i++) {
            char temp = data[i];
            data[i] = data[len-1-i];
            data[len-1-i] = temp;
        }

        // Convert hex to binary
        if (len % 2 != 0) {
            send(client_socket, "ERROR|Invalid hex length", 24, 0);
            return;
        }

        size_t bin_len = len/2;
        unsigned char *bin_data = malloc(bin_len);
        if (!bin_data) {
            send(client_socket, "ERROR|Memory error", 18, 0);
            return;
        }

        hex_to_bin(data, bin_data, len);

        // Save JPEG
        time_t timestamp = time(NULL);
        char jpeg_path[256];
        snprintf(jpeg_path, sizeof(jpeg_path), "%s/%ld.jpeg", DATABASE_DIR, timestamp);

        FILE *jpeg_file = fopen(jpeg_path, "wb");
        if (!jpeg_file) {
            free(bin_data);
            send(client_socket, "ERROR|File creation failed", 26, 0);
            return;
        }

        fwrite(bin_data, 1, bin_len, jpeg_file);
        fclose(jpeg_file);
        free(bin_data);

        snprintf(response, sizeof(response), "OK|%ld.jpeg", timestamp);
        log_action("Server", "SAVE", jpeg_path);
    }
    else if (strcmp(command, "DOWNLOAD") == 0 && filename) {
        log_action("Client", "DOWNLOAD", filename);
        
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/%s", DATABASE_DIR, filename);

        FILE *file = fopen(filepath, "rb");
        if (!file) {
            send(client_socket, "ERROR|File not found", 20, 0);
            return;
        }

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *file_data = malloc(file_size);
        if (!file_data) {
            fclose(file);
            send(client_socket, "ERROR|Memory error", 18, 0);
            return;
        }

        if (fread(file_data, 1, file_size, file) != file_size) {
            fclose(file);
            free(file_data);
            send(client_socket, "ERROR|Read error", 16, 0);
            return;
        }
        fclose(file);

        // Send file size first
        snprintf(response, sizeof(response), "OK|%ld", file_size);
        send(client_socket, response, strlen(response), 0);

        // Then send file data
        send(client_socket, file_data, file_size, 0);
        free(file_data);
        log_action("Server", "UPLOAD", filename);
        return;
    }
    else if (strcmp(command, "LIST") == 0) {
        DIR *dir;
        struct dirent *ent;
        char list[BUFFER_SIZE] = {0};

        if ((dir = opendir(DATABASE_DIR)) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                if (strstr(ent->d_name, ".jpeg")) {
                    strcat(list, ent->d_name);
                    strcat(list, "\n");
                }
            }
            closedir(dir);
            send(client_socket, list, strlen(list), 0);
        } else {
            send(client_socket, "No files found", 14, 0);
        }
        return;
    }
    else if (strcmp(command, "EXIT") == 0) {
        send(client_socket, "OK|Goodbye", 10, 0);
        log_action("Client", "EXIT", "Client exit");
    }
    else {
        send(client_socket, "ERROR|Invalid command", 21, 0);
    }
}

int main() {
    // Create database directory if not exists
    mkdir(DATABASE_DIR, 0755);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
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
    if (fork() > 0) {
        exit(EXIT_SUCCESS);
    }

    while (1) {
        int new_socket;
        int addrlen = sizeof(address);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            log_action("Server", "ERROR", "Accept failed");
            continue;
        }

        if (fork() == 0) {
            close(server_fd);
            handle_client(new_socket);
            close(new_socket);
            exit(0);
        }
        close(new_socket);
    }

    return 0;
}
