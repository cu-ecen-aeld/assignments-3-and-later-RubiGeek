// filepath: /server/aesdsocket.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <fcntl.h>
#include <netinet/in.h>

#define PORT 9000      
#define SOCKET_PATH "/tmp/aesdsocket.sock"
#define DATA_FILE "/var/tmp/aesdsocketdata"

int server_socket;
int client_socket;

void handle_signal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        system("rm -f " DATA_FILE);
        close(server_socket);
        unlink(SOCKET_PATH);
        exit(0);
    }
}

void log_message(const char *message) {
    syslog(LOG_INFO, "%s", message);
}

void append_to_file(const char *data) {
    int fd = open(DATA_FILE, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd != -1) {
        write(fd, data, strlen(data));
        close(fd);
    }
}

void send_file_content(int client_socket) {
    char buffer[1024];
    int fd = open(DATA_FILE, O_RDONLY);
    if (fd != -1) {
        ssize_t bytes_read;
        while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            send(client_socket, buffer, bytes_read, 0);
        }
        close(fd);
    }
}

int main(int argc, char *argv[]) {

    struct sockaddr_in server_addr;
    int daemon_opt = 0;
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    int opt;
      // Handle command line Add commentMore actions
    while ((opt = getopt(argc, argv, "dk")) != -1) {
        switch (opt) {
        case 'd':
            daemon_opt = 1;
            break;
        default: /* '?' */
            break;
        }
    }
    system("rm -f " DATA_FILE);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);      
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    //strncpy(server_addr.sin_path, SOCKET_PATH, sizeof(server_addr.sin_path) - 1);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (daemon_opt) {
        switch(fork()) {
        case -1:
            perror("fork");
            close(server_socket);
            exit(EXIT_FAILURE);
        case 0: // Child process
            break;
        default: // Parent process
            exit(EXIT_SUCCESS);
        }
        // Create a new session and detach from terminal
        if (setsid() == -1) {
            perror("setsid");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        // Redirect standard input, output, and error to /dev/null
        freopen("/dev/null", "r", stdin);
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);
        if (daemon(0, 0) == -1) {
            perror("daemon");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
    }
    if (listen(server_socket, 5) == -1) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    log_message("Server is listening...");

    while (1) {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }

        log_message("Client connected.");
        char buffer[1024*1000];
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        log_message("Received data from client.");
        if (bytes_received > 0) {
            
            buffer[bytes_received] = '\0';
            append_to_file(buffer);
            send_file_content(client_socket);
        }   
        
        
        log_message("Client disconnected.");
        close(client_socket);
    }

    return 0;
}