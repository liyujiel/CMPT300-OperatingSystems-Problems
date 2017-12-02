#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "running_status.h"



// --- Pause, Resume Implementation ---


Status new_status() {
    Status s;
    s.pause = false;
    pthread_mutex_init(&s.lock, NULL);
    pthread_cond_init(&s.cond, NULL);
    return s;
}

bool is_running(Status *s) {
    pthread_mutex_lock(&s->lock);
    if (!s->pause) {
        pthread_mutex_unlock(&s->lock);
        return true;
    }
    while (s->pause) {
        pthread_cond_wait(&s->cond, &s->lock);
    }
    pthread_mutex_unlock(&s->lock);
    return true;
}

void mypause(Status *s) {
    pthread_mutex_lock(&s->lock);
    s->pause = true;
    pthread_mutex_unlock(&s->lock);
}

void resume(Status *s) {
    pthread_mutex_lock(&s->lock);
    s->pause = false;
    pthread_cond_broadcast(&s->cond);
    pthread_mutex_unlock(&s->lock);
}

void *monitor(void *status) {
    Status *s = (Status *) status;
    char input;
    while (true) {
        input = getchar();
        switch (input) {
            case 'p':
                mypause(s); break;
            case 'r':
                resume(s); break;
            default: break;
        }
    }
    return NULL;
}