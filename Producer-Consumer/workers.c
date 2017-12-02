#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include "workers.h"


// --- Generator Implementation ---


void *__generator(void *args) {
    Generator_args *param = (Generator_args *) args;
    int material = param->material;

    while (true) {
        if (is_running(param->s)) {
            enqueue(param->q, material);
            incr_inventory(param->input_inventory, material);
        }
    }

}



// --- Operator Implementation ---


bool is_nextto(Queue *q, int current_item) {
    int last_item = last(q);
    if (last_item == -1) {
        return false;
    } else if (last_item == current_item) {
        return true;
    } else {
        return false;
    }
}




void *__operator(void *args) {
    Operator_args *param = (Operator_args *) args;
    Queue *material_buffer = param->material_buffer;
    Queue *output_buffer = param->output_buffer;
    Tools *t = param->tools;
    Inventory *output_inventory = param->output_inventory;
    Inventory *input_inventory = param->input_inventory;

    while (true) {
        if (is_running(param->s)) {

            fetch_tools(t);

            int material_a = dequeue(material_buffer);
            int material_b = dequeue(material_buffer);

            // We cannot use 2 same materials to produce product.
            if (material_a == material_b) {
                return_tools(t);
                continue;
            }

            int product = concatenate(material_a, material_b);


            // Producing product...
            int usec = rand() % 1000000 - 10000;
            usleep(usec);

            if (inventory_allowed(output_inventory, product) && 
                !is_nextto(output_buffer, product)) {
                
                enqueue(output_buffer, product);
                incr_inventory(output_inventory, product);
                

                // Print information

                // To clear console output
                // Adapted from: 
                // https://stackoverflow.com/questions/17271576/clear-screen-in-c-and-c-on-unix-based-system
                printf("\033[H\033[J");

                printf("Input Buffer: ");
                print_queue(material_buffer);
                printf("\n\n");

                printf("Output Buffer: ");
                print_queue(output_buffer);
                printf("\n\n");

                printf("Materials Produced: \n----------------------------------------\n");
                print_inventory(input_inventory, "Material Name");
                printf("\n\n");

                printf("Products Produced: \n----------------------------------------\n");
                print_inventory(output_inventory, "Product Name");

                
                if (param->s->pause) {
                    printf("\n\n** Execution paused. Press 'r' and 'Enter' quickly to resume. **\n");
                } else {
                    printf("\n\n** Press 'p' and 'Enter' quickly to pause. **\n");
                }


                return_tools(t);

            } else { // Production criteria not met, will disgard the materials and return tools.
                return_tools(t);
            }

        }
    }
}
