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
typedef struct Ingredient_List_s
{
    Ingredient * ingredient;
    int quantity;
    struct Ingredient_List_s * next;
}Ingredient_List;

typedef struct recipe_s
{
    char * name;
    Ingredient_List * required_ingredients;
    int weight;
    struct recipe_s * next;
}Recipe;

typedef struct
{
    Recipe ** recipes;
    int dim;
    int count;
}RecipeBook;

// inizio dichiarazione delle strutture dati necessarie per la gestione degli ordini
typedef struct order_s
{
    Recipe * recipe;
    int qnt;
    int weight;
    int t_arrival;
    struct order_s * prev;
    struct order_s * next;
}Order;

typedef struct 
{
    Order * front;
    Order * tail;
}Deque;

typedef struct 
{
    Order ** orders;
    int count;
    int dim;
    int height;
}Heap;

// struttura dati che rappresenta il furgone del corriere
typedef struct 
{
    int max_load;
    int period;
}Carrier;

// variabili globali temporanee
RecipeBook * b;
Inventory * inv;
Deque * waiting_orders;
Heap * ready_orders;
Heap * loaded_orders;
Carrier carrier;
int t;

// prototipi delle funzioni usate nella gestione del magazzino e degli stock
Inventory * InitializeInventory();
Ingredient * CreateIngredient(char *);
int hash(char *, int);
void ResizeInventory(Inventory *);
void DestroyInventory();
void InitializeHeapStocks(Ingredient *);
void ResizeHeapStocks(Ingredient *);
void MinHeapifyStocks(Ingredient *, int);
void PopMinStocks(Ingredient *);
void RemoveExpired(Ingredient *);
void InsertStock(char *, int, int);

// prototipi delle funzioni usate nella gestione del ricettario e delle ricette
RecipeBook * InitBook();
Recipe * SearchRecipe(char *);
Recipe * AddRecipe(char *);
void ResizeBook(RecipeBook *);
void DestroyRecipeBook();
void AddIngredientToRecipe(char *, int, Recipe *);
int RemoveRecipe(char *);

// prototipi funzioni usate nella gestione degli ordini
Deque * InitDeque();
void DestroyDeque();
void Enqueue(Order *, Deque *);
void RemoveFromWaitingList(Order *, Deque *);
Heap * InitHeapOrders();
void DestroyHeapOrders(Heap *);
void ResizeHeapOrders(Heap *);
void MinHeapifyOrders(Heap * h, int index);
void InsertMinHeap(Order *, Heap *);
Order * PopMinOrder(Heap * h);
void MaxHeapifyOrders(Heap *, int);
void InsertMaxHeap(Order *, Heap *);
void PopMaxOrder(Heap *);
void ReceiveOrder(Recipe * r, int qnt);
int CheckIngredients(Order *);
void PrepareOrder(Order *, Heap * h);
void PrepareWaitingOrders();
void LoadCarrier();

// prototipi delle funzioni usate per il parsing dell'input
int GetCommand(char ** buffer, FILE * fp, int * buffer_size);
void ParseCommand(char * command);

// prototipi delle funzioni di utility usate per il debugging
void VisualizeInventory();
void VisualizeRecipe(Recipe *);
void VisualizeRecipeBook();
void VisualizeWaitingList();
void VisualizeReadyOrders();
void VisualizeLoadedOrders();
void VisualizeIngredient(Ingredient *);

int main()
{
    waiting_orders = InitDeque();
    ready_orders = InitHeapOrders();
    loaded_orders = InitHeapOrders();
    inv = InitializeInventory();
    b = InitBook();
    char * buffer = NULL;
    int buffer_size = 0;
    int line_lenght;

    line_lenght = GetCommand(&buffer, stdin, &buffer_size);        // il primo comando sono i parametri del carrier
    ParseCommand(buffer);

    t = 0;
    line_lenght = GetCommand(&buffer, stdin, &buffer_size);
    while(line_lenght != -1)
    {
        if((t % carrier.period) == 0 && t != 0)
        {
            LoadCarrier();                        // main da sistemare
        }
        ParseCommand(buffer);
        VisualizeWaitingList();
        VisualizeReadyOrders();
        VisualizeInventory();
        VisualizeRecipeBook();
        line_lenght = GetCommand(&buffer, stdin, &buffer_size);
        t++;
    }

    DestroyInventory();                                //fase di liberazione della memoria
    DestroyDeque();                                     
    DestroyHeapOrders(ready_orders);
    DestroyHeapOrders(loaded_orders);
    DestroyRecipeBook();

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

// funzione per liberare l'inventario
void DestroyInventory()
{
    Ingredient * next, * del;
    int i, j;

    for(i = 0; i < inv->dim; i++)
    {
        del = inv->ingredients[i];
        while(del)
        {
            next = del->next;
            for(j = 0; j < del->count; j++)
            {
                free(del->stocks[j]);
            }
            free(del->stocks);
            free(del->name);
            del = next;
        }
    }
    
    free(inv);
}

// inizializzazione dell'heap che conterrà i lotti
void InitializeHeapStocks(Ingredient * ing)
{
    ing->stocks = calloc(STANDARD_ARRAY_LENGHT, sizeof(stock *));
    ing->dim = STANDARD_ARRAY_LENGHT;
    ing->count = 0;
    ing->total = 0;
    ing->height = 3;
}

void ResizeHeapStocks(Ingredient * ing)
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

void MinHeapifyStocks(Ingredient * ing, int index)
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
        MinHeapifyStocks(ing, posmin); 
    }
}

// rimozione del lotto con la scadenza più prossima
void PopMinStocks(Ingredient * ing)
{
    stock * min;
    stock * last;

    if(ing->count < 1)
    {
        return;
    }

    min = ing->stocks[0];
    ing->total -= min->weight;
    if(ing->count == 1){
        free(min);
        ing->stocks[0] = NULL;
        ing->count = 0;
    }else{
        last = ing->stocks[ing->count - 1];
        ing->stocks[0] = last;
        free(min);
        ing->stocks[ing->count - 1] = NULL;
        ing->count--;
        MinHeapifyStocks(ing, 0);
    }
}

// controllo degli stock per elimininare quelli scaduti
void RemoveExpired(Ingredient * ing)
{
    while(ing->stocks[0] != NULL && ing->stocks[0]->deadline < t)
    {
        PopMinStocks(ing);
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
    InitializeHeapStocks(new);

    return new;
}

// inserimento di un nuovo stock a seguito di un rifornimento
void InsertStock(char * name, int weight, int expire_date)
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

    RemoveExpired(curr);                                                                  // passo 2: ricerca ed eliminazione degli stock scaduti

    new = malloc(sizeof(stock));                                                            // passo 3: inizializzazione del nuovo lotto;
    new->weight = weight;
    new->deadline = expire_date;

    curr->count++;                                                                          // se si eccede la dimensione dell'heap effettuiamo una resize
    if(curr->count > curr->dim)
    {
        curr->height++;
        ResizeHeapStocks(curr);
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

// funzione per liberare il ricettario
void DestroyRecipeBook()
{
    Recipe * del, * next;
    Ingredient_List * remove, * follow;
    int i;

    for(i = 0; i < b->dim; i++)
    {
        del = b->recipes[i];
        while(del)
        {
            next = del->next;
            remove = del->required_ingredients;
            while(remove)
            {
                follow = remove->next;
                free(remove);
                remove = follow;
            }
            free(del->name);
            free(del);
            del = next;
        }
    }
    free(b);
}

Recipe * AddRecipe(char * name)
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

void AddIngredientToRecipe(char * name, int qnt, Recipe * r)
{
    Ingredient * ing;
    Ingredient_List * new;
    int index;

    index = hash(name, inv->dim);
    ing = inv->ingredients[index];
    while(ing != NULL && strcmp(ing->name, name) != 0)
    {
        ing = ing->next;
    }

    if(ing == NULL)
    {
        ing = CreateIngredient(name);
        ing->next = inv->ingredients[index];
        inv->ingredients[index] = ing;
        inv->count++;
        if(inv->count * 100 / inv->dim >= 70)
        {
            ResizeInventory(inv);
        }
    }

    new = malloc(sizeof(Ingredient_List));
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

int RemoveRecipe(char * name)
{
    Recipe * del, * pre;
    Ingredient_List * remove, * follow;
    Order * el;
    int check, index;

    index = hash(name, b->dim);
    del = b->recipes[index];
    pre = NULL;
    while(del && strcmp(del->name, name) != 0)
    {
        pre = del;
        del = del->next;
    }

    if(del == NULL)
    {
        return -1;
    }
    check = 1;

    el = waiting_orders->front;
    while(el && check)
    {
        if(el->recipe == del)
        {
            check = 0;
        }
        el = el->next;
    }
    
    for(int i = 0; i < ready_orders->count && check; i++)
    {
        el = ready_orders->orders[i];
        if(el->recipe == del)
        {
            check = 0;
        }
    }

    if(check)
    {
        if(!pre){
            b->recipes[index] = del->next;
        }else{
            pre->next = del->next;
        }
        remove = del->required_ingredients;
        while(remove)
        {
            follow = remove->next;
            free(remove);
            remove = follow;
        }
        free(del->name);
        free(del);
    }

    return check;
}

// funzione per inizializzare gli heap degli ordini
Heap * InitHeapOrders()
{
    Heap * new;
    
    new = malloc(sizeof(Heap));
    new->orders = calloc(STANDARD_ARRAY_LENGHT, sizeof(Order *));
    new->dim = STANDARD_ARRAY_LENGHT;
    new->height = 3;
    new->count = 0;

    return new;
}

// funzione per la resize
void ResizeHeapOrders(Heap * h)
{
    int i, pow, new_dim;

    pow = 2;
    for(i = 0; i < h->height; i++)
    {
        pow *= 2;
    }
    new_dim = h->dim + pow;
    h->orders = realloc(h->orders, sizeof(Order *) * new_dim);
    for(i = h->dim; i < new_dim; i++)
    {
        h->orders[i] = NULL;
    }
    h->dim = new_dim;
}

void DestroyHeapOrders(Heap * h)
{
    int i;

    for(i = 0; i < h->count; i++)
    {
        free(h->orders[i]);
    }
    free(h);
}

// funzione per mantenere un miniheap
void MinHeapifyOrders(Heap * h, int index)
{
    int posmin;
    Order * tmp;

    if((2 * index + 1) < h->dim && h->orders[2 * index + 1] != NULL && h->orders[2 * index + 1]->t_arrival < h->orders[index]->t_arrival){
        posmin = 2 * index + 1;
    }else{
        posmin = index;
    }

    if((2 * index + 2) < h->dim && h->orders[2 * index + 2] != NULL && h->orders[2 * index + 2]->t_arrival < h->orders[posmin]->t_arrival)
    {
        posmin = 2 * index + 2;
    }

    if(posmin != index)
    {
        tmp = h->orders[index];
        h->orders[index] = h->orders[posmin];
        h->orders[posmin] = tmp;
        MinHeapifyOrders(h, posmin); 
    }
}

// funzione per inserire nel minheap
void InsertMinHeap(Order * new, Heap * h)
{   
    int i;
    Order * tmp;

    h->count++;                                                                          // se si eccede la dimensione dell'heap effettuiamo una resize
    if(h->count > h->dim)
    {
        h->height++;
        ResizeHeapOrders(h);
    }
    h->orders[h->count - 1] = new;
    i = h->count - 1;                                                                    // passo 4: inserimento del nuovo lotto e ordinamento dell'array
    while(i > 0 && h->orders[(i - 1) / 2]->t_arrival > h->orders[i]->t_arrival)
    {
        tmp = h->orders[(i - 1) / 2];
        h->orders[(i - 1) / 2] = h->orders[i];
        h->orders[i] = tmp;
        i = (i - 1) / 2;
    }
}

Order * PopMinOrder(Heap * h)
{
    Order * min;
    Order * last;

    if(h->count < 1)
    {
        return NULL;
    }

    min = h->orders[0];
    if(h->count == 1){
        h->count = 0;
        free(min);
        h->orders[0] = NULL;
    }else{
        last = h->orders[h->count - 1];
        h->orders[0] = last;
        h->orders[h->count - 1] = NULL;
        h->count--;
        MinHeapifyOrders(h, 0);
    }
    
    return min;
}

// funzione per tenere gli ordini in un maxheap ordinato secondo peso e istante di arrivo
void MaxHeapifyOrders(Heap * h, int index)
{
    int posmax, left, right;
    Order * tmp;

    left = 2 * index + 1;
    right = 2 * index + 2;
    if(left < h->count && (h->orders[left]->weight > h->orders[index]->weight || (h->orders[left]->weight == h->orders[index]->weight && h->orders[left]->t_arrival < h->orders[index]->t_arrival))){
        posmax = left;
    }else{
        posmax = index;
    }

    if(right < h->count && (h->orders[right]->weight > h->orders[posmax]->weight || (h->orders[right]->weight == h->orders[posmax]->weight && h->orders[right]->t_arrival < h->orders[posmax]->t_arrival)))
    {
        posmax = right;
    }

    if(posmax != index)
    {
        tmp = h->orders[index];
        h->orders[index] = h->orders[posmax];
        h->orders[posmax] = tmp;
        MaxHeapifyOrders(h, posmax); 
    }
}

void InsertMaxnHeap(Order * new, Heap * h)
{   
    int i, father;
    Order * tmp;

    h->count++;                                                                          // se si eccede la dimensione dell'heap effettuiamo una resize
    if(h->count > h->dim)
    {
        h->height++;
        ResizeHeapOrders(h);
    }
    h->orders[h->count - 1] = new;
    i = h->count - 1;
    father = (i - 1) / 2;                                                                    // passo 4: inserimento del nuovo lotto e ordinamento dell'array
    while(i > 0 && (h->orders[father]->weight < h->orders[i]->weight || (h->orders[father]->weight == h->orders[i]->weight && h->orders[father]->t_arrival > h->orders[i]->t_arrival)))
    {
        tmp = h->orders[father];
        h->orders[father] = h->orders[i];
        h->orders[i] = tmp;
        i = father;
        father = (i - 1) / 2;
    }
}

void PopMaxOrder(Heap * h)
{
    Order * max;
    Order * last;

    if(h->count < 1)
    {
        return;
    }

    max = h->orders[0];
    if(h->count == 1){
        h->count = 0;
        free(max);
        h->orders[0] = NULL;
    }else{
        last = h->orders[h->count - 1];
        h->orders[0] = last;
        h->orders[h->count - 1] = NULL;
        h->count--;
        MaxHeapifyOrders(h, 0);
    }
}

// funzione per inizializzare i deque
Deque * InitDeque()
{
    Deque * new;
    new = malloc(sizeof(Deque));
    new->front = NULL;
    new->tail  = new->front;

    return new;
}

void DestroyDeque()
{
    Order * del, * next;

    del = waiting_orders->front;
    while(del)
    {
        next = del->next;
        free(del);
        del = next;
    }
    free(waiting_orders);
}

void Enqueue(Order * n, Deque * d)
{
    if((d->tail == d->front) && d->front == NULL){
        d->tail = n;
        d->front = d->tail;
    }else{
        d->tail->next = n;
        n->prev = d->tail;
        d->tail = n;
    }
}

void RemoveFromWaitingList(Order * del, Deque * d)
{
    if(del == d->front)  
    {
        d->front = del->next;
        if (d->front != NULL){
            d->front->prev = NULL;
        }
        else{
            d->tail = NULL;
        }
    }else if(del == d->tail){
        d->tail = del->prev;
        if (d->tail != NULL){
            d->tail->next = NULL;
        }else{
            d->front = NULL;
        }
    }else{
        del->prev->next = del->next;
        del->next->prev = del->prev;
    }

    del->next = NULL;
    del->prev = NULL;
}

void PrepareOrder(Order * ord, Heap * h)
{
    Ingredient * curr;
    Ingredient_List * el;
    int required, tmp;

    el = ord->recipe->required_ingredients;
    while(el)
    {
        required = ord->qnt * el->quantity;
        curr = el->ingredient;
        while(required > 0)
        {
            tmp = required;
            required -= curr->stocks[0]->weight;
            if(required < 0)
            {
                curr->stocks[0]->weight = -1 * required; 
                curr->total -= tmp;
            }else{
                PopMinStocks(curr);
            }
        }
        el = el->next;
    }
    InsertMinHeap(ord, h);
}

// funzione che dato un ordine verifica se sono disponibili le risorse per prepararlo subito
int CheckIngredients(Order * order)
{
    Ingredient_List * el;
    Ingredient * curr;
    int required, result;

    result = 1;
    el = order->recipe->required_ingredients;
    while(el && result)
    {
        required = order->qnt * el->quantity;
        curr = el->ingredient;
        RemoveExpired(curr);
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
    new->weight = qnt * r->weight;
    new->next = NULL;
    new->prev = NULL;

    if(CheckIngredients(new)){
        PrepareOrder(new, ready_orders);
    }else{
        Enqueue(new, waiting_orders);
    }
}

void PrepareWaitingOrders()
{
    Order * el, * tmp;

    el = waiting_orders->front;
    while(el)
    {
        if(CheckIngredients(el))
        {
            tmp = el->next;
            RemoveFromWaitingList(el, waiting_orders);
            PrepareOrder(el, ready_orders);
            el = tmp;
        }else{
            el = el->next;
        }
    }
}

void LoadCarrier()
{
    int load;
    Order * curr;

    load = 0;
    while(ready_orders->count && load + ready_orders->orders[0]->weight <= carrier.max_load)
    {
        curr = PopMinOrder(ready_orders);
        load += curr->weight;
        InsertMaxnHeap(curr, loaded_orders);
    }

    VisualizeLoadedOrders();

    if(loaded_orders->count == 0)
    {
        printf("camioncino vuoto\n");
        return;
    }

    while(loaded_orders->count)
    {
        curr = loaded_orders->orders[0];
        printf("%d %s %d\n", curr->t_arrival, curr->recipe->name, curr->qnt);
        PopMaxOrder(loaded_orders);
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
        Recipe * new = AddRecipe(name);
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
            AddIngredientToRecipe(name, qnt, new);
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
            InsertStock(name, qnt, expire_t);
            token = strtok(NULL, DELIMITER);
        }
        printf("rifornito\n");
        PrepareWaitingOrders();
    }else if(strcmp(token, "rimuovi_ricetta") == 0){                // caso rimuovi ricetta da inventario, in questo caso l'unico argomento è il nome della ricetta
        name = strtok(NULL, DELIMITER);
        int res = RemoveRecipe(name);
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
        carrier.max_load = atoi(token);
        token = strtok(NULL, DELIMITER);
        carrier.period = atoi(token);
    }
}

// funzioni di utility per il debugging
void VisualizeInventory()
{
    Ingredient * el;
    int i;

    printf("--------------------------------------------------------\n");
    for(i = 0; i < inv->dim; i++)
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
    
    for(i = 0; i < inv->dim; i++)
    {
        el = inv->ingredients[i];
        while(el)
        {
            printf("--------------------------------------------------------\n");
            VisualizeIngredient(el);
            printf("--------------------------------------------------------\n");
            el = el->next;
        }
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
    Ingredient_List * el;

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

void VisualizeWaitingList()
{
    Order * el;

    printf("LISTA D'ATTESA: ");
    if(waiting_orders->front == NULL)
    {
        printf("lista vuota\n");
        return;
    }
    el = waiting_orders->front;
    while(el)
    {
        printf("(%s, %d, %d) ", el->recipe->name, el->qnt, el->t_arrival);
        el = el->next;
    }
    printf("\n");
}

void VisualizeReadyOrders()
{
    Order * el;
    int i;

    printf("ORDINI PRONTI: ");
    if(ready_orders->count == 0)
    {
        printf("lista vuota\n");
        return;
    }
    for(i = 0; i < ready_orders->count;i++)
    {
        el = ready_orders->orders[i];
        printf("(%s, %d, %d) ", el->recipe->name, el->qnt, el->t_arrival);
    }
    printf("\n");
}

void VisualizeLoadedOrders()
{
    Order * el;
    int i;

    printf("ORDINI CARICATI: ");
    if(loaded_orders->count == 0)
    {
        printf("lista vuota\n");
        return;
    }
    for(i = 0; i < loaded_orders->count;i++)
    {
        el = loaded_orders->orders[i];
        printf("(%s, %d, %d) ", el->recipe->name, el->qnt, el->t_arrival);
    }
    printf("\n");
}

void VisualizeIngredient(Ingredient * ing)
{
    stock * curr;
    int i;

    printf("%s(%d): ", ing->name, ing->total);
    for(i = 0; i < ing->count; i++)
    {
        curr = ing->stocks[i];
        printf("(%d, %d) ", curr->weight, curr->deadline);
    }
    printf("\n");
}
