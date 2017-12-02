
#ifndef HELPERS_H
#define HELPERS_H


int concatenate(int, int);



typedef struct {
    int *productType;
    int data[3];
    pthread_mutex_t lock;
} Inventory;

Inventory new_inventory(int *productType);
void incr_inventory(Inventory *, int product);
void print_inventory(Inventory *, const char *inventoryName);
bool inventory_allowed(Inventory *, int product);




typedef struct {
    size_t count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Tools;

Tools new_tools();
void fetch_tools(Tools *);
void return_tools(Tools *);



#endif