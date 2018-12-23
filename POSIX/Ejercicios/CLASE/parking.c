#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
//Señales que voy a usar
#define PET_ENTRAR SIGRTMIN
#define CON_ENTRAR SIGRTMIN+1
//Definimos el maximo numero de coches en el parking
#define MAX_COCHES 60

void check(int e){
    if(e!=0){
        perror("Se ha producido un error");
        //exit(-1);
    }
}

struct Data{
    pthread_t controlador;
    pthread_t coche;
    int numCoches;
    sem_t *entrada;
};



void *controlador(void *ptr);
void manejadorPeticiones(struct Data *data, int sig);

int main(int argc, char const *argv[])
{
    sigset_t sigset;
    pthread_t coche_thread;
    pthread_t controlador_thread;
    sem_t semaphore;
    check(sem_init(&semaphore,1,1));
    //inicializamos a vacio el conjunto de señales
    sigemptyset(&sigset);
    //Le añadimos las 2 señales que vamos a usar en este caso
    sigaddset(&sigset,PET_ENTRAR);
    sigaddset(&sigset, CON_ENTRAR);
    //Bloqueamos estas señales con sigmask
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    //creamos la struct que nos permite acceder a los distintos datos
    struct Data data;
    data.coche=coche;
    data.controlador=controlador;
    data.numCoches=0;
    data.entrada=&semaphore;


    //ahora aqui se crean los threads y se hace un join
    check(pthread_create(coche_thread, NULL, coche, &data));
    check(pthread_create(controlador_thread, NULL, controlador, &data));

    pthread_join(controlador_thread, NULL);

    
    

    return 0;
}

void *coche(void *ptr){
    struct Data *data;
    data=(struct Data*)ptr;
    int info=0;
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, CON_ENTRAR);
    sem_t *sem= data->entrada;
    int sig=0;
    while(1){
        pthread_mutex_lock(sem);
        kill(data->controlador, PET_ENTRAR);
        sig=sigwait(&sigset,&info);
        if(sig== CON_ENTRAR){
            printf("\n SOY EL COCHE Y HE APARCADO EN EL PARKING\n");
        }

        pthread_mutex_unlock(sem);

    }

}

void *controlador(void *ptr){
    struct Data *data;
    data=(struct Data*)ptr;
    int info=0;
    sigset_t sigset; 
    sigemptyset(&sigset);
    sigaddset(&sigset, PET_ENTRAR);
    int sig=0;

    while(1){
        sig=sigwait(&sigset, &info);
        if(sig!=-1){
            printf("-----RECIBIDA PETICION PARA ENTRAR AL PARKING-----\n");
            manejadorPeticiones(data, sig);
        }
    }

}

void manejadorPeticiones(struct Data *data, int sig){
    printf("\n SOY EL MANEJADOR Y HE RECIBIDO LA SEÑAL : %d\n", sig);
    pthread_mutex_t mutex;
    check(pthread_mutex_init(&mutex,NULL));
    pthread_mutex_lock(&mutex);
    if(sig == PET_ENTRAR){
        printf("He recibido una peticion para entrar\n");
        if(data->numCoches<MAX_COCHES){
            data->numCoches=data->numCoches+1;
            kill(data->coche,CON_ENTRAR);
        }else{
            printf("PARKING LLENO \n")
        }
        
    }
    pthread_mutex_unlock(&mutex);
}