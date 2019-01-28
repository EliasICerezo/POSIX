#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
//defines
#define SIGNAL_A SIGRTMIN
#define SIGNAL_B SIGRTMIN+1
#define SIGNAL_C SIGRTMAX
#define FIFO SCHED_FIFO
#define RR SCHED_RR

void *tareaA(void *ptr);
void *tareaB(void *ptr);
void *tareaC(void *ptr);
void check(int n);



int main(int argc, char const *argv[])
{
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, SIGNAL_A));
    check(sigaddset(&sigset, SIGNAL_B));
    check(sigaddset(&sigset, SIGNAL_C));
    check(pthread_sigmask(SIG_BLOCK, &sigset, NULL));

    int policy = FIFO;
    struct sched_param param;
    pthread_attr_t attr;
    pthread_t threadA, threadB, threadC;
    

    check(pthread_attr_init(&attr));
    check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));
    check(mlockall(MCL_CURRENT | MCL_FUTURE));

    //INICIAMOS LOS THREADS
    param.sched_priority=30;
    check(sched_setscheduler(getpid(), policy, &param));

    param.sched_priority=27;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadA, &attr, &tareaA, NULL));

    param.sched_priority=28;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadB, &attr, &tareaB, NULL));

    param.sched_priority=29;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&threadC, &attr, &tareaC, NULL));

    printf("Threads creados, soy el main y me voy a dormir\n");
    sleep(2);
    printf("Soy el main y me he despertado\n");
    //sending signals
    pthread_kill(threadA,SIGNAL_A);
    printf("He signaleado al proceso A\n");
    pthread_kill(threadB,SIGNAL_B);
    printf("He signaleado al proceso B\n");
    pthread_kill(threadC,SIGNAL_C);
    printf("He signaleado al proceso C\n");

    //Esperamos a que los threads terminen
    check(pthread_join(threadA,NULL));
    check(pthread_join(threadB,NULL));
    check(pthread_join(threadC,NULL));

    return 0;
}

void check(int n){
    if(n!=0){
        printf("Error: <%d>\n",n);
        exit(-1);
    }
}

void *tareaA(void *ptr){
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset,SIGNAL_A));
    int end=0;
    int i=0;
    while(end==0){
        check(sigwait(&sigset,&i));
        if(i==SIGNAL_A){
            printf("Soy la tarea A y he recibido la señal para terminar\n");
            end=1;
        }
    }
    printf("Soy la tarea A y he terminado\n");
    
}

void *tareaB(void *ptr){
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset,SIGNAL_B));
    int end=0;
    int i=0;
    while(end==0){
        check(sigwait(&sigset,&i));
        if(i==SIGNAL_B){
            printf("Soy la tarea B y he recibido la señal para terminar\n");
            end=1;
        }
    }
    printf("Soy la tarea B y he terminado\n");
}

void *tareaC(void *ptr){
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset,SIGNAL_C));
    int end=0;
    int i=0;
    while(end==0){
        check(sigwait(&sigset,&i));
        if(i==SIGNAL_C){
            printf("Soy la tarea C y he recibido la señal para terminar\n");
            end=1;
        }
    }
    printf("Soy la tarea C y he terminado\n");
}