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
#include "aesdsocket.h"
#include <string.h>
#include <sys/time.h>
#define PORT 9000    
#define SOCKET_PATH "/tmp/aesaesdsocketdsocket.sock"
#define DATA_FILE "/var/tmp/aesdsocketdata"
#define LOG_FILE "/var/tmp/temp_log"
#define TIMER_INTERVAL_SEC 10  // Timer interval (10 seconds)

int server_socket;
int client_socket;
pthread_mutex_t *mutex;
struct slisthead head = SLIST_HEAD_INITIALIZER(head);

void handle_signal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        system("rm -f " DATA_FILE);
        close(server_socket);
        unlink(SOCKET_PATH);
        exit(0);
    }
}

void cleanup_list() {
    struct entry *current = head.slh_first;
    while (current != NULL) {
        struct entry *next = current->entries.sle_next;
        free(current);
        current = next;
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

void process_data(void) {
    char buffer[1024*1000];
    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    log_message("Received data from client.");
    if (bytes_received > 0) {
        
        buffer[bytes_received] = '\0';
        append_to_file(buffer);
        send_file_content(client_socket);
    }   
}

void* trhead_function(void *thread_param) { 
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    int rc = 0;
    if (thread_func_args == NULL) {
        perror("Thread function received NULL thread_func_args");
        pthread_exit(NULL);
    }
    thread_func_args->thread_id = pthread_self();
    rc = pthread_mutex_lock(thread_func_args->mutex);
    if (rc != 0) {
        perror("Failed to lock mutex, error code");
        thread_func_args->thread_complete_success = false;
        pthread_exit(NULL);
    }
    process_data();
    log_message("Client disconnected.");
    close(client_socket);
    rc = pthread_mutex_unlock(thread_func_args->mutex);
    if (rc != 0) {
        perror("Failed to unlock mutex");
        thread_func_args->thread_complete_success = false;
        pthread_exit(NULL);
    }
    
    thread_func_args->thread_complete_success = true;   
    return thread_param;
}

void timer_handler(int sig) {
    (void)sig; // Suppress unused warnings

    if (pthread_mutex_lock(mutex)) {
        perror("pthread_mutex_lock");
        return;
    }

    time_t now = time(NULL);
    struct tm tm = *localtime(&now);
    char timestamp[64];
    char timestamp_full[128] = "timestamp:";
    strftime(timestamp, sizeof(timestamp), "%a, %d %b %Y %H:%M:%S %z", &tm);
    strcat(timestamp_full,timestamp);
    strcat(timestamp_full,"\n");
    append_to_file(timestamp_full);
    // Exit critical section
    if (pthread_mutex_unlock(mutex)) {
        perror("pthread_mutex_unlock");
    }
    printf("Wrote: %s\n", timestamp);
}

int main(int argc, char *argv[]) {
    printf("Starting AESD Socket Server...\n");

    struct sockaddr_in server_addr;
    int daemon_opt = 0;
    int opt;

    // Handle command line Add commentMore actions
    while ((opt = getopt(argc, argv, "dk")) != -1) {
        switch (opt) {
        case 'd':
            daemon_opt = 1;
            break;
        case 'k':
            handle_signal(SIGTERM);
            exit(EXIT_SUCCESS);
        default: /* '?' */
            break;
        }
    }

    // Create a mutex for thread synchronization
    mutex = malloc(sizeof(pthread_mutex_t));
    if (mutex == NULL) {
        perror("Failed to allocate memory for mutex");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(mutex, NULL) != 0) {
        perror("Failed to initialize mutex");
        free(mutex);
        exit(EXIT_FAILURE);
    }

    // Remove the data file if it exists
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
    // Initialize the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);      
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    // Bind the socket to the address and port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_socket);
        exit(EXIT_FAILURE);   
    }



    atexit(cleanup_list);
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (daemon_opt) {
        printf("Working in daemon option\n");
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
        freopen(LOG_FILE, "r", stdin);
        freopen(LOG_FILE, "w", stdout);
        freopen(LOG_FILE, "w", stderr);
        
        if (daemon(0, 0) == -1) {
            perror("daemon");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        
        printf("Daemon started successfully.\n");
    }

    struct itimerval timer;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = timer_handler;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }

    // Configure the timer (first after 10s, then every 10s)
    timer.it_value.tv_sec = 10;      // First timeout after 10 seconds
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 10;   // Repeat every 10 seconds
    timer.it_interval.tv_usec = 0;

    // Start the timer
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        append_to_file("setitimer failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, SOMAXCONN) == -1) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    log_message("Server is listening...");

    while (1) {
        log_message("Waiting for clients.\n");
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }
        log_message("Client connected.");
        struct entry *node = malloc(sizeof(struct entry));
        if (node == NULL) {
            perror("Failed to allocate memory for thread_node");
            close(client_socket);
            continue;
        }
        node->data.mutex = mutex;
        node->data.thread_complete_success = false;
        
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork"); 
            close(client_socket);
            free(node);
            continue;
        }
        else if (pid == 0) {
            if (pthread_create(&node->data.thread_id, NULL, trhead_function, &node->data) != 0) {
                perror("Failed to create thread");
                close(client_socket);
                exit(EXIT_FAILURE);          
            }
        }
        else{
            SLIST_INSERT_HEAD(&head, node, entries);
            SLIST_FOREACH(datap, &head, entries) {
                if (datap->data.thread_complete_success) {
                    pthread_join(datap->data.thread_id, NULL);
                    SLIST_REMOVE(&head, node, entry, entries);
                    free(node);
                }
            }
        }
    }

    return 0;
}