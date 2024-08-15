#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STANDARD_BUFFER_LENGHT 255
#define STANDARD_TABLE_LENGHT 10
#define STANDARD_ARRAY_LENGHT 7

// inizio dichiarazione delle strutture dati necessarie per il magazzino
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

// inizio dichiarazione delle strutture dati necessarie per il ricettario
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

//inizio dichiarazione delle strutture dati necessarie per la gestione degli ordini
typedef struct 
{
    Recipe * recipe;
    int qnt;
    int weight;
    int t_arrival;
}Order;

// variabili globali temporanee
RecipeBook * b;
Inventory * inv;
int t = 0;

// prototipi delle funzioni usate nella gestione del magazzino e degli stock
Inventory * InitializeInventory();
Ingredient * CreateIngredient(char *);
int hash(char *, int);
void ResizeInventory(Inventory *);
void InitializeHeap(Ingredient *);
void ResizeHeap(Ingredient *);
void MinHeapify(Ingredient *, int);
void PopMin(Ingredient *);
void RemoveExpired(Ingredient *, int);
void InsertStock(char *, int, int, Inventory *, int);

// prototipi delle funzioni usate nella gestione del ricettario e delle ricette
RecipeBook * InitBook();
Recipe * SearchRecipe(char *);
Recipe * AddRecipe(RecipeBook *, char *);
void ResizeBook(RecipeBook *);
void AddIngredientToRecipe(char *, int, Recipe *, Inventory *);
int RemoveRecipe(char *, RecipeBook *);

// prototipi funzioni usate nella gestione degli ordini
void ReceiveOrder(Recipe * r, int qnt);
int CheckIngredients(Order *);

// prototipi delle funzioni usate per il parsing dell'input
int GetCommand(char ** buffer, FILE * fp, int * buffer_size);
void ParseCommand(char * command);

// prototipi delle funzioni di utility usate per il debugging
void VisualizeInventory();
void VisualizeRecipe(Recipe *);
void VisualizeRecipeBook();

int main()
{
    inv = InitializeInventory();
    b = InitBook();
    char * buffer = NULL;
    int buffer_size = 0;
    int line_lenght;

    line_lenght = GetCommand(&buffer, stdin, &buffer_size);
    while(line_lenght != -1)
    {
        ParseCommand(buffer);
        VisualizeRecipeBook();
        line_lenght = GetCommand(&buffer, stdin, &buffer_size);
    }

    return 0;
}

// inizializazione dell'inventario
Inventory * InitializeInventory()
{
    Inventory * new;

    new = malloc(sizeof(Inventory));
    new->dim = STANDARD_TABLE_LENGHT;
    new->count = 0;
    new->ingredients = calloc(STANDARD_TABLE_LENGHT, sizeof(Ingredient *));

    return new;
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
    ing->stocks = calloc(STANDARD_ARRAY_LENGHT, sizeof(stock *));
    ing->dim = STANDARD_ARRAY_LENGHT;
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
    stock * last;

    if(ing->count < 1)
    {
        return;
    }

    min = ing->stocks[0];
    last = ing->stocks[ing->count - 1];
    ing->stocks[0] = last;
    ing->total -= min->weight;
    free(min);
    ing->count--;
    MinHeapify(ing, 0);
}

// controllo degli stock per elimininare quelli scaduti
void RemoveExpired(Ingredient * ing, int t)
{
    while(ing->stocks[0] != NULL && ing->stocks[0]->deadline < t)
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
        curr->next = inv->ingredients[index];
        inv->ingredients[index] = curr;
    }

    RemoveExpired(curr, t);                                                                  // passo 2: ricerca ed eliminazione degli stock scaduti

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

Recipe * AddRecipe(RecipeBook * b, char * name)
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
        return NULL;
    }

    b->count++;                                                     // step 3: se la tabella è troppo piena effettuiamo una resize
    if(b->count * 100 / b->dim > 70)
    {
        ResizeBook(b);
    }

    new = malloc(sizeof(Recipe));                                   // step 4: creazione della nuova ricetta
    new->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(new->name, name);
    new->required_ingredients = NULL;
    new->weight = 0;
    
    new->next = b->recipes[index];
    b->recipes[index] = new;

    return b->recipes[index];
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
        m->count++;
        if(m->count * 100 / m->dim >= 70)
        {
            ResizeInventory(m);
        }
    }

    new = malloc(sizeof(required_ingredient));
    new->ingredient = ing;
    new->quantity = qnt;
    new->next = r->required_ingredients;
    r->required_ingredients = new;
    r->weight += new->quantity;
}

// funzione per cercare una ricetta per nome
Recipe * SearchRecipe(char * name)
{
    int index;
    Recipe * el;

    index = hash(name, b->dim);
    el = b->recipes[index];
    while(el && strcmp(name, el->name) != 0)
    {
        el = el->next;
    }
    return el;
}

int RemoveRecipe(char * name, RecipeBook * b){ return 0; }


// funzione che dato un ordine verifica se sono disponibili le risorse per prepararlo subito
int CheckIngredients(Order * order)
{
    required_ingredient * el;
    Ingredient * curr;
    int required, result;

    result = 1;
    el = order->recipe->required_ingredients;
    while(el && result)
    {
        required = order->qnt * el->quantity;
        curr = el->ingredient;
        RemoveExpired(curr, t);
        if(curr->total < required)
        {
            result = 0;
        }
        el = el->next;
    }

    return result;
} 

// funzione che riceve i nuovi ordini e li indirizza nel "percorso" giusto
void ReceiveOrder(Recipe * r, int qnt)
{
    Order * new;

    new = malloc(sizeof(Order));
    new->recipe = r;
    new->t_arrival = t;
    new->qnt = qnt;

    if(CheckIngredients(new)){
        // prosegui alla preparazione dell'ordine e mettilo nella lista ready
    }else{
        // inserisci nella waiting list
    }
}


int GetCommand(char ** buffer, FILE * fp, int * buffer_size)
{
    int c;
    int line_lenght;

    if(*buffer == NULL)
    {
        *buffer = calloc(STANDARD_BUFFER_LENGHT, sizeof(char));
        *buffer_size = STANDARD_BUFFER_LENGHT;
    }

    line_lenght = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (line_lenght + 1 >= *buffer_size) {
            *buffer_size *= 2;
            *buffer = (char *)realloc(*buffer, *buffer_size);
        }
        if (c == '\n') {
            (*buffer)[line_lenght] = '\0';
            break;
        }
        (*buffer)[line_lenght++] = (char)c;
    }

    if(line_lenght == 0 || c == EOF)
        return -1;
    return line_lenght;
}

void ParseCommand(char * command)
{
    char * token, * name;
    char * DELIMITER = " ";
    int qnt, expire_t;

    token = strtok(command, DELIMITER);
    if(strcmp(token, "aggiungi_ricetta") == 0)                      // caso aggiungi ricetta al ricettario
    {
        name = strtok(NULL, DELIMITER);                             // il primo argomento sarà il nome della ricetta da inserire
        Recipe * new = AddRecipe(b, name);
        if(!new){
            printf("ignorato\n");
            return;
        }
        token = strtok(NULL, DELIMITER);                            // se l'inserimento va a buon fine procediamo a caricare gli ingredienti della ricetta
        while(token != NULL)
        {
            name = token;                                           // gli ingredienti arrivano in coppie <nome, quantità> quindi parsiamo gli argomenti a coppie finchè non finiscono
            token = strtok(NULL, DELIMITER);
            qnt = atoi(token);
            AddIngredientToRecipe(name, qnt, new, inv);
            token = strtok(NULL, DELIMITER);
        }
        printf("aggiunta\n");    
    }else if(strcmp(token, "rifornimento") == 0){                   // caso rifornimento
        token = strtok(NULL, DELIMITER);
        while (token != NULL)
        {
            name = token;                                           // in questo caso gli stock arrivano come triple <nome_ingrediente, quantità, scadenza>
            token = strtok(NULL, DELIMITER);
            qnt = atoi(token);
            token = strtok(NULL, DELIMITER);
            expire_t = atoi(token);
            InsertStock(name, qnt, expire_t, inv, t);
            token = strtok(NULL, DELIMITER);
        }
        printf("rifornito\n");
    }else if(strcmp(token, "rimuovi_ricetta") == 0){                // caso rimuovi ricetta da inventario, in questo caso l'unico argomento è il nome della ricetta
        name = strtok(NULL, DELIMITER);
        int res = RemoveRecipe(name, b);
        if(res == 0){
            printf("ordini in sospeso\n");
        }else if(res == -1){
            printf("non presente\n");
        }else{
            printf("rimossa\n");
        }
    }else if(strcmp(token, "ordine") == 0){
        name = strtok(NULL, DELIMITER);
        Recipe * r = SearchRecipe(name);
        if(r == NULL){
            printf("rifiutato\n");
        }else{
            token = strtok(NULL, DELIMITER);
            qnt = atoi(token);
            ReceiveOrder(r, qnt);
            printf("accettato\n");
        } 
    }else{
        token = strtok(NULL, DELIMITER);
        qnt = atoi(token);
        token = strtok(NULL, DELIMITER);
        expire_t = atoi(token);
        // funzione che setta i parametri del camioncino degli ordini
    }
}

// funzioni di utility per il debugging
void VisualizeInventory()
{
    Ingredient * el;

    for(int i = 0; i < inv->dim; i++)
    {   
        printf("%d.", i);
        el = inv->ingredients[i];
        while(el != NULL)
        {
            printf("%s --> ", el->name);
            el = el->next;
        }
        printf("\n");
    }
}

void VisualizeRecipeBook()
{
    Recipe * el;
    int i;

    printf("--------------------------------------------------------\n");
    for(i = 0; i < b->dim; i++)
    {
        printf("%d.", i);
        el = b->recipes[i];
        while(el != NULL)
        {
            printf("%s --> ", el->name);
            el = el->next;
        }
        printf("\n");
    }
    printf("--------------------------------------------------------\n");
    VisualizeInventory();
    for(i = 0; i < b->dim; i++)
    {
        el = b->recipes[i];
        while(el != NULL)
        {
            VisualizeRecipe(el);
            el = el->next;
        }
    }
}

void VisualizeRecipe(Recipe * r)
{
    required_ingredient * el;

    printf("--------------------------------------------------------\n");
    printf("%s %d\n", r->name, r->weight);
    printf("--------------------------------------------------------\n");
    el = r->required_ingredients;
    while(el != NULL)
    {
        printf("%s %d\n", el->ingredient->name, el->quantity);
        el = el->next;
    }
    printf("--------------------------------------------------------\n");
}
