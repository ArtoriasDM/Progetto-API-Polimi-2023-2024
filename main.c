#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STANDARD_BUFFER_LENGHT 1024
#define STANDARD_TABLE_LENGHT 10
#define STANDARD_ARRAY_LENGHT 7

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

typedef struct 
{
    int max_load;
    int period;
}Carrier;

RecipeBook * b;
Inventory * inv;
Deque * waiting_orders;
Heap * ready_orders;
Heap * loaded_orders;
Carrier carrier;
int t;

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

RecipeBook * InitBook();
Recipe * SearchRecipe(char *);
Recipe * AddRecipe(char *);
void ResizeBook(RecipeBook *);
void DestroyRecipeBook();
void ClearIngredientList(Ingredient_List *);
void AddIngredientToRecipe(char *, int, Recipe *);
int RemoveRecipe(char *);

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

char * GetCommand(FILE * fp);
void ParseCommand(char * command);

int main()
{
    waiting_orders = InitDeque();
    ready_orders = InitHeapOrders();
    loaded_orders = InitHeapOrders();
    inv = InitializeInventory();
    b = InitBook();
    char * buffer;

    buffer = GetCommand(stdin);        
    ParseCommand(buffer);

    t = 0;
    buffer = GetCommand(stdin);
    while(buffer != NULL)
    {
        if((t % carrier.period) == 0 && t != 0)
        {
            LoadCarrier();                        
        }
        ParseCommand(buffer);
        free(buffer);
        buffer = GetCommand(stdin);
        t++;
    }

    if((t % carrier.period) == 0)
    {
        LoadCarrier();
    }

    free(buffer);
    DestroyInventory();                                
    DestroyDeque();                                     
    DestroyHeapOrders(ready_orders);
    DestroyHeapOrders(loaded_orders);
    DestroyRecipeBook();

    return 0;
}

Inventory * InitializeInventory()
{
    Inventory * new;

    new = malloc(sizeof(Inventory));
    new->dim = STANDARD_TABLE_LENGHT;
    new->count = 0;
    new->ingredients = calloc(STANDARD_TABLE_LENGHT, sizeof(Ingredient *));

    return new;
}

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
                while(el->next != NULL)
                {
                    el = el->next;
                }
                el->next = curr;
            }
            curr = next;
        }
    }
    free(old_table);
    magazzino->dim = new_size;
}

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
            free(del);
            del = next;
        }
    }
    free(inv->ingredients);
    free(inv);
}

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

void RemoveExpired(Ingredient * ing)
{
    while(ing->stocks[0] != NULL && ing->stocks[0]->deadline <= t)
    {
        PopMinStocks(ing);
    }
}

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

void InsertStock(char * name, int weight, int expire_date)
{
    int index, i;
    Ingredient * curr;
    stock * new, * tmp;

    index = hash(name, inv->dim);

    curr = inv->ingredients[index];                                                         
    while(curr != NULL && strcmp(curr->name, name) != 0)
    {
        curr = curr->next;
    }

    if(curr == NULL)                                                                        
    {
        inv->count++;
        if(inv->count * 100 / inv->dim > 70)
        {
            ResizeInventory(inv);                                                           
            index = hash(name, inv->dim);
        }
        curr = CreateIngredient(name);
        curr->next = inv->ingredients[index];
        inv->ingredients[index] = curr;
    }

    RemoveExpired(curr);                                                                  

    new = malloc(sizeof(stock));                                                           
    new->weight = weight;
    new->deadline = expire_date;

    curr->count++;                                                                       
    if(curr->count > curr->dim)
    {
        curr->height++;
        ResizeHeapStocks(curr);
    }
    curr->stocks[curr->count - 1] = new;
    curr->total += weight;
    i = curr->count - 1;                                                                    
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
    Recipe * curr, * next;

    old_table = b->recipes;
    new_size = b->dim * 2;
    b->recipes = calloc(new_size, sizeof(Recipe *));

    for(i = 0; i < b->dim; i++)
    {
        curr = old_table[i];
        while(curr != NULL)
        {
            next = curr->next;
            index = hash(curr->name, new_size);
            curr->next = b->recipes[index];
            b->recipes[index] = curr;
            curr = next;
        }
    }
    free(old_table);
    b->dim = new_size;
}

void DestroyRecipeBook()
{
    Recipe * del, * next;
    int i;

    for(i = 0; i < b->dim; i++)
    {
        del = b->recipes[i];
        while(del)
        {
            next = del->next;
            ClearIngredientList(del->required_ingredients);
            free(del->name);
            free(del);
            del = next;
        }
    }
    free(b->recipes);
    free(b);
}

void ClearIngredientList(Ingredient_List * head)
{
    Ingredient_List * del;

    while(head)
    {
        del = head;
        head = head->next;
        free(del);
    }
}

Recipe * AddRecipe(char * name)
{
    Recipe * new, * curr;
    int index;

    index = hash(name, b->dim);
    curr = b->recipes[index];
    while(curr != NULL && strcmp(name, curr->name) != 0)            
    {
        curr = curr->next;
    }

    if(curr != NULL)                                             
    {
        return NULL;
    }

    b->count++;                                                     
    if(b->count * 100 / b->dim > 70)
    {
        ResizeBook(b);
        index = hash(name, b->dim);                               
    }

    new = malloc(sizeof(Recipe));                                  
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
        ClearIngredientList(del->required_ingredients);
        free(del->name);
        free(del);
    }

    return check;
}

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
    free(h->orders);
    free(h);
}

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

void InsertMinHeap(Order * new, Heap * h)
{   
    int i;
    Order * tmp;

    h->count++;                                                                         
    if(h->count > h->dim)
    {
        h->height++;
        ResizeHeapOrders(h);
    }
    h->orders[h->count - 1] = new;
    i = h->count - 1;                                                                
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

void InsertMaxHeap(Order * new, Heap * h)
{   
    int i, father;
    Order * tmp;

    h->count++;                                                                          
    if(h->count > h->dim)
    {
        h->height++;
        ResizeHeapOrders(h);
    }
    h->orders[h->count - 1] = new;
    i = h->count - 1;
    father = (i - 1) / 2;                                                                    
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
        free(max);
        last = h->orders[h->count - 1];
        h->orders[0] = last;
        h->orders[h->count - 1] = NULL;
        h->count--;
        MaxHeapifyOrders(h, 0);
    }
}

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
        InsertMaxHeap(curr, loaded_orders);
    }

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

char * GetCommand(FILE * fp)
{
    int c, line_lenght, size;
    char * buffer;

    buffer = calloc(STANDARD_BUFFER_LENGHT, sizeof(char));
    size = STANDARD_BUFFER_LENGHT;

    line_lenght = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (line_lenght + 1 >= size) {
            size *= 2;
            buffer = (char *)realloc(buffer, size);
        }
        if (c == '\n') {
            buffer[line_lenght] = '\0';
            break;
        }
        buffer[line_lenght++] = (char)c;
    }

    if(line_lenght == 0 || c == EOF)
        return NULL;
    return buffer;
}

void ParseCommand(char * command)
{
    char * token, * name;
    char * DELIMITER = " ";
    int qnt, expire_t;

    token = strtok(command, DELIMITER);
    if(strcmp(token, "aggiungi_ricetta") == 0)                      
    {
        name = strtok(NULL, DELIMITER);                             
        Recipe * new = AddRecipe(name);
        if(!new){
            printf("ignorato\n");
            return;
        }
        token = strtok(NULL, DELIMITER);                           
        while(token != NULL)
        {
            name = token;                                           
            token = strtok(NULL, DELIMITER);
            qnt = atoi(token);
            AddIngredientToRecipe(name, qnt, new);
            token = strtok(NULL, DELIMITER);
        }
        printf("aggiunta\n");    
    }else if(strcmp(token, "rifornimento") == 0){                   
        token = strtok(NULL, DELIMITER);
        while (token != NULL)
        {
            name = token;                                           
            token = strtok(NULL, DELIMITER);
            qnt = atoi(token);
            token = strtok(NULL, DELIMITER);
            expire_t = atoi(token);
            InsertStock(name, qnt, expire_t);
            token = strtok(NULL, DELIMITER);
        }
        printf("rifornito\n");
        PrepareWaitingOrders();
    }else if(strcmp(token, "rimuovi_ricetta") == 0){                
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
        carrier.period = atoi(token);
        token = strtok(NULL, DELIMITER);
        carrier.max_load = atoi(token);
    }
}
