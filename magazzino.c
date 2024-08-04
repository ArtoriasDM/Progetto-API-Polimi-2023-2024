#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "magazzino.h"

// inizializazione dell'inventario
void InitializeInventory(Inventory * new)
{
    new = malloc(sizeof(Inventory));
    new->dim = STANDARD_INVENTORY_LENGHT;
    new->count = 0;
    new->ingredients = calloc(STANDARD_INVENTORY_LENGHT, sizeof(Ingredient *));
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
void ResizeInventory(Inventory * magazzino)
{
    int i, index, new_size;
    Ingredient ** old_table;
    Ingredient * el, * curr, * next;

    old_table = magazzino->ingredients;
    new_size = magazzino->dim * 2;
    magazzino->ingredients = calloc(new_size, sizeof(Ingredient *));

    for(i = 0; i < magazzino->dim; i++)
    {
        curr = old_table[i];
        while(curr != NULL)
        {
            next = curr->next;
            curr->next = NULL;
            index = hash(curr->name, new_size);
            if(magazzino->ingredients[index] == NULL){
                magazzino->ingredients[index] = curr;
            }else{
                el = magazzino->ingredients[index];
                while(el != NULL)
                {
                    el = el->next;
                }
                el = curr;
            }
            curr = next;
        }
    }
    free(old_table);
    magazzino->dim = new_size;
}

// inizializzazione dell'heap che conterrà i lotti
void InitializeHeap(Ingredient * ing)
{
    ing->stocks = calloc(STANDARD_HEAP_LENGHT, sizeof(stock *));
    ing->dim = STANDARD_HEAP_LENGHT;
    ing->count = 0;
    ing->total = 0;
    ing->height = 3;
}

void ResizeHeap(Ingredient * ing)
{
    int i, pow, new_dim;

    pow = 2;
    for(i = 0; i < ing->height; i++)
    {
        pow *= 2;
    }
    new_dim = ing->dim + pow;
    ing->stocks = realloc(ing->stocks, sizeof(stock *) * new_dim);
    for(i = ing->dim; i < new_dim; i++)
    {
        ing->stocks[i] = NULL;
    }
    ing->dim = new_dim;
}

void MinHeapify(Ingredient * ing, int index)
{
    int posmin;
    stock * tmp;

    if((2 * index + 1) < ing->dim && ing->stocks[2 * index + 1] != NULL && ing->stocks[2 * index + 1]->deadline < ing->stocks[index]->deadline){
        posmin = 2 * index + 1;
    }else{
        posmin = index;
    }

    if((2 * index + 2) < ing->dim && ing->stocks[2 * index + 2] != NULL && ing->stocks[2 * index + 2]->deadline < ing->stocks[posmin]->deadline)
    {
        posmin = 2 * index + 2;
    }

    if(posmin != index)
    {
        tmp = ing->stocks[index];
        ing->stocks[index] = ing->stocks[posmin];
        ing->stocks[posmin] = tmp;
        MinHeapify(ing, posmin); 
    }
}

// rimozione del lotto con la scadenza più prossima
void PopMin(Ingredient * ing)
{
    stock * min;

    if(ing->count < 1)
    {
        return;
    }

    min = ing->stocks[0];
    ing->stocks[0] = ing->stocks[ing->count - 1];
    ing->stocks[ing->count - 1] = min;
    ing->total -= min->weight;
    free(min);
    ing->count--;
    MinHeapify(ing, 0);
}

// controllo degli stock per elimininare quelli scaduti
void CheckExpired(Ingredient * ing, int t)
{
    while(ing->stocks[0]->deadline < t)
    {
        PopMin(ing);
    }
}

// creazione di un nuovo ingrediente
Ingredient * CreateIngredient(char * name)
{
    Ingredient * new;

    new = malloc(sizeof(Ingredient));

    new->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(new->name, name);
    new->next = NULL;
    InitializeHeap(new);

    return new;
}

// inserimento di un nuovo stock a seguito di un rifornimento
void InsertStock(char * name, int weight, int expire_date, Inventory * inv, int t)
{
    int index, i;
    Ingredient * curr;
    stock * new, * tmp;

    index = hash(name, inv->dim);

    curr = inv->ingredients[index];                                                         // passo 1: ricerca del bucket di destinazione
    while(curr != NULL && strcmp(curr->name, name) != 0)
    {
        curr = curr->next;
    }

    if(curr == NULL)                                                                        // se il bucket è vuoto creiamo l'ingrediente
    {
        inv->count++;
        if(inv->count * 100 / inv->dim > 70)
        {
            ResizeInventory(inv);                                                           // se la tabella è troppo piena effettuiamo una resize
        }
        curr = CreateIngredient(name);
    }

    CheckExpired(curr, t);                                                                  // passo 2: ricerca ed eliminazione degli stock scaduti

    new = malloc(sizeof(stock));                                                            // passo 3: inizializzazione del nuovo lotto;
    new->weight = weight;
    new->deadline = expire_date;

    curr->count++;                                                                          // se si eccede la dimensione dell'heap effettuiamo una resize
    if(curr->count > curr->dim)
    {
        curr->height++;
        ResizeHeap(curr);
    }
    curr->stocks[curr->count - 1] = new;
    curr->total += weight;
    i = curr->count - 1;                                                                    // passo 4: inserimento del nuovo lotto e ordinamento dell'array
    while(i > 0 && curr->stocks[(i - 1) / 2]->deadline > curr->stocks[i]->deadline)
    {
        tmp = curr->stocks[(i - 1) / 2];
        curr->stocks[(i - 1) / 2] = curr->stocks[i];
        curr->stocks[i] = tmp;
        i = (i - 1) / 2;
    }
}

