#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>
//defines

void *tareaA(void *ptr);
void check(int n);


int main(int argc, char const *argv[])
{
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, SIGUSR1));
    check(sigaddset(&sigset, SIGUSR2));
    check(pthread_sigmask(SIG_BLOCK, &sigset, NULL));

    struct sched_param param;
    pthread_attr_t attr;
    pthread_t thread;

    check(pthread_attr_init(&attr));
    check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));
    check(mlockall(MCL_CURRENT | MCL_FUTURE));

    param.sched_priority=30;
    check(sched_setscheduler(getpid(), SCHED_FIFO, &param));

    param.sched_priority=25;
    check(pthread_attr_setschedpolicy(&attr, SCHED_FIFO));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&thread, &attr, &tareaA, NULL));
    
    check(pthread_join(thread, NULL));
    return 0;
}


void check(int n){
    if(n!=0){
        printf("Error <%d>",n);
        exit(-1);
    }
}

void *tareaA(void *ptr){
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, SIGUSR1));
    check(sigaddset(&sigset, SIGUSR2));
    int cont=0;
    int signum=0;
    int end=0;
    printf("Soy la tarea, estoy preparada y mi PID es : <%d>\n",getpid());
    while(end==0){
        sigwait(&sigset, &signum);
        if(signum==SIGUSR1){
            cont++;
            printf("He recibido la señar SIGUSR1 y he incrementado el contador, valor:<%d>\n",cont);
        }else if(signum==SIGUSR2){
            printf("He recibido SIGUSR2, voy a terminar\n");
            end=1;
        }
    }
    printf("Soy la tarea y he terminado\n");
}

