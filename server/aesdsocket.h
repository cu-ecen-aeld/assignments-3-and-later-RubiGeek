#include <stdbool.h>
#include <pthread.h>
#include <sys/queue.h>

struct thread_data {
    pthread_mutex_t *mutex;
    pthread_t thread_id;
    bool thread_complete_success;
};

struct entry {
    struct thread_data data;
    SLIST_ENTRY(entry) entries;
};

SLIST_HEAD(slisthead, entry);
struct entry *datap = NULL;