
#ifndef QUEUE_H
#define QUEUE_H

typedef struct {
    int *data;
    size_t head;
    size_t tail;
    size_t size;
    size_t capacity;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Queue;


Queue new_ArrayBlockingQueue(size_t capacity);
bool is_empty(Queue *);
bool is_full(Queue *);
void enqueue(Queue *, int item);
int dequeue(Queue *);
int last(Queue *);
void print_queue(Queue *);

#endif