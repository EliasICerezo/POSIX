/*


COMPILAR CON -LPTHREAD -LRT





*/



#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>

void check(int x);
void *tareaA(void *ptr);
void esperaActiva();
void * tareaB(void *ptr);


struct Data{
    pthread_mutex_t *sem;
    int n;
};

int main(int argc, char const *argv[])
{
    if(argc!=4){
        printf("Error. el comando correcto de ejecucion es:\n %s P1 P2 politica(0=fifo,1=rr)\n",argv[0]);
        exit(-1);
    }else{
        //SETUP DE LAS SEÑALES
        sigset_t sigset;
        check(sigemptyset(&sigset));
        check(sigaddset(&sigset, SIGRTMIN));
        check(sigaddset(&sigset, SIGRTMIN+1));
        pthread_sigmask(SIG_BLOCK, &sigset,NULL);
        //END SETUP

        int p1=atoi(argv[1]), p2=atoi(argv[2]);
        struct Data data;
        pthread_mutex_t mutex1;
        pthread_mutex_init(&mutex1, NULL);
        data.sem=&mutex1;
        data.n=0;
        int pol;

        
        struct sched_param param;
        int policy=atoi(argv[3])==0? SCHED_FIFO : SCHED_RR;
        pthread_t t1,t2;
        pthread_attr_t attr;
        check(pthread_attr_init(&attr));
        check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));
        //Prioridad del main
        param.sched_priority = p1>p2 ? p1+1 : p2+1;
        check(sched_setscheduler(getpid(), policy, &param));


        check(mlockall(MCL_CURRENT | MCL_FUTURE));

        param.sched_priority=p1;
        check(pthread_attr_setschedpolicy(&attr, policy));
        check(pthread_attr_setschedparam(&attr,&param));
        check(pthread_create(&t1, &attr, &tareaA, &data));

        param.sched_priority=p2;
        check(pthread_attr_setschedparam(&attr,&param));
        check(pthread_create(&t2, &attr, &tareaB, &data));

        check(pthread_join(t1, NULL));
        check(pthread_join(t2, NULL));

        pthread_mutex_destroy(data.sem);
        pthread_attr_destroy(&attr);
        
        return 0;
    }
}

void esperaActiva(){
  time_t t = time(0)+1;
  while(time(0)<t){}
}


void check(int x){
    if (x!=0){
        printf("Error: %d\n",x);
    }
}


void *tareaA(void *ptr){
    
    struct Data *data;
    data= (struct Data *) ptr;
    pthread_mutex_t *sem=data->sem;
    int i;
    //----------SETUP DE LOS TIMERS
    int signum; //numero de la señal que vamos a escuchar
    sigset_t set; //set que guarda la señal que aguardamos
    struct sigevent sig;
    timer_t timer;
    struct itimerspec required, old; //estructuras para iterar el tiempo
    struct timespec period; //periodo de la tarea
    sig.sigev_notify = SIGEV_SIGNAL; //modo para mandar una señal
    sig.sigev_signo = SIGRTMIN; //numero de la señal que se eleva cada vez que salta el timer
    check(clock_gettime(CLOCK_MONOTONIC, &required.it_value));
    period.tv_sec=90;
    period.tv_nsec=200*1000000;
    period.tv_sec += period.tv_nsec / 1000000000;
    period.tv_nsec = period.tv_nsec % 1000000000;
    required.it_interval=period;
    check(timer_create(CLOCK_MONOTONIC,&sig,&timer)); 
    check(sigemptyset(&set));
    sigaddset(&set,SIGRTMIN);
    check(timer_settime(timer,TIMER_ABSTIME, &required, &old));
    //--------------SETUP TERMINADO
    while(1){
        sigwait(&set, &signum);
        for(i=0; i<40; i++){
            pthread_mutex_lock(sem);
            data->n=data->n+100;
            printf("Ejecutando tarea A, valor de la variable: %d\n", data->n);
            pthread_mutex_unlock(sem);
            esperaActiva();
            
        }

    }


}

void *tareaB(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;
    pthread_mutex_t *sem=data->sem;
    int i;

    //----------SETUP DE LOS TIMERS
    int signum; //numero de la señal que vamos a escuchar
    sigset_t set; //set que guarda la señal que aguardamos
    struct sigevent sig;
    timer_t timer;
    struct itimerspec required, old; //estructuras para iterar el tiempo
    struct timespec period; //periodo de la tarea
    sig.sigev_notify = SIGEV_SIGNAL; //modo para mandar una señal
    sig.sigev_signo = SIGRTMIN+1; //numero de la señal que se eleva cada vez que salta el timer
    check(clock_gettime(CLOCK_MONOTONIC, &required.it_value));
    period.tv_sec=100;
    period.tv_nsec=300*1000000;
    required.it_interval=period;
    check(timer_create(CLOCK_MONOTONIC,&sig,&timer)); 
    check(sigemptyset(&set));
    sigaddset(&set,SIGRTMIN+1);
    check(timer_settime(timer,TIMER_ABSTIME, &required, &old));
    //--------------SETUP TERMINADO
    while(1){
        sigwait(&set, &signum);
        for(i=0; i<40; i++){
            pthread_mutex_lock(sem);
            data->n=data->n+1;
            printf("Ejecutando tarea B, valor de la variable: %d\n", data->n);
            pthread_mutex_unlock(sem);
            esperaActiva();
            
        }

    }
}




