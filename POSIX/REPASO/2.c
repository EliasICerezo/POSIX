#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>


//defines
#define ESPERA 1
#define PRIO_A 24
#define PRIO_B 26


void *tareaA(void *ptr);
void *tareaB(void *ptr);
void esperaActiva(int nsec);
void check(int n);
void dormir(int nsec, int nnsec);

struct Data{
    int n;
    pthread_mutex_t *mutex;
};

int main(int argc, char const *argv[])
{
    struct sched_param param;
    pthread_attr_t attr;
    pthread_mutex_t m1;
    int policy= SCHED_FIFO; 
    pthread_t threadA, threadB;

    //inicializamos el struct
    check(pthread_mutex_init(&m1,NULL));
    struct Data data;
    data.mutex = &m1;
    data.n=0;
    //fin de inicializacion del struct


    check(pthread_attr_init(&attr));
    check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));
    check(mlockall(MCL_CURRENT | MCL_FUTURE));

    param.sched_priority = 30;
    check(sched_setscheduler(getpid(), policy, &param));

    param.sched_priority = PRIO_A;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadA, &attr, &tareaA, &data));

    param.sched_priority=PRIO_B;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadB, &attr, &tareaB, &data));

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);

    return 0;
}

void dormir(int nsec, int ms){
    struct timespec t;
    check(clock_gettime(CLOCK_MONOTONIC, &t));
    t.tv_sec += nsec;
    t.tv_nsec += (ms*1000000);
    t.tv_sec = t.tv_sec + t.tv_nsec / 1000000000;
    t.tv_nsec = t.tv_nsec % 1000000000;
    check(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL));
}

void esperaActiva(int nsec){
    time_t t = time(0)+nsec;
    while(time(0)<t){}
}

void check(int n){
    if(n!=0){
        printf("Error: <%d>\n",n);
        exit(-1);
    }
}


void *tareaA(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;
    int i;
    while(1){
        for(i=0; i<40;i++){
            pthread_mutex_lock(data->mutex);
            data->n+=100;
            printf("Soy la tarea A y el valor de la variable es: <%d>\n", data->n);
            pthread_mutex_unlock(data->mutex);
            esperaActiva(1);
        }
        dormir(50,200);
    }

}

void *tareaB(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;
    int i;
    while(1){
        for(i=0; i<40;i++){
            pthread_mutex_lock(data->mutex);
            data->n++;
            printf("Soy la tarea B y el valor de la variable es: <%d>\n", data->n);
            pthread_mutex_unlock(data->mutex);
            esperaActiva(1);
        }
        dormir(60,300);
    }  
}