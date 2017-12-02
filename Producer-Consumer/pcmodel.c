#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


#define TOOLS_TOTAL 3
#define TOOLS_THRESHOLD 2
#define QUEUE_CAPACITY__INPUT 10
#define QUEUE_CAPACITY__OUTPUT 100000



// --- Helpers ---

int concatenate(int x, int y) {
    if (x > y) {
        int temp = x;
        x = y;
        y = temp;
    }
    return x * 10 + y;        
}

// Inventory is used to keep how many materials or products have been made.
typedef struct {
    int *productType;
    int data[3];
    pthread_mutex_t lock;
} Inventory;

Inventory new_inventory(int *productType) {
    Inventory i;
    i.productType = productType;
    pthread_mutex_init(&i.lock, NULL);
    return i;
}

void incr_inventory(Inventory *pInventory, int product) {
    int productTypeInd = -1;

    pthread_mutex_lock(&pInventory->lock);
    for (int i = 0; i < 3; i++) {
        if (pInventory->productType[i] == product) {
            productTypeInd = i;
        }
    }

    if (productTypeInd != -1) {
        pInventory->data[productTypeInd]++;
        pthread_mutex_unlock(&pInventory->lock);
    } else { // The input product is not recognized.
        pthread_mutex_unlock(&pInventory->lock);
        return;
    }
}

void print_inventory(Inventory *pInventory, const char *inventoryName) {
    printf("%s: \t", inventoryName);
    for (int i = 0; i < 3; i++) {
        printf("\"%d\"\t", pInventory->productType[i]);
    }
    printf("\nInventory: \t");
    for (int i = 0; i < 3; i++) {
        printf("%d\t", pInventory->data[i]);
    }
    printf("\n");
}

bool inventory_allowed(Inventory *pInventory, int product) {
    int productTypeInd = -1;
    
    pthread_mutex_lock(&pInventory->lock);
    for (int i = 0; i < 3; i++) {
        if (pInventory->productType[i] == product) {
            productTypeInd = i;
        }
    }

    if (productTypeInd != -1) {
        // Construct another array that contains the same data as inventory->data
        int temp[3];
        for (int i = 0; i < 3; i++) {
            temp[i] = pInventory->data[i];
        }

        // Pretend to increment the inventory of the product
        temp[productTypeInd]++;

        // Check if violated "differnece should be less than 10"
        int diff1 = abs(temp[0] - temp[1]);
        int diff2 = abs(temp[1] - temp[2]);
        int diff3 = abs(temp[0] - temp[2]);

        pthread_mutex_unlock(&pInventory->lock);
        return (diff1 <= 10 && diff2 <= 10 && diff3 <= 10);

    } else { // The input product is not recognized.
        pthread_mutex_unlock(&pInventory->lock);
        return false;
    }
}

// --- Tools Implementation ---

typedef struct {
    size_t count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Tools;

Tools new_tools() {
    Tools t;
    t.count = TOOLS_TOTAL;
    pthread_mutex_init(&t.lock, NULL);
    pthread_cond_init(&t.cond, NULL);
    return t;
}

void fetch_tools(Tools *t) {
    pthread_mutex_lock(&t->lock);
    t->count += 2;
    pthread_cond_broadcast(&t->cond);
    pthread_mutex_unlock(&t->lock);
}

void return_tools(Tools *t) {
    pthread_mutex_lock(&t->lock);
    while (t->count < TOOLS_THRESHOLD) {
        pthread_cond_wait(&t->cond, &t->lock);
    }
    t->count -= 2;
    pthread_mutex_unlock(&t->lock);
}


// --- Queue Implementation ---

typedef struct {
    int *data;
    size_t head;
    size_t tail;
    size_t size;
    size_t capacity;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Queue;

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


// --- Pause, Resume Implementation ---
typedef struct {
    bool pause;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Status;


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


// --- Generator Implementation ---

typedef struct {
    Queue *q;
    int material;
    Inventory *input_inventory;
    Status *s;
} Generator_args;

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

typedef struct {
    Queue *material_buffer;
    Queue *output_buffer;
    Tools *tools;
    Inventory *output_inventory;
    Inventory *input_inventory;
    Status *s;
} Operator_args;


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