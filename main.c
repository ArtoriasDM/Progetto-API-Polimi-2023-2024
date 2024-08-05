#include <stdio.h>
#include <stdlib.h>
#include "magazzino.h"
#include <string.h>
#include <math.h>


int calculate_level(int);
void print_spaces(int);
void print_heap_as_tree(Ingredient *);
void VisualizeInventory(Inventory *);

int main()
{
    Inventory * magazzino;
    int t = 1;

    InitializeInventory(magazzino);
    InsertStock("pomodori", 50, 5, magazzino, t);
    InsertStock("pomodori", 50, 6, magazzino, t);
    InsertStock("panna", 10, 5, magazzino, t);
    InsertStock("banana", 100, 6, magazzino, t);
    InsertStock("uova", 20, 8, magazzino, t);
    VisualizeInventory(magazzino);
    t = 6;
    InsertStock("pomodori", 100, 10, magazzino, t);
    VisualizeInventory(magazzino);

    return 0;
}



// Funzione di utilità per calcolare il livello di un nodo
int calculate_level(int index) {
    return (int)log2(index + 1);
}

// Funzione di utilità per stampare spazi
void print_spaces(int count) {
    for (int i = 0; i < count; i++) {
        printf(" ");
    }
}

// Funzione per visualizzare l'heap come un albero
void print_heap_as_tree(Ingredient * ing) {
    int level = 0;
    int elements_at_current_level = 1;
    int index = 0;

    while (index < ing->count) {
        int spaces_between_nodes = (int)pow(2, ing->count - level);

        // Stampa gli elementi al livello corrente
        for (int i = 0; i < elements_at_current_level && index < ing->count; i++) {
            if (i == 0) {
                // Stampa spazi iniziali
                print_spaces(spaces_between_nodes / 2);
            } else {
                // Stampa spazi tra i nodi
                print_spaces(spaces_between_nodes);
            }
            printf("(%d, %d)", ing->stocks[index]->deadline, ing->stocks[index]->weight);
            index++;
        }
        printf("\n");

        // Passa al livello successivo
        level++;
        elements_at_current_level *= 2;
    }
}

// funzione per visualizzare l'inventario
void VisualizeInventory(Inventory * inv)
{
    int i;
    Ingredient * el;

    for(i = 0; i < inv->dim; i++)
    {
        printf("%d. ", i);
        el = inv->ingredients[i];
        if(el != NULL)
        {
            printf("(%s,%d)", el->name, el->total);
            el = el->next;
            while(el)
            {
                printf("-->");
                printf("(%s,%d)", el->name, el->total);
                el = el->next;
            }
        }
        printf("\n\n");
    }
    printf("LOAD FACTOR: %f.3\n\n", ((float)inv->count / inv->dim));

    for(i = 0; i < inv->dim; i++)
    {
        el = inv->ingredients[i];
        while(el)
        {
            printf("%s:\n\n", el->name);
            print_heap_as_tree(el);
            printf("\n");
            el = el->next;
        }
    }
}

