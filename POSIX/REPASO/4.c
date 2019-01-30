#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

//defines
#define ESPERA 1
#define PRIO_A 24
#define PRIO_B 24
#define DESPERTAR_A SIGRTMIN
#define DESPERTAR_B SIGRTMAX

void *tareaA(void *ptr);
void *tareaB(void *ptr);
void esperaActiva(int nsec);
void check(int n);
void dormir(int nsec, int ms, int signum);

struct Data{
    int n;
    pthread_mutex_t *mutex;
};

int main(int argc, char const *argv[])
{
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, DESPERTAR_A));
    check(sigaddset(&sigset, DESPERTAR_B));
    check(pthread_sigmask(SIG_BLOCK,&sigset,NULL));

    struct sched_param param;
    pthread_attr_t attr;
    pthread_mutex_t m1;
    int policy= SCHED_RR; 
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

void dormir(int nsec, int ms, int signum){
    timer_t timerid;
    struct itimerspec its;
    struct timespec interval;
    //Rellenamos sigevent y creamos el timer
    struct sigevent sgev;
    sgev.sigev_notify = SIGEV_SIGNAL;
    sgev.sigev_signo = signum;
    //sgev.sigev_notify_thread_id=pid;
    sgev.sigev_value.sival_ptr=&timerid;
    check(timer_create(CLOCK_MONOTONIC, &sgev, &timerid));
    //Ahora rellenamos la estructura itimerspec y el interval
    interval.tv_sec=nsec;
    interval.tv_nsec = (ms*1000000);
    its.it_interval=interval;
    its.it_value.tv_sec=0;
    its.it_value.tv_nsec=1;
    check(timer_settime(timerid, 0, &its, NULL));    
   
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
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset,DESPERTAR_A));
    int signum=0;
    dormir(90,200,DESPERTAR_A);
    while(1){
        sigwait(&sigset,&signum);
        if(signum==DESPERTAR_A){
            for(i=0; i<40;i++){
                pthread_mutex_lock(data->mutex);
                data->n+=100;
                printf("Soy la tarea A y el valor de la variable es: <%d>\n", data->n);
                pthread_mutex_unlock(data->mutex);
                esperaActiva(1);
            }
        }
    }

}

void *tareaB(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;
    int i;
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, DESPERTAR_B));
    int signum=0;
    dormir(100,300,DESPERTAR_B);
    while(1){
        sigwait(&sigset,&signum);
        if(signum==DESPERTAR_B){
            for(i=0; i<40;i++){
                pthread_mutex_lock(data->mutex);
                data->n++;
                printf("Soy la tarea B y el valor de la variable es: <%d>\n", data->n);
                pthread_mutex_unlock(data->mutex);
                esperaActiva(1);
            }
        }
    }  
}