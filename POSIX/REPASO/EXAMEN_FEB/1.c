#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/mman.h>
//defines
#define SIGNAL_A SIGRTMIN+1
#define SIGNAL_B SIGRTMIN+2
#define SIGNALATOB SIGRTMIN+4
#define SIGNALBTOA SIGRTMIN+3

void *tareaA(void *ptr);
void *tareaB(void *ptr);
void check(int n);
void generaTimer(int s, int ms, int signum);

struct Data{
    int AVISOS_A;
    int AVISOS_B;
    int estadoA;
    int estadoB;
    pthread_t threadA;
    pthread_t threadB;
};

int main(int argc, char const *argv[])
{
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, SIGNAL_A));
    check(sigaddset(&sigset, SIGNAL_B));
    check(sigaddset(&sigset, SIGNALATOB));
    check(sigaddset(&sigset, SIGNALBTOA));
    check(pthread_sigmask(SIG_BLOCK, &sigset,NULL));

    pthread_attr_t attr;
    struct sched_param param;
    pthread_t threadA, threadB;
    int policy = SCHED_RR;

    //inicializacion parcial del struct data
    struct Data data;
    data.AVISOS_A=0;
    data.AVISOS_B=0;
    data.estadoA=0;
    data.estadoB=1;
    //fin de la inicializacion parcial del struct 

    check(pthread_attr_init(&attr));
    check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));
    check(mlockall(MCL_CURRENT | MCL_FUTURE));

    param.sched_priority=30;
    check(sched_setscheduler(getpid(), policy, &param));
    
    param.sched_priority= 25;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadA, &attr, &tareaA, &data));
    data.threadA=threadA;

    param.sched_priority= 25;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadB, &attr, &tareaB, &data));
    data.threadB=threadB;
    
    check(pthread_attr_destroy(&attr));
    check(pthread_join(threadA,NULL));
    check(pthread_join(threadB,NULL));

    return 0;
}


void check(int n){
    if(n!=0){
        printf("Error: <%d>",n);
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

void *tareaA(void *ptr){
    struct Data *data;
    data= (struct Data *)ptr;
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, SIGNAL_A));
    check(sigaddset(&sigset, SIGNALBTOA));
    int end=0;
    int signum=0;
    double estadisticoA;
    generaTimer(1,0,SIGNAL_A);

    while(end==0){
        check(sigwait(&sigset,&signum));
        if(signum==SIGNALBTOA){
            data->estadoA=1;
        }else if(signum==SIGNAL_A){
            data->AVISOS_A++;
            printf("Valor de A: <%d>\n",data->AVISOS_A);
            if(data->estadoA==1){
                estadisticoA = 50 + 50 * sin(2*3.14159*0.01*data->AVISOS_A);
                printf("El valor del estadistico A es: <%lf>\n",estadisticoA);
                if(estadisticoA < 75.0){
                    data->estadoA=0;
                    pthread_kill(data->threadB, SIGNALATOB);
                }
            }
        }
    }

}

void *tareaB(void *ptr){
struct Data *data;
    data= (struct Data *)ptr;
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, SIGNAL_B));
    check(sigaddset(&sigset, SIGNALATOB));
    int end=0;
    int signum=0;
    double estadisticoB;
    generaTimer(2,0,SIGNAL_B);

    while(end==0){
        check(sigwait(&sigset,&signum));
        
        if(signum==SIGNALATOB){
            data->estadoB=1;
        }else if(signum==SIGNAL_B){
            data->AVISOS_B+=2;
            printf("Valor de B: <%d>\n",data->AVISOS_B);
            if(data->estadoB==1){
                estadisticoB = 50 + 50 * sin(2*3.14159*0.01*data->AVISOS_B);
                printf("El valor del estadistico B es: <%lf>\n",estadisticoB);
                if(estadisticoB >= 75.0){
                    data->estadoB=0;
                    pthread_kill(data->threadA, SIGNALBTOA);
                }
            }
        }
    }

}