#include <pthread.h>
#include "log.h"

pthread_mutex_t mutex;

void LogInit() {
    pthread_mutex_init(&mutex, NULL);
}

void lock_log() {
    pthread_mutex_lock(&mutex);
}

void unlock_log() {
    pthread_mutex_unlock(&mutex);
}