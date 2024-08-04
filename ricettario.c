#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "magazzino.h"

#define STANDARD_DICT_DIM 10

typedef struct 
{
    char * name;
    int quantity;
}required_ingredient;

typedef struct recipe_s
{
    char * name;
    required_ingredient ** ingredients;
    int dim;
    int tot;
    struct recipe_s * next;
}Recipe;

typedef struct 
{
    Recipe ** recipes;
    int count;
    int dim;
}Dict;

void initDict(Dict * d)
{
    d = malloc(sizeof(Dict));
    d->count = 0;
    d->dim = STANDARD_DICT_DIM;
    d->recipes = calloc(d->dim, sizeof(Recipe *));
}

void ResizeDict(Dict * d)
{
    int i, index, new_size;
    Recipe ** old_table;
    Recipe * el, * curr, * next;

    old_table = d->recipes;
    new_size = d->dim * 2;
    d->recipes = calloc(new_size, sizeof(Ingredient *));

    for(i = 0; i < d->dim; i++)
    {
        curr = old_table[i];
        while(curr != NULL)
        {
            next = curr->next;
            curr->next = NULL;
            index = hash(curr->name, new_size);
            if(d->recipes[index] == NULL){
                d->recipes[index] = curr;
            }else{
                el = d->recipes[index];
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
    d->dim = new_size;
}

int AddRecipe(Dict * d, char * name)
{
    int index;
    Recipe * new;

    index = hash(name, d->dim);
    new = d->recipes[index];
    while(new != NULL && strcmp(new->name, name) != 0)
    {
        new = new->next;
    }

    if(new != NULL)
    {
        return 0;
    }

    d->count++;
    if(d->count * 100 / d->dim > 70)
    {
        ResizeDict(d);
    }
    new = malloc(sizeof(Recipe));
    new->next = NULL;
    new->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(new->name, name);


    // aggiunta degli ingredienti richiesti


    return 1;
}