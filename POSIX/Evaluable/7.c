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

void *tareaA(void *ptr);
void *tareaM(void *ptr);
void *tareaB(void *ptr);
void esperaActiva(int numsec);
void check(int n);


struct Data{
    pthread_mutex_t *mutex1;
    pthread_mutex_t *mutex2;
    pthread_mutex_t *mutex3;
    int cnt1;
    int cnt2;
};


int main(int argc, char const *argv[])
{
    if(argc != 2){
        printf("Error en la ejecucion, el programa debe lanzarse asi: \n%s MODO_PRIORIDAD(0=Herencia prioridad, 1=Techo prioridad, 2=Ninguno de los 2)\n", argv[0]);
        exit(-1);
    }else{
        pthread_mutex_t mutex1;
        pthread_mutex_t mutex2;
        pthread_mutex_t mutex3;
        pthread_mutexattr_t mutex_attr;
        int modo= atoi(argv[1]);
        switch(modo){
            case HERENCIA_PRIORIDAD:
                check(pthread_mutexattr_init(&mutex_attr));
                check(pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_INHERIT));
                check(pthread_mutex_init(&mutex1,&mutex_attr));
                check(pthread_mutex_init(&mutex2,&mutex_attr));
                check(pthread_mutex_init(&mutex3,&mutex_attr));
                check(pthread_mutexattr_destroy(&mutex_attr));
                break;
            case TECHO_PRIORIDAD:
                check(pthread_mutexattr_init(&mutex_attr));
                check(pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_PROTECT));
                check(pthread_mutexattr_setprioceiling(&mutex_attr,PRIORIDAD_A));
                check(pthread_mutex_init(&mutex1, &mutex_attr));
                check(pthread_mutexattr_setprioceiling(&mutex_attr, PRIORIDAD_M));
                check(pthread_mutex_init(&mutex2,&mutex_attr));
                check(pthread_mutex_init(&mutex3,&mutex_attr));
                check(pthread_mutexattr_destroy(&mutex_attr));
                break;
            case NADA:
                check(pthread_mutex_init(&mutex1, NULL));
                check(pthread_mutex_init(&mutex2, NULL));
                check(pthread_mutex_init(&mutex3, NULL));
                break;
        }

        //inicializamos el struct de los datos
        struct Data data;
        data.cnt1=0;
        data.cnt2=0;
        data.mutex1=&mutex1;
        data.mutex2=&mutex2;
        data.mutex3=&mutex3;
        //fin de la inicializacion del struct de los datos

        struct sched_param param;
        pthread_attr_t attr;
        int policy=SCHED_FIFO;
        pthread_t threadA, threadM, threadB;

        check(pthread_attr_init(&attr));
        check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));

        param.sched_priority=PRIORIDAD_MAIN;
        check(sched_setscheduler(getpid(), policy, &param));
        check(mlockall( MCL_CURRENT | MCL_FUTURE ));

        param.sched_priority=PRIORIDAD_A;
        check(pthread_attr_setschedpolicy(&attr, policy));
        check(pthread_attr_setschedparam(&attr, &param));
        check(pthread_create(&threadA, &attr, &tareaA, &data));


        param.sched_priority=PRIORIDAD_M;
        check(pthread_attr_setschedpolicy(&attr, policy));
        check(pthread_attr_setschedparam(&attr, &param));
        check(pthread_create(&threadM, &attr, &tareaM, &data));

        param.sched_priority=PRIORIDAD_B;
        check(pthread_attr_setschedpolicy(&attr, policy));
        check(pthread_attr_setschedparam(&attr,&param));
        check(pthread_create(&threadB, &attr, &tareaB, &data));

        check(pthread_join(threadA,NULL));
        check(pthread_join(threadM,NULL));
        check(pthread_join(threadB,NULL));
        
        check(pthread_mutex_destroy(&mutex1));
        check(pthread_mutex_destroy(&mutex2));
        check(pthread_mutex_destroy(&mutex3));
        
        return 0;
    }
}

void check(int n){
    if(n!=0){
        printf("Error %d\n", n);
        exit(-1);
    }
}

void *tareaA(void *ptr){
    struct Data *data;
    data= (struct Data *)ptr;
    printf("Soy la tarea alta y me duermo\n");
    sleep(3);
    printf("Soy la tarea alta y me he despertado\n");
    pthread_mutex_lock(data->mutex1);
    printf("Soy la tarea alta y he bloqueado con exito el mutex 1\n");
    data->cnt1++;
    pthread_mutex_unlock(data->mutex1);
    printf("Soy la tarea alta y he terminado\n");
    
}


void *tareaM(void *ptr){
    struct Data *data;
    data= (struct Data *)ptr;
    
    printf("Soy la tarea media y me duermo\n");
    sleep(2);
    printf("Soy la tarea media y me he despertado\n");
    pthread_mutex_lock(data->mutex2);
    printf("Soy la tarea media y he bloqueado con exito el mutex 2\n");
    pthread_mutex_lock(data->mutex3);
    printf("Soy la tarea media y he bloqueado con exito el mutex 3\n");
    data->cnt2++;
    pthread_mutex_unlock(data->mutex2);
    pthread_mutex_unlock(data->mutex3);
    printf("Soy la tarea media y he acabado\n");

}


void *tareaB(void *ptr){
    struct Data *data;
    data= (struct Data *)ptr;
    printf("Soy la tarea baja y me duermo\n");
    sleep(1);
    printf("Soy la tarea baja y me he despertado\n");
    pthread_mutex_lock(data->mutex3);
    printf("Soy la tarea baja y he bloqueado el mutex 3, ahora voy a hacer una espera activa\n");
    esperaActiva(3);
    printf("Soy la tarea baja y he terminado la espera activa\n");
    pthread_mutex_lock(data->mutex2);
    printf("Soy la tarea baja y he bloqueado con exito el mutex 2\n0");
    data->cnt2++;
    pthread_mutex_unlock(data->mutex2);
    pthread_mutex_unlock(data->mutex3);
    printf("Soy la tarea baja y he terminado\n");
}


void esperaActiva(int numsec){
    time_t t = time(0)+numsec;
    while(time(0)<t){}
}



