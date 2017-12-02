#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "queue.h"


// --- Queue Implementation ---


Queue new_ArrayBlockingQueue(size_t capacity) {
    Queue q = {
        .data = malloc(sizeof(int) * capacity),
        .head = 0,
        .tail = 0,
        .size = 0,
        .capacity = capacity
    };
    pthread_mutex_init(&q.lock, NULL);
    pthread_cond_init(&q.cond, NULL);
    return q;
}

bool is_empty(Queue *q) {
    return q && q->size == 0;
}

bool is_full(Queue *q) {
    return q && q->size == q->capacity;
}

void enqueue(Queue *q, int item) {
    pthread_mutex_lock(&q->lock);
    while (!q || is_full(q)) {
        pthread_cond_wait(&q->cond, &q->lock);
    }

    q->data[q->tail] = item;
    q->tail = (q->tail + 1) % q->capacity;
    q->size++;
    pthread_cond_broadcast(&q->cond);
    pthread_mutex_unlock(&q->lock);
}

int dequeue(Queue *q) {
    pthread_mutex_lock(&q->lock);
    while (!q || is_empty(q)) {
        pthread_cond_wait(&q->cond, &q->lock);
    }

    int result = q->data[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->size--;
    pthread_cond_broadcast(&q->cond);
    pthread_mutex_unlock(&q->lock);
    return result;
}

int last(Queue *q) {
    pthread_mutex_lock(&q->lock);
    if (q->size == 0) {
        pthread_mutex_unlock(&q->lock);
        return -1;
    }

    int result = q->data[q->tail - 1];
    pthread_mutex_unlock(&q->lock);
    return result;
}

void print_queue(Queue* q) {
    size_t index;
    size_t tmp;

    if (!q) {
        printf("null");
        return;
    }
    printf("[");

    if (q->size >= 1) {
        printf("%d", (int) q->data[q->head]);
    }
    for (index = 1; index < q->size; ++index) {
        tmp = (q->head + index) % q->capacity;
        printf(", %d", (int) q->data[tmp]);
    }
    printf("]\n");
}
