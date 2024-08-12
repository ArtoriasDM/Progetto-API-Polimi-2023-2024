#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ricettario.h>

#define STANDARD_BUFFER_LENGHT 256

RecipeBook b;
Recipe r;
Inventory inv;
int t;

int GetCommand(char ** buffer, FILE * fp, int * buffer_size)
{
    int c;
    int line_lenght;

    if(*(buffer) == NULL)
    {
        *(buffer) = calloc(STANDARD_BUFFER_LENGHT, sizeof(char));
        *buffer_size = STANDARD_BUFFER_LENGHT;
    }

    line_lenght = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (line_lenght + 1 >= *buffer_size) {
            *buffer_size *= 2;
            *buffer = (char *)realloc(*buffer, *buffer_size);
        }
        if (c == '\n') {
            *(buffer)[line_lenght++] = '\0';
            break;
        }
        (*buffer)[line_lenght++] = (char)c;
    }

    if(line_lenght == 0 || c == EOF)
        return -1;
    return line_lenght;
}

void ParseCommand(char * command, int lenght)
{
    char * token, * name;
    char * DELIMITER = " ";
    int qnt, expire_t;

    token = strtok(command, DELIMITER);
    if(strcmp(token, "aggiungi_ricetta") == 0)                      // caso aggiungi ricetta al ricettario
    {
        name = strtok(NULL, DELIMITER);                             // il primo argomento sarà il nome della ricetta da inserire
        Recipe * new = AddRecipe(&b, name);
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
            AddIngredientToRecipe(name, qnt, new, &inv);
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
            InsertStock(name, qnt, expire_t, &inv, t);
            token = strtok(NULL, DELIMITER);
        }
        printf("rifornito\n");
    }else if(strcmp(token, "rimuovi_ricetta") == 0){                // caso rimuovi ricetta da inventario, in questo caso l'unico argomento è il nome della ricetta
        name = strtok(NULL, DELIMITER);
        int res = RemoveRecipe(name, &b);
        if(res == 0){
            printf("ordini in sospeso\n");
        }else if(res == -1){
            printf("non presente\n");
        }else{
            printf("rimossa\n");
        }
    }
}
