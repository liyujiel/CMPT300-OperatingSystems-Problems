#include "queue.h"
#include "helpers.h"
#include "running_status.h"


#ifndef WORKERS_H
#define WORKERS_H


typedef struct {
    Queue *q;
    int material;
    Inventory *input_inventory;
    Status *s;
} Generator_args;

void *__generator(void *args);



typedef struct {
    Queue *material_buffer;
    Queue *output_buffer;
    Tools *tools;
    Inventory *output_inventory;
    Inventory *input_inventory;
    Status *s;
} Operator_args;

bool is_nextto(Queue *q, int current_item);
void *__operator(void *args);


#endif