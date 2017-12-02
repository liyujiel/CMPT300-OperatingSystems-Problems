#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "pcmodel.h"
#include "queue.h"
#include "helpers.h"
#include "workers.h"


// ----------------------------------------------------------------------------------



int main() {

    // Seeds for random number generator
    srand(time(NULL));


    // -------- Initializing required data... --------
    Queue material_buffer = new_ArrayBlockingQueue(QUEUE_CAPACITY__INPUT);
    Queue output_queue = new_ArrayBlockingQueue(QUEUE_CAPACITY__OUTPUT);


    Tools tools = new_tools();
    Status s = new_status();


    int output_productType[3] = {12, 13, 23};
    Inventory output_inventory = new_inventory(output_productType);

    int input_materialType[3] = {1, 2, 3};
    Inventory input_inventory = new_inventory(input_materialType);





    pthread_t generator_thread_id[3];
    pthread_t operator_thread_id[3];


    // -------- Spawning generators --------
    Generator_args generator_args[3];

    for (int i = 0; i < 3; i++) {
        generator_args[i].q = &material_buffer;
        generator_args[i].input_inventory = &input_inventory;
        generator_args[i].material = i + 1;
        generator_args[i].s = &s;
    }

    for (int i = 0; i < 3; i++) {
        pthread_create(&generator_thread_id[i], NULL, __generator, (void *) &generator_args[i]);
    }


    // -------- Spawning operators --------
    Operator_args operator_args = {
        .material_buffer = &material_buffer,
        .output_buffer = &output_queue,
        .tools = &tools,
        .output_inventory = &output_inventory,
        .input_inventory = &input_inventory,
        .s = &s
    };


    for (int i = 0; i < 3; i++) {
        pthread_create(&operator_thread_id[i], NULL, __operator, (void *) &operator_args);
    }


    // -------- Spawning a thread to monior user inputs --------
    pthread_t monitor_thread_id;
    pthread_create(&monitor_thread_id, NULL, monitor, (void *) &s);



    // --------
    for (int i = 0; i < 3; i++) {
        pthread_join(generator_thread_id[i], NULL);
        pthread_join(operator_thread_id[i], NULL);
    }



    return 0;
}