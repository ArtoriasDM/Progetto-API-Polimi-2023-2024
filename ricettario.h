#include "magazzino.h"

typedef struct required_ingredient_s
{
    Ingredient * ingredient;
    int quantity;
    struct required_ingredient_s * next;
}required_ingredient;

typedef struct recipe_s
{
    char * name;
    required_ingredient * required_ingredients;
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
int AddRecipe(RecipeBook *, char *);
void AddIngredientToRecipe(char *, int, Recipe *, Inventory *);