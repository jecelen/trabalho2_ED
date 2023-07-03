
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SEED    0x12345678

typedef struct _reg{
    char codIBGE[10];
    char nome[100];
    int codUF;
    int capital;
    float lat;
    float longt;
    int codSiafi;
    int ddd;
    char fuso[33];
}municipio;

typedef struct {
     uintptr_t * table;
     int size;
     int max;
     uintptr_t deleted;
     char * (*get_key)(void *);
}thash;

char * get_key(void * reg){
    return (*((municipio *)reg)).codIBGE;
}


uint32_t hashf(const char* str, uint32_t h){
    /* One-byte-at-a-time Murmur hash
    Source: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp */
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}


int hash_insere(thash ** p, void * bucket){
    thash *h;
    h = *(p);
    uint32_t hash = hashf(h->get_key(bucket),SEED);
    int c1 = hash % (h->max);
    int c2 = hash % (h->max -1)+1;
    int pos = c1;
    int i = 1;


    if (h->max == (h->size+1)){
        free(bucket);
        return EXIT_FAILURE;
    }else{
        while(h->table[pos] != 0){
            if (h->table[pos] == h->deleted)
                break;
            pos = (c1+(i*c2)) % h->max;
            i++;
        }
        h->table[pos] = (uintptr_t) bucket;
        h->size +=1;
    }
    *(p) = h;
    return EXIT_SUCCESS;
}



int hash_constroi(thash * h,int nbuckets, char * (*get_key)(void *) ){
    h->table = (uint32_t*) malloc(sizeof(uintptr_t)*nbuckets +1);
    if (h->table == NULL){
        return EXIT_FAILURE;
    }
    h->max = nbuckets +1;
    h->size = 0;
    h->deleted = (uintptr_t) & (h->size);
    h->get_key = get_key;
    return EXIT_SUCCESS;
}


void * hash_busca(thash h, const char * key){
    int c1 = hashf(key,SEED) % (h.max);
    int c2 = hashf(key,SEED) % (h.max -1)+1;
    int pos = c1;
    int i = 2;
    while(h.table[pos] != 0){
        if (strcmp (h.get_key((void*)h.table[pos]),key) ==0)
            return (void *)h.table[pos];
        else{
            pos = (c1+c2)%(h.max);
            c2 = c2*i;
            i++;
        }
    }
    return NULL;
}

int hash_remove(thash * h, const char * key){
    int pos = hashf(key,SEED) % (h->max);
    while(h->table[pos]!=0){
        if (strcmp (h->get_key((void*)h->table[pos]),key) ==0){
            free((void *) h->table[pos]);
            h->table[pos] = h->deleted;
            h->size -=1;
            return EXIT_SUCCESS;
        }else{
            pos = (pos+1)%h->max;
        }

    }
    return EXIT_FAILURE;
}

void hash_apaga(thash *h){
    int pos;
    for(pos =0;pos< h->max;pos++){
        if (h->table[pos] != 0){
            if (h->table[pos]!=h->deleted){
                free((void *)h->table[pos]);
            }
        }
    }
    free(h->table);
}

void aloca_municipio(municipio** preg, thash* h){
    FILE *ptrarq;
    int result;
    char nome[100];


    ptrarq = fopen("municipios.csv", "r");

    if(ptrarq == NULL){
        printf("Problema ao abrir o arquivo municipios");
    }

    else{
        char indice[1000];
        char lixo;
        int ibge;
        int i = 0;
        *preg = (municipio *) malloc(sizeof(municipio) * 5571);  /*total de cidades*/
        result = fscanf(ptrarq, " %[^\n]", indice);

        while((result = fscanf(ptrarq, "%d", &ibge)) != EOF){

            municipio *cidade;
            cidade = &((*preg)[i]);
            snprintf(cidade->codIBGE, sizeof(cidade->codIBGE), "%d", ibge);

            result = fscanf(ptrarq, "%c", &lixo);
            result = fscanf(ptrarq, "%[^,]", nome);
            strcpy(cidade->nome, nome);

            result = fscanf(ptrarq, "%c", &lixo);
            result = fscanf(ptrarq, "%f", &cidade->lat);

            result = fscanf(ptrarq, "%c", &lixo);
            result = fscanf(ptrarq, "%f", &cidade->longt);

            result = fscanf(ptrarq, "%c", &lixo);
            result = fscanf(ptrarq, "%d", &cidade->capital);

            result = fscanf(ptrarq, "%c", &lixo);
            result = fscanf(ptrarq, "%d", &cidade->codUF);

            result = fscanf(ptrarq, "%c", &lixo);
            result = fscanf(ptrarq, "%d", &cidade->codSiafi);

            result = fscanf(ptrarq, "%c", &lixo);
            result = fscanf(ptrarq, "%d", &cidade->ddd);

            result = fscanf(ptrarq, "%c", &lixo);
            result = fscanf(ptrarq, " %[^\n]", cidade->fuso);


            hash_insere(&h, cidade);
            i++;
        }
    }

    fclose(ptrarq);
}



void encontrarCidades(thash h){

    municipio *registro;
    char ibge[10];

    printf("\nDigite o código do IBGE:\n");
    scanf("%s", ibge);


    registro = hash_busca(h, ibge);
    if(registro != NULL){
        printf("Código IBGE: %s\n", registro->codIBGE);
        printf("Nome: %s\n", registro->nome);
        printf("Latitude: %f Longitude: %f\n", registro->lat, registro->longt);
        printf("Código UF: %d\n", registro->codUF);
        printf("Código Siafi: %d\n", registro->codSiafi);
        printf("DDD: %d\n", registro->ddd);
        printf("Capital: %d\n", registro->capital);
        printf("Fuso Horário: %s\n\n", registro->fuso);
    }
    else
        printf("NULL\n");
}



int main(int argc, char* argv[]){
    thash h;
    municipio * cid;
    cid = NULL;
    int nbuckets = 15000;
    hash_constroi(&h,nbuckets,get_key);
    aloca_municipio(&cid, &h);
    int op;
    do{
        printf("1. Encontrar município.\n");
        printf("2. Encerrar.\n");
        scanf("%d", &op);
        if(op == 1){
            encontrarCidades(h);
            }
    }while(op!=2);

    return 0;
}
