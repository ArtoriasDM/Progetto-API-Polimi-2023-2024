#define STANDARD_INVENTORY_LENGHT 10
#define STANDARD_HEAP_LENGHT 7

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
}Ingredient;

typedef struct 
{
    Ingredient ** ingredients;
    int dim;
    int count;
}Inventory;

void InitializeInventory(Inventory *);
int hash(char *, int);
void ResizeInventory(Inventory *);
void InitializeHeap(Ingredient *);
void ResizeHeap(Ingredient *);
void MinHeapify(Ingredient *, int);
void PopMin(Ingredient *);
void CheckExpired(Ingredient *, int);
void InsertStock(char *, int, int, Inventory *, int);
Ingredient * CreateIngredient(char *);
