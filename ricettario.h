#include "magazzino.h"

typedef struct 
{
    Ingredient * ingredient;
    int quantity;
}required_ingredient;

typedef struct recipe_s
{
    char * name;
    required_ingredient ** required_ingredients;
    int weight;
    struct recipe_s * next;
}Recipe;

typedef struct
{
    Recipe ** recipes;
    int dim;
    int count;
}RecipeBook;

RecipeBook * InitBook();
void ResizeBook(RecipeBook *);