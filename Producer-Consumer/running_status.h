
#ifndef RUNNING_STATUS_H
#define RUNNING_STATUS_H


typedef struct {
    bool pause;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Status;

Status new_status();
bool is_running(Status *);
void mypause(Status *);
void resume(Status *);
void *monitor(void *status);

#endif