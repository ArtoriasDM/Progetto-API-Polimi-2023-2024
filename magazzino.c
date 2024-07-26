#include <stdio.h>
#include <stdlib.h>

#define STANDARD_INVENTORY_LENGHT 10

typedef struct 
{
    int weight;
    int deadline;
}stock;

typedef struct ingredient_s
{
    char * name;
    stock ** stocks;
    int total;
    int dim;
    int count;
    int height;
    struct ingredient_s * next;
}ingredient;

typedef struct 
{
    ingredient ** ingredients;
    int dim;
    int count;
}inventory;

// inizializazione dell'inventario
void InitializeInventory(inventory * new)
{
    new = malloc(sizeof(inventory));
    new->dim = STANDARD_INVENTORY_LENGHT;
    new->count = 0;
    new->ingredients = calloc(STANDARD_INVENTORY_LENGHT, sizeof(ingredient *));
}

// hash usato nell'inventario
int hash(char * key, int dim)
{
    int i, index;

    index = 0;
    for(i = 0; key[i] != '\0'; i++)
    {
        index += key[i];
    }
    index = index % dim;

    return index;
}

// resize dell'inventario