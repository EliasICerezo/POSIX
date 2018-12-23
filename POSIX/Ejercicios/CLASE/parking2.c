#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
//Señales que voy a usar
#define PET_ENTRAR SIGRTMIN
#define CON_ENTRAR SIGRTMIN+1
#define MON_ENTRAR SIGRTMIN+2
#define CON_MONOVOLUMEN SIGRTMIN+3
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
    pthread_t monovolumen;
    int numCoches;
    sem_t entrada;
    sem_t entradaMonovolumen;
};



void *controlador(void *ptr);
void manejadorPeticiones(struct Data *data, int sig);
void *coche(void *ptr);
void *monovolumen(void *ptr);

int main(int argc, char const *argv[])
{
    sigset_t sigset;
    pthread_t coche_thread;
    pthread_t controlador_thread;
    pthread_t monovolumen_thread;
    sem_t semaphore;
    sem_t semaphore2;
    check(sem_init(&semaphore,1,1));
    check(sem_init(&semaphore2,1,1));
    //inicializamos a vacio el conjunto de señales
    sigemptyset(&sigset);
    //Le añadimos las 2 señales que vamos a usar en este caso
    sigaddset(&sigset, PET_ENTRAR);
    sigaddset(&sigset, CON_ENTRAR);
    sigaddset(&sigset, MON_ENTRAR);
    sigaddset(&sigset, CON_MONOVOLUMEN);
    //Bloqueamos estas señales con sigmask
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    //creamos la struct que nos permite acceder a los distintos datos
    struct Data data;
    data.coche=coche_thread;
    data.controlador=controlador_thread;
    data.monovolumen=monovolumen_thread;
    data.numCoches=0;
    data.entrada=semaphore;
    data.entradaMonovolumen=semaphore2;


    //ahora aqui se crean los threads y se hace un join
    check(pthread_create(&monovolumen_thread, NULL, monovolumen, &data));
   check(pthread_create(&coche_thread, NULL, coche, &data));
    check(pthread_create(&controlador_thread, NULL, controlador, &data));


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
    sem_t sem= data->entrada;
    int sig=0;
    while(1){
        sem_wait(&sem);
        kill(data->controlador, PET_ENTRAR);
        sig=sigwait(&sigset,&info);
        if(info== CON_ENTRAR){
            //printf("\n SOY EL COCHE Y HE APARCADO EN EL PARKING\n");
        }

        sem_post(&sem);

    }

}

void *monovolumen(void *ptr){
    struct Data *data;
    data=(struct Data*)ptr;
    int info=0;
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, CON_MONOVOLUMEN);
    sem_t sem= data->entradaMonovolumen;
    int sig=0;
    while(1){
        sem_wait(&sem);
        kill(data->controlador, MON_ENTRAR);
        sig=sigwait(&sigset,&info);
        if(info== CON_MONOVOLUMEN){
            //printf("\n SOY EL MONOVOLUMEN Y HE APARCADO EN EL PARKING\n");
        }

        sem_post(&sem);

    }

}

void *controlador(void *ptr){
    struct Data *data;
    data=(struct Data*)ptr;
    int info=0;
    sigset_t sigset; 
    sigemptyset(&sigset);
    sigaddset(&sigset, PET_ENTRAR);
    sigaddset(&sigset, MON_ENTRAR);
    int sig=0;

    while(1){
        sig=sigwait(&sigset, &info);
        if(sig!=-1){
            printf("\n\n-----RECIBIDA PETICION PARA ENTRAR AL PARKING-----\n");
            manejadorPeticiones(data, info);
        }
    }

}

void manejadorPeticiones(struct Data *data, int info){
    printf("\n SOY EL MANEJADOR Y HE RECIBIDO LA SEÑAL : %d\n", info);
    pthread_mutex_t mutex;
    check(pthread_mutex_init(&mutex,NULL));
    pthread_mutex_lock(&mutex);
    if(info == PET_ENTRAR){
        printf("He recibido una peticion para entrar\n");
        if(data->numCoches<MAX_COCHES){
            data->numCoches=data->numCoches+1;
            kill(data->coche,CON_ENTRAR);
        }
    }
    if(info == MON_ENTRAR){
        printf("He recibido una peticion de uin monovolumen para entrar\n");
        if(data->numCoches+2<=MAX_COCHES){
            data->numCoches=data->numCoches+2;
            kill(data->monovolumen, CON_MONOVOLUMEN);
        }
    }

    printf("OCUPACION: %d/%d",data->numCoches,MAX_COCHES);
   
    pthread_mutex_unlock(&mutex);
}

/*CUANDO SE REALIZA UNA TRAZA DE EJECUCION HAY 2 PETICIONES EXTRA DE CADA UNO DE LOS PROCESOS COCHE Y MONOVOLUMEN.
SIN EMBARGO, COMO EL PARKING ESTA LLENO, ESTAS NO SON RESPONDIDAS Y EL PROGRAMA SE QUEDA AHI
*/