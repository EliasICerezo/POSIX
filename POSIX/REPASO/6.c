#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
// defined
#define PRIORIDAD_A 29
#define PRIORIDAD_M 27
#define PRIORIDAD_B 25
#define PRIORIDAD_MAIN 30
#define HERENCIA_PRIORIDAD 0
#define TECHO_PRIORIDAD 1
#define NADA 2

void *tareaA(void *ptr);
void *tareaB(void *ptr);
void *tareaM(void *ptr);
void check(int n);
void esperaActiva(int ns);

struct Data{
    pthread_mutex_t *mutex;
    int cnt;

};

int main(int argc, char const *argv[])
{
    if(argc != 2){
        printf("Error en la ejecucion, el programa debe lanzarse asi: \n%s MODO_PRIORIDAD(0=Herencia prioridad, 1=Techo prioridad, 2=Ninguno de los 2)\n", argv[0]);
        exit(-1);
    }else{
        int modo = atoi(argv[1]);
        pthread_mutex_t mutex;
        pthread_mutexattr_t mutex_attr;
        
        switch(modo){
            case HERENCIA_PRIORIDAD:
                check(pthread_mutexattr_init(&mutex_attr));
                check(pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_INHERIT));
                check(pthread_mutex_init(&mutex, &mutex_attr));
                check(pthread_mutexattr_destroy(&mutex_attr));
                break;
            case TECHO_PRIORIDAD:
                check(pthread_mutexattr_init(&mutex_attr));
                check(pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_PROTECT));
                check(pthread_mutexattr_setprioceiling(&mutex_attr, PRIORIDAD_A));
                 check(pthread_mutex_init(&mutex, &mutex_attr));
                check(pthread_mutexattr_destroy(&mutex_attr));
                break;
            case NADA:
                check(pthread_mutex_init(&mutex,NULL));
                break;
        }
        struct Data data;
        data.cnt=0;
        data.mutex = &mutex;

        struct sched_param param;
        pthread_attr_t attr;
        int policy = SCHED_FIFO;
        pthread_t threadA,threadM,threadB;

        check(pthread_attr_init(&attr));
        check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));

        param.sched_priority=PRIORIDAD_MAIN;
        check(sched_setscheduler(getpid(), policy, &param));
        
        param.sched_priority=PRIORIDAD_A;
        check(pthread_attr_setschedpolicy(&attr, policy));
        check(pthread_attr_setschedparam(&attr,&param));
        check(pthread_create(&threadA, &attr, &tareaA, &data));

        param.sched_priority=PRIORIDAD_M;
        check(pthread_attr_setschedpolicy(&attr, policy));
        check(pthread_attr_setschedparam(&attr,&param));
        check(pthread_create(&threadM, &attr, &tareaM, &data));

        param.sched_priority=PRIORIDAD_B;
        check(pthread_attr_setschedpolicy(&attr, policy));
        check(pthread_attr_setschedparam(&attr,&param));
        check(pthread_create(&threadB, &attr, &tareaB, &data));

        check(pthread_join(threadA, NULL));
        check(pthread_join(threadB, NULL));
        check(pthread_join(threadM, NULL));
        check(pthread_attr_destroy(&attr));
        check(pthread_mutex_destroy(&mutex));



        return 0;
    }
}



void check(int n){
    if(n!=0){
        printf("Error: <%d>",n);
        exit(-1);
    }
}

void esperaActiva(int ns){
    time_t t;
    t=time(0)+ns;
    while(time(0)<t){}
}

void *tareaA(void *ptr){
    struct Data *data;
    data=(struct Data *)ptr;
    
    printf("Soy la tarea A y me duermo\n");
    sleep(3);
    printf("Soy la tarea A y me he despertado\n");
    check(pthread_mutex_lock(data->mutex));
    printf("Soy la tarea A y he bloqueado el mutex con exito\n");
    data->cnt++;
    check(pthread_mutex_unlock(data->mutex));
    printf("Soy la tarea A y he desbloqueado el mutex\nSoy la tarea A y he terminado\n");
}

void *tareaM (void *ptr){
    struct Data *data;
    data=(struct Data *)ptr;

    printf("Soy la tarea M y me duermo\n");
    sleep(5);
    printf("Soy la tarea M y me he despertado, empiezo espera activa\n");
    esperaActiva(15);
    printf("Soy la tarea M y he terminado\n");

}

void *tareaB(void *ptr){
    struct Data *data;
    data=(struct Data *)ptr;

    printf("Soy la tarea B y me duermo\n");
    sleep(1);
    printf("Soy la tarea B y me he despertado\n");
    check(pthread_mutex_lock(data->mutex));
    printf("Soy la tarea B y he bloqueado el mutex, ahora empiezo una espera activa\n");
    esperaActiva(7);
    printf("Soy la tarea B y he terminado mi espera activa\n");
    data->cnt++;
    check(pthread_mutex_unlock(data->mutex));
    printf("Soy la tarea B y he terminado\n");


}