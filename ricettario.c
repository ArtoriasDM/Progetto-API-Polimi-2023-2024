#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <magazzino.h>
#include <ricettario.h>

RecipeBook * InitBook()
{
    RecipeBook * new;

    new = malloc(sizeof(RecipeBook));
    new->dim = STANDARD_TABLE_LENGHT;
    new->count = 0;

    new->recipes = calloc(new->dim, sizeof(Recipe *));

    return new;
}

void ResizeBook(RecipeBook * b)
{
    int i, index, new_size;
    Recipe ** old_table;
    Recipe * el, * curr, * next;

    old_table = b->recipes;
    new_size = b->dim * 2;
    b->recipes = calloc(new_size, sizeof(Recipe *));

    for(i = 0; i < b->dim; i++)
    {
        curr = old_table[i];
        while(curr != NULL)
        {
            next = curr->next;
            curr->next = NULL;
            index = hash(curr->name, new_size);
            if(b->recipes[index] == NULL){
                b->recipes[index] = curr;
            }else{
                el = b->recipes[index];
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
    b->dim = new_size;
}

int AddRecipe(RecipeBook * b, char * name)
{
    Recipe * new, * curr;
    int index, i;

    index = hash(name, b->dim);
    curr = b->recipes[index];
    while(curr != NULL && strcmp(name, curr->name) != 0)            // step 1: verifichiamo ceh nel ricettario non sia già presente la stessa ricetta
    {
        curr = curr->next;
    }

    if(curr != NULL)                                                // step 2: se la ricetta è già presente non procediamo
    {
        return 0;
    }

    b->count++;                                                     // step 3: se la tabella è troppo piena effettuiamo una resize
    if(b->count * 100 / b->dim > 70)
    {
        ResizeBook(b);
    }

    new = malloc(sizeof(Recipe));                                   // step 4: creazione della nuova ricerca
    new->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(new->name, name);
    new->required_ingredients = NULL;
    new->weight = 0;
    
    new->next = b->recipes[index];
    b->recipes[index] = new;

    return 1;
}

void AddIngredientToRecipe(char * name, int qnt, Recipe * r, Inventory * m)
{
    Ingredient * ing;
    required_ingredient * new;
    int index;

    index = hash(name, m->dim);
    ing = m->ingredients[index];
    while(ing != NULL && strcmp(ing->name, name) != 0)
    {
        ing = ing->next;
    }

    if(ing == NULL)
    {
        ing = CreateIngredient(name);
        ing->next = m->ingredients[index];
        m->ingredients[index] = ing;
    }

    new = malloc(sizeof(required_ingredient));
    new->ingredient = ing;
    new->quantity = qnt;
    new->next = r->required_ingredients;
    r->required_ingredients = new;
    r->weight += new->quantity;
}
