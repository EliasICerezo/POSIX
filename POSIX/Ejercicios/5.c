#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX 10
//--------INICIO FUNCIONES AUXILIARES
void perror_exit (char *s)
{
  perror (s);  exit (-1);
}

pthread_mutex_t* mutex_init(pthread_mutex_t *mutex){
    mutex= (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if(pthread_mutex_init(mutex,NULL)!=0){
        perror("MUTEX INIT FAILED\n");
    }
    return mutex;
}

//----------FIN FUNCIONES AUXILIARES

struct Viales{
    pthread_mutex_t *pegamentos;
    pthread_mutex_t *papeles;
    pthread_mutex_t *botellas;
};

pthread_mutex_t *controlador_sigue;

//Cada uno de los siguiente procesos va a estar esperando a que le llegue la señal de que puede continuar
void *pegamento(void *ptr){
    pthread_mutex_t *mutex;
    mutex=(pthread_mutex_t*) ptr;

    while(1){
        pthread_mutex_lock(mutex);
        printf("Soy pegamento y estoy embotellando\n");
        pthread_mutex_unlock(controlador_sigue);
    }
        

}


void *papel(void *ptr){
    pthread_mutex_t *mutex;
    mutex=(pthread_mutex_t*) ptr;

    while(1){
        pthread_mutex_lock(mutex);
        printf("Soy papel y estoy embotellando\n");
        pthread_mutex_unlock(controlador_sigue);
    }
        

}
void *botella(void *ptr){
    pthread_mutex_t *mutex;
    mutex=(pthread_mutex_t*) ptr;

    while(1){
        pthread_mutex_lock(mutex);
        printf("Soy botella y estoy embotellando\n");
        pthread_mutex_unlock(controlador_sigue);
    }
        

}

void *controlador(void *ptr){
    struct Viales *viales;
    viales= (struct Viales*) ptr;
    while(1){
        pthread_mutex_lock(controlador_sigue);
        switch(rand()%3){
            case 0:
            printf("Botella puede embotellar\n");
            sleep(1);
            pthread_mutex_unlock(viales->botellas);
            break;

            case 1:
            printf("Papel puede embotellar\n");
            sleep(1);
            pthread_mutex_unlock(viales->papeles);
            break;

            case 2:
            printf("pegamento puede embotellar\n");
            sleep(1);
            pthread_mutex_unlock(viales->pegamentos);
            break;


        }
    }



}


int main(int argc, char const *argv[])
{
    pthread_mutex_t *botellas;
    pthread_mutex_t *papeles;
    pthread_mutex_t *pegamentos;

    botellas=mutex_init(botellas);
    papeles=mutex_init(papeles);
    pegamentos=mutex_init(pegamentos);
    controlador_sigue=mutex_init(controlador_sigue);
    //tenemos que inicializarlos a 0 para que ya se queden esperando los distintos procesos
    pthread_mutex_lock(botellas);
    pthread_mutex_lock(papeles);
    pthread_mutex_lock(pegamentos);
    

    struct Viales viales;
    viales.botellas=botellas;
    viales.papeles=papeles;
    viales.pegamentos=pegamentos;


    pthread_t threads[4];

    pthread_create(&threads[0], NULL, controlador,  &viales);
    pthread_create(&threads[1], NULL, botella, viales.botellas);
    pthread_create(&threads[2], NULL, pegamento,  viales.pegamentos);
    pthread_create(&threads[3], NULL, papel,  viales.papeles);

    pthread_join(threads[0],NULL); 
    return 0;
}
