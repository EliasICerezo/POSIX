#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
//DEFINE
#define HERENCIA_PRIORIDAD 0
#define TECHO_PRIORIDAD 1
#define NADA 2
#define PRIORIDAD_MAIN 30
#define PRIORIDAD_A 29
#define PRIORIDAD_M 25
#define PRIORIDAD_B 20
//Funciones

void check(int x);
void esperaActiva();
void *tareaA(void *ptr);
void *tareaM(void *ptr);
void *tareaB(void *ptr);

struct Data{
    int cont;
    pthread_mutex_t *mutex;
};


int main(int argc, char const *argv[])
{
    //distintos modos de prioridad
    if(argc != 2){
        printf("Error en la ejecucion, el programa debe lanzarse asi: \n%s MODO_PRIORIDAD(0=Herencia prioridad, 1=Techo prioridad, 2=Ninguno de los 2)\n", argv[0]);
        exit(-1);
    }else{
        

        int modo= atoi(argv[1]);
        pthread_mutex_t mutex;
        pthread_mutexattr_t mutex_attr;
        switch(modo){
            case TECHO_PRIORIDAD:
                check(pthread_mutexattr_init(&mutex_attr));
                check(pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_PROTECT));
                check(pthread_mutexattr_setprioceiling(&mutex_attr, PRIORIDAD_A));
                check(pthread_mutex_init(&mutex, &mutex_attr));
                check(pthread_mutexattr_destroy(&mutex_attr));
                break;
            case HERENCIA_PRIORIDAD:
                check(pthread_mutexattr_init(&mutex_attr));
                check(pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_INHERIT));
                check(pthread_mutex_init(&mutex, &mutex_attr));
                check(pthread_mutexattr_destroy(&mutex_attr));
                break;
            case NADA:
                check(pthread_mutex_init(&mutex,NULL));
                break;
        }
        
        







        //Inicializamos el struct compartido por las tareas
        struct Data data;
        data.cont=0;
        data.mutex=&mutex;

        //pthread_mutexattr_t mutex_attr;
        struct sched_param param;
        pthread_attr_t attr;
        pthread_t threadA, threadB, threadM;
        int policy = SCHED_FIFO;
        //inicializo el atributo del thread
        check(pthread_attr_init(&attr));
        check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));


        

        param.sched_priority=PRIORIDAD_MAIN;
        check(sched_setscheduler(getpid(), policy, &param) );
        check(mlockall(MCL_CURRENT | MCL_FUTURE));


        //Tarea de prioridad alta
        param.sched_priority=PRIORIDAD_A;
        check(pthread_attr_setschedpolicy(&attr, policy));
        check(pthread_attr_setschedparam(&attr, &param));
        check(pthread_create(&threadA, &attr, &tareaA, &data));

        //Tarea de prioridad media
        param.sched_priority=PRIORIDAD_M;
        check(pthread_attr_setschedpolicy(&attr, policy));
        check(pthread_attr_setschedparam(&attr, &param));
        check(pthread_create(&threadM, &attr, &tareaM, &data));

        //Tarea de prioridad Baja
        param.sched_priority=PRIORIDAD_B;
        check(pthread_attr_setschedpolicy(&attr, policy));
        check(pthread_attr_setschedparam(&attr, &param));
        check(pthread_create(&threadB, &attr, &tareaB, &data));

        check(pthread_join(threadA, NULL));
        check(pthread_join(threadM, NULL));
        check(pthread_join(threadB, NULL));

        return 0;
    }
}



void check(int x){
    if (x!=0){
        printf("Error: %d\n",x);
    }
}

void esperaActiva(int n){
  time_t t = time(0)+n;
  while(time(0)<t){}
}

void *tareaA(void *ptr){
    struct Data *data;
    data=(struct Data *)ptr;
    printf("Soy la tarea de prioridad alta y me voy a dormir\n");
    sleep(3);
    printf("Soy la tarea de prioridad alta y me he despertado\n");
    pthread_mutex_lock(data->mutex);
    data->cont=data->cont+1;
    pthread_mutex_unlock(data->mutex);
    printf("Soy la tarea de prioridad alta y he terminado \n");

}

void *tareaM(void *ptr){
    struct Data *data;
    data=(struct Data *)ptr;
    printf("Soy la tarea de prioridad media y me voy a dormir\n");
    sleep(5);
    printf("Soy la tarea de prioridad media, me he despertado y ahora voy a hacer una espera activa\n");
    esperaActiva(15);
    printf("Soy la tarea de prioridad media y he terminado\n");


}

void *tareaB(void *ptr){
    struct Data *data;
    data=(struct Data *)ptr;
    printf("Soy la tarea de prioridad baja y me voy a dormir\n");
    sleep(1);
    printf("Soy la tarea de prioridad baja y me he despertado\n");
    pthread_mutex_lock(data->mutex);
    printf("Soy la tarea de prioridad baja y entro en espera activa\n");
    esperaActiva(7);
    data->cont=data->cont+1;
    pthread_mutex_unlock(data->mutex);
    printf("Soy la tarea de prioridad baja y he terminado\n");
}