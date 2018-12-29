//compilar con -lpthread -lrt
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <signal.h>
//SEÑALES
#define FIN SIGRTMAX
#define LLAM SIGRTMIN
#define SMS SIGRTMIN+1
#define ENTRADAEXT SIGRTMIN+2
#define NIVELBAT SIGRTMIN+4
#define CONSUMOBAT SIGRTMIN+5


//FUNCIONES
void check(int x);
void *entradaExterior(void *ptr);
void *esperaAcciones(void *ptr);
void *nivelBateria(void *ptr);
void *consumoBateria(void *ptr);

//struct que lleva los datos compartidos
struct Data{
    int bateria;
    pthread_t thread_entradaExterior;
    pthread_t thread_esperaAcciones;
    pthread_t thread_nivelBateria;
    pthread_t thread_consumoBateria;
};


int main(int argc, char const *argv[])
{
    //SIGNAL SETUP
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, FIN);
    sigaddset(&sigset, LLAM);
    sigaddset(&sigset, SMS);
    sigaddset(&sigset, ENTRADAEXT);
    sigaddset(&sigset, NIVELBAT);
    sigaddset(&sigset, CONSUMOBAT);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);
    //END SIG SETUP

    pthread_t entrada, espera, nivelbat, consumobat;
    struct Data data;
    data.bateria=100;
    data.thread_consumoBateria=consumobat;
    data.thread_entradaExterior=entrada;
    data.thread_esperaAcciones=espera;
    //data.thread_nivelBateria=t[3];

    pthread_attr_t attr;
    struct sched_param param;
    check(pthread_attr_init(&attr));
    check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));
    //prioridad del main
    param.sched_priority=28;
    check(sched_setscheduler(getpid(), SCHED_RR, &param));
    check(pthread_attr_setschedpolicy(&attr, SCHED_RR));
    check(mlockall(MCL_CURRENT | MCL_FUTURE));
    
    //PRIORIDADES Y LANZAMIENTO DE LOS DISTINTOS THREADS
    param.sched_priority=27;
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&consumobat, &attr, &consumoBateria,&data));
    
    param.sched_priority=25;
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&entrada, &attr, entradaExterior,&data));

    param.sched_priority=26;
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&espera, &attr, esperaAcciones,&data));
    /*
    param.sched_priority=20;
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&t[3], &attr, &nivelBateria,&data));
    */
    //check(pthread_join(t[0],NULL));
    check(pthread_join(entrada,NULL));
    check(pthread_join(espera,NULL));
    //check(pthread_join(t[3],NULL));


    return 0;
}


void *entradaExterior(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;
    //----------SETUP DE LOS TIMERS
    int signum; //numero de la señal que vamos a escuchar
    sigset_t set; //set que guarda la señal que aguardamos
    struct sigevent sig;
    timer_t timer;
    struct itimerspec required, old; //estructuras para iterar el tiempo
    struct timespec period; //periodo de la tarea
    sig.sigev_notify = SIGEV_SIGNAL; //modo para mandar una señal
    sig.sigev_signo = ENTRADAEXT; //numero de la señal que se eleva cada vez que salta el timer
    check(clock_gettime(CLOCK_MONOTONIC, &required.it_value));
    period.tv_sec=1;
    period.tv_nsec=0;
    period.tv_sec += period.tv_nsec / 1000000000;
    period.tv_nsec = period.tv_nsec % 1000000000;
    required.it_interval=period;
    check(timer_create(CLOCK_MONOTONIC,&sig,&timer)); 
    check(sigemptyset(&set));
    sigaddset(&set,ENTRADAEXT);
    sigaddset(&set,FIN);
    check(timer_settime(timer,TIMER_ABSTIME, &required, &old));
    //TAREA
    int signo;
    while(1){
        sigwait(&set, &signum);
        if(signum==ENTRADAEXT){
            int accion=rand()%100;
            if(accion<10){
               printf("\n---------------------LLAMADA ENVIADA\n");
               kill(data->thread_esperaAcciones,LLAM);
            }else if(accion<20){
                printf("\n---------------------SMS ENVIADO\n");
                kill(data->thread_esperaAcciones,SMS);
               
            }
        }else if(signum==FIN){
            break;
        }
    }
    
}


void *esperaAcciones(void *ptr){
    //ESPERAR ACCIONES
    struct Data *data;
    data= (struct Data *) ptr;
    //SETUP
    sigset_t set;
    check(sigemptyset(&set));
    sigaddset(&set, LLAM);
    sigaddset(&set, SMS);
    sigaddset(&set, FIN);
    int sig;
    int signo;

    while(1){
        sigwait(&set, &sig);
        if(sig==LLAM){
            printf("EL TELEFONO HA RECIBIDO UNA LLAMADA\n");
        }else if(sig==SMS){
            printf("EL TELEFONO HA RECIBIDO UN MENSAJE\n");
        }else if(sig==FIN){
            printf("EL TELEFONO HA RECIBIDO FIN\n");
            break;
        }

    }
}

void *consumoBateria(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex,NULL);
    //----------SETUP DE LOS TIMERS
    int signum; //numero de la señal que vamos a escuchar
    sigset_t set; //set que guarda la señal que aguardamos
    struct sigevent sig;
    timer_t timer;
    struct itimerspec required, old; //estructuras para iterar el tiempo
    struct timespec period; //periodo de la tarea
    sig.sigev_notify = SIGEV_SIGNAL; //modo para mandar una señal
    sig.sigev_signo = CONSUMOBAT; //numero de la señal que se eleva cada vez que salta el timer
    check(clock_gettime(CLOCK_MONOTONIC, &required.it_value));
    period.tv_sec=1;
    period.tv_nsec=0;
    period.tv_sec += period.tv_nsec / 1000000000;
    period.tv_nsec = period.tv_nsec % 1000000000;
    required.it_interval=period;
    check(timer_create(CLOCK_MONOTONIC,&sig,&timer)); 
    check(sigemptyset(&set));
    sigaddset(&set,CONSUMOBAT);
    check(timer_settime(timer,TIMER_ABSTIME, &required, &old));
    //TAREA

    while(1){
        sigwait(&set, &signum);
        pthread_mutex_lock(&mutex);
        data->bateria=data->bateria-1;
        if(data->bateria==0){
            kill(data->thread_entradaExterior,FIN);
            kill(data->thread_esperaAcciones,FIN);
            kill(data->thread_nivelBateria,FIN);

            break;
        }
        pthread_mutex_unlock(&mutex);
        
    }
}

void *nivelBateria(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;
    //----------SETUP DE LOS TIMERS
    int signum; //numero de la señal que vamos a escuchar
    sigset_t set; //set que guarda la señal que aguardamos
    struct sigevent sig;
    timer_t timer;
    struct itimerspec required, old; //estructuras para iterar el tiempo
    struct timespec period; //periodo de la tarea
    sig.sigev_notify = SIGEV_SIGNAL; //modo para mandar una señal
    sig.sigev_signo = NIVELBAT; //numero de la señal que se eleva cada vez que salta el timer
    check(clock_gettime(CLOCK_MONOTONIC, &required.it_value));
    period.tv_sec=5;
    period.tv_nsec=0;
    period.tv_sec += period.tv_nsec / 1000000000;
    period.tv_nsec = period.tv_nsec % 1000000000;
    required.it_interval=period;
    check(timer_create(CLOCK_MONOTONIC,&sig,&timer)); 
    check(sigemptyset(&set));
    sigaddset(&set,NIVELBAT);
    sigaddset(&set, FIN);
    check(timer_settime(timer,TIMER_ABSTIME, &required, &old));
    int signo;
    while(1){
        sigwait(&set, &signum);
        if(signum==CONSUMOBAT){
            printf("\nBATERIA RESTANTE:%d  \n",data->bateria);
        }else if(signum==FIN){
            break;
        }
    }

}

void check(int x){
    if (x!=0){
        printf("Error: %d\n",x);
    }
}