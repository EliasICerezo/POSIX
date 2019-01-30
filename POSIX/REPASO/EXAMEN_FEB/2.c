#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/mman.h>
//defines
#define CONTROL1 SIGRTMIN
#define CONTROL2 SIGRTMIN+1
#define MONITOR SIGRTMAX
#define MAX 100 
#define MIN 0


void *tarea_control_1(void *ptr);
void *tarea_control_2(void *ptr);
void *tarea_monitor(void *ptr);
void check(int n);
void generaTimer(int s, int ms, int signum);

struct Data{
    pthread_mutex_t *mutex1;
    double presion1;
    double presion2;

};

int main(int argc, char const *argv[])
{
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, CONTROL1));
    check(sigaddset(&sigset, CONTROL2));
    check(sigaddset(&sigset, MONITOR));
    check(pthread_sigmask(SIG_BLOCK, &sigset, NULL));

    pthread_mutexattr_t mutex_attr;
    pthread_mutex_t mutex;
    //vamos a poner herencia de prioridad
    check(pthread_mutexattr_init(&mutex_attr));
    check(pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_INHERIT));
    check(pthread_mutex_init(&mutex, &mutex_attr));
    int policy = SCHED_RR;
    struct Data data;
    data.mutex1= &mutex;

    struct sched_param param;
    pthread_attr_t attr;
    pthread_t threadA, threadB, threadC;
    
    check(pthread_attr_init(&attr));
    check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));

    param.sched_priority=30;
    check(sched_setscheduler(getpid(), policy, &param));

    param.sched_priority=29;
    check(pthread_attr_setschedpolicy(&attr,policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&threadA, &attr, &tarea_control_1, &data));

    param.sched_priority=28;
    check(pthread_attr_setschedpolicy(&attr,policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&threadB, &attr, &tarea_control_2, &data));

    param.sched_priority=20;
    check(pthread_attr_setschedpolicy(&attr,policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&threadC, &attr, &tarea_monitor, &data));

    check(pthread_join(threadA, NULL));
    check(pthread_join(threadB, NULL));
    check(pthread_join(threadC, NULL));

    return 0;
}


void *tarea_control_1(void *ptr){
    struct Data *data;
    data= (struct Data*) ptr;
    sigset_t sigset;

    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, CONTROL1));
    int signum=0;
    double presion;

    generaTimer(2,0,CONTROL1);

    while(1){
        sigwait(&sigset, &signum);
        if(signum==CONTROL1){
            check(pthread_mutex_lock(data->mutex1));
            presion= MIN+(rand()%(MAX-MIN+1));
            data->presion1=presion;
            if(presion < 0.2*MAX){
                printf("Activacion alarma presion baja tanque 1\n");
            }else if(presion < 0.9*MAX){
                printf("Presion optima tanque 1\n");
            }else{
                printf("Alarma: Necesario apertura tanque 1\n");
            }
            check(pthread_mutex_unlock(data->mutex1));
        }
    }


}
void *tarea_control_2(void *ptr){
    struct Data *data;
    data= (struct Data*) ptr;
    sigset_t sigset;

    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, CONTROL2));
    int signum=0;
    double presion;

    generaTimer(3,0,CONTROL2);

    while(1){
        sigwait(&sigset, &signum);
        if(signum==CONTROL2){
            check(pthread_mutex_lock(data->mutex1));
            presion= MIN+(rand()%(MAX-MIN+1));
            data->presion2=presion;
            if(presion < 0.2*MAX){
                printf("Activacion alarma presion baja tanque 2\n");
            }else if(presion < 0.9*MAX){
                printf("Presion optima tanque 2\n");
            }else{
                printf("Alarma: Necesario apertura tanque 2\n");
            }
            check(pthread_mutex_unlock(data->mutex1));
        }
    }
}
void *tarea_monitor(void *ptr){
    struct Data *data;
    data= (struct Data*) ptr;
    sigset_t sigset;

    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, MONITOR));
    int signum=0;

    generaTimer(4,0,MONITOR);

    while(1){
        sigwait(&sigset, &signum);
        if(signum==MONITOR){
            check(pthread_mutex_lock(data->mutex1));
            printf("\n\nPRESIONES\nTanque 1: <%lf>\nTanque 2: <%lf>\n\n",data->presion1,data->presion2);
            check(pthread_mutex_unlock(data->mutex1));
        }
    }
}

void check(int n){
    if(n!=0){
        printf("Error: <%d>\n",n);
        exit(-1);
    }
}


void generaTimer(int s, int ms, int signum){
    timer_t timerid;
    struct sigevent sgev;
    struct timespec interval;
    struct timespec value;
    struct itimerspec its;

    sgev.sigev_notify = SIGEV_SIGNAL;
    sgev.sigev_signo = signum;
    sgev.sigev_value.sival_ptr = &timerid;
    check(timer_create(CLOCK_MONOTONIC, &sgev, &timerid));
    interval.tv_sec=s;
    interval.tv_nsec= (ms*1000000);
    value.tv_sec=0;
    value.tv_nsec=1;
    its.it_value=value;
    its.it_interval=interval;
    check(timer_settime(timerid, 0, &its, NULL));
}