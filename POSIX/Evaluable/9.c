#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>
//defines
#define INC_A SIGRTMIN
#define INC_B SIGRTMIN+1
#define INC_C SIGRTMAX

void *tareaA(void *ptr);
void *tareaB(void *ptr);
void *tareaC(void *ptr);
void *tareaD(void *ptr);
void check(int n);

struct Data{
    int cva;
    int cvb;
    int cvc;
};

int main(int argc, char const *argv[])
{   
    //bloqueamos las señales que usamos, para que no haya problemas
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, INC_A));
    check(sigaddset(&sigset, INC_B));
    check(sigaddset(&sigset, INC_C));
    check(pthread_sigmask(SIG_BLOCK, &sigset, NULL));

    //inicializacion del struct
    
    struct Data data;
    data.cva=0;
    data.cvb=0;
    data.cvc=0;

    //fin de inicializacion del struct

    int policy = SCHED_FIFO;
    struct sched_param param;
    pthread_attr_t attr;
    pthread_t threadA, threadB, threadC, threadD;

    check(pthread_attr_init(&attr));
    check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));
    check(mlockall(MCL_CURRENT | MCL_FUTURE));

    //prioridad del main
    param.sched_priority=30;
    check(sched_setscheduler(getpid(), policy, &param));

    //prioridad de los procesos
    param.sched_priority=25;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadA, &attr, &tareaA, &data));

    param.sched_priority=25;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadB, &attr, &tareaB, &data));

    param.sched_priority=25;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadC, &attr, &tareaC, &data));

    param.sched_priority=25;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&threadC, &attr, &tareaC, &data));

    param.sched_priority=28;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadD, &attr, &tareaD, &data));
    
    printf("Se han creado las hebras, soy el main y me duermo\n");
    sleep(2);
    printf("Soy el main y me he despertado\n");
    //Enviamos las señales
    check(pthread_kill(threadD, INC_A));
    check(pthread_kill(threadD, INC_B));
    check(pthread_kill(threadD, INC_C));

    printf("Soy el proceso main y he enviado las señales al proceso D\n");

    check(pthread_join(threadA, NULL));
    printf("La tarea A ha terminado con exito\n");
    check(pthread_join(threadB, NULL));
    printf("La tarea B ha terminado con exito\n");
    check(pthread_join(threadC, NULL));
    printf("La tarea C ha terminado con exito\n");
    check(pthread_join(threadD, NULL));
    printf("La tarea D ha terminado con exito\n");
    return 0;
}

void *tareaA(void *ptr){
    struct Data *data;
    data = (struct Data *) ptr;
    int end=0;
    while(end==0){
        if(data->cva!=0){
            printf("Soy la tarea A y mi variable ha sido aumentada\n");
            data->cva--;
            end=1;
        }else{
            sleep(1);
        }
    }
    printf("Soy la tarea A y he terminado\n");
}

void *tareaB(void *ptr){
    struct Data *data;
    data = (struct Data *) ptr;
    int end=0;
    while(end==0){
        if(data->cvb!=0){
            printf("Soy la tarea B y mi variable ha sido aumentada\n");
            data->cvb--;
            end=1;
        }else{
            sleep(1);
        }
    }
    printf("Soy la tarea B y he terminado\n");
}

void *tareaC(void *ptr){
    struct Data *data;
    data = (struct Data *) ptr;
    int end=0;
    while(end==0){
        if(data->cvc!=0){
            printf("Soy la tarea C y mi variable ha sido aumentada\n");
            data->cvc--;
            end=1;
        }else{
            sleep(1);
        }
    }
    printf("Soy la tarea C y he terminado\n");
}

void *tareaD(void *ptr){
    struct Data *data;
    data=(struct Data *) ptr;
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, INC_A));
    check(sigaddset(&sigset, INC_B));
    check(sigaddset(&sigset, INC_C));
    int end=0;
    int num=0;
    while(end<3){
        check(sigwait(&sigset,&num));
        if(num==INC_A){
            printf("He recibido la señal para incrementar A y end vale <%d>\n", end);
            data->cva++;
            end++;
        }else if(num==INC_B){
            printf("He recibido la señal para incrementar B y end vale <%d>\n", end);
            data->cvb++;
            end++;
        }else if(num==INC_C){
            printf("He recibido la señal para incrementar C y end vale <%d>\n", end);
            data->cvc++;
            end++;
        }
    }
    printf("Soy la tarea D y he terminado\n");
}

void check(int n){
    if(n!=0){
        printf("Error <%d>\n", n);
        exit(-1);
    }
}

