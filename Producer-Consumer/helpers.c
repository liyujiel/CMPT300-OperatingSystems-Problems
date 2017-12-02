#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "pcmodel.h"
#include "helpers.h"



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
