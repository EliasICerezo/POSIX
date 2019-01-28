#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
//defines
#define LLAM SIGRTMIN
#define SMS SIGRTMIN+1
#define FIN SIGRTMAX
#define EVENT SIGRTMIN+2
#define BATERIA SIGRTMIN+4
#define MONITOR SIGRTMIN+5

void *eventos(void *ptr);
void *control_telefono(void *ptr);
void *consumobat(void *ptr);
void *muestrabat(void *ptr);
void check(int n);
void generaTimer(int sec, int ms, int signal);

struct Data{
    int bat;
    int telefono;
    pthread_t threadEvent;
    pthread_t threadControl;
    pthread_t threadConsumo;
    pthread_t threadMonitor;
};

int main(int argc, char const *argv[])
{
    
    return 0;
}


void check(int n){
    if(n!=0){
        printf("Error : <%d>",n);
        exit(-1);
    }
}

void generaTimer(int sec, int ms, int signal){
    struct timespec interval;
    struct itimerspec its;
    struct sigevent sgev;

}

void *eventos(void *ptr){
    struct Data *data;
    data=(struct Data *) ptr;

    int evento;
    int end=0;
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset,FIN));
    check(sigaddset(&sigset,EVENT));
    int signum=0;

    generaTimer(1,0,EVENT);

    while(end==0){
        sigwait(&sigset,&signum);
        if(signum==FIN){
            end=1;
        }else if(signum==EVENT){
            evento=rand()%100;
            if(evento < 10){
                //LLAMAR
                data->telefono=rand()%100;
                printf("Soy el proceso de eventos y he mandado una llamada\n");
                check(pthread_kill(data->threadControl, LLAM));
            }else if(evento <20){
                //MENSAJE
                data->telefono=rand()%100;
                printf("Soy el proceso de eventos y he mandado un mensaje\n");
                check(pthread_kill(data->threadControl, SMS));
            }
            
        }
    }
    printf("Soy la tarea generadora de eventos, he recibido la señal fin y he terminado\n");
}
void *control_telefono(void *ptr){
    struct Data *data;
    data=(struct Data *) ptr;

    int end=0;
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset,FIN));
    check(sigaddset(&sigset,LLAM));
    check(sigaddset(&sigset,SMS));
    int signum=0;

    while(end==0){
        sigwait(&sigset, &signum);

        if(signum==FIN){
            end=1;
        }else if(signum==LLAM){
            printf("Se ha recibido una llamada del numero de telefono:<%d>\n",data->telefono);
        }else if(signum==SMS){
            printf("Se ha recibido un mensaje del numero <%d>\n",data->telefono);
        }
    }
    printf("Soy el thread de control de telefono, he recibido la señal fin y he terminado\n");
}

void *consumobat(void *ptr){
    struct Data *data;
    data=(struct Data *) ptr;

    int end=0;
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset,BATERIA));
    int signum=0;

    generaTimer(1,0,BATERIA);
    while(end==0){
        sigwait(&sigset, &signum);
        if(signum==BATERIA){
            if(data->bat<=0){
                end=1;
                //Envio las señales fin a todo el mundo
                check(pthread_kill(data->threadControl,FIN));
                check(pthread_kill(data->threadEvent,FIN));
                check(pthread_kill(data->threadMonitor,FIN));
                printf("Soy el thread del consumo de bateria y he mandado las señales de fin al resto de procesos\n");
            }else{
                data->bat--;
            }
        }
    }
    printf("Soy el thread de consumo de bateria y he terminado\n");
}

void *muestrabat(void *ptr){
    struct Data *data;
    data=(struct Data *) ptr;

    int end=0;
    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset,FIN));
    check(sigaddset(&sigset,MONITOR));
    int signum=0;

    generaTimer(5,0,MONITOR);

    while(end==0){
        sigwait(&sigset, &signum);
        if(signum==FIN){
            end=1;
        }else if(signum==MONITOR){
            printf("\nNivel de bateria:<%d>\n",data->bat);
        }
    }
    printf("Soy el proceso que muestra la bateria y he terminado\n");
}