#include <pthread.h>
#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
//defines
#define UMBRALSUPTEMP 100
#define UMBRALINFTEMP 90
#define UMBRALSUPPRES 1000
#define UMBRALINFPRES 900


void *monitor(void *ptr);
void *control_temp(void *ptr);
void *control_presion(void *ptr);
void *sensor_temp(void *ptr);
void *sensor_presion(void *ptr);
void check(int n);
void dormir(int sec , int ms);

struct Data{
    int presion;
    int temperatura;
    int valvula;
    int inyeccion;
    pthread_mutex_t *mpresion;
    pthread_mutex_t *mtemperatura;
};

int main(int argc, char const *argv[])
{
    
    pthread_attr_t attr;
    struct sched_param param;
    pthread_t threadA, threadB, threadC, threadD, threadE;
    int policy = SCHED_RR;

    //inicializacion del struct data
    pthread_mutex_t m1,m2;
    check(pthread_mutex_init(&m1,NULL));
    check(pthread_mutex_init(&m2,NULL));
    struct Data data;
    data.mpresion=&m1;
    data.mtemperatura=&m2;
    data.inyeccion=0;
    data.valvula=0;
    data.temperatura=80;
    data.presion=800;
    //fin de la inicializacion del struct data


    check(pthread_attr_init(&attr));
    check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));

    param.sched_priority=30;
    check(sched_setscheduler(getpid(), policy, &param));

    param.sched_priority=28;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadA,&attr,&sensor_presion,&data));

    param.sched_priority=28;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr, &param));
    check(pthread_create(&threadB,&attr,&sensor_temp,&data));

    param.sched_priority=25;
    check(pthread_attr_setschedpolicy(&attr,policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&threadC,&attr,&control_presion,&data));
    
    param.sched_priority=25;
    check(pthread_attr_setschedpolicy(&attr,policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&threadD,&attr,&control_temp,&data));

    param.sched_priority=20;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&threadE, &attr, &monitor, &data));

    check(pthread_join(threadA,NULL));
    check(pthread_join(threadB,NULL));
    check(pthread_join(threadC,NULL));
    check(pthread_join(threadD,NULL));
    check(pthread_join(threadE,NULL));

    check(pthread_mutex_destroy(&m1));
    check(pthread_mutex_destroy(&m2));

    return 0;
}




void check(int n){
    if(n!=0){
        printf("Error <%d>\n",n);
        exit(-1);
    }
}

void dormir(int sec , int ms){
    struct timespec t;
    check(clock_gettime(CLOCK_MONOTONIC, &t));
    t.tv_sec+=sec;
    t.tv_nsec+=(ms*1000000);
    t.tv_sec = t.tv_sec + (t.tv_nsec / 1000000000);
    t.tv_nsec = t.tv_nsec % 1000000000;
    check(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL));
}

void *monitor(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;

    while(1){
        check(pthread_mutex_lock(data->mtemperatura));
        check(pthread_mutex_lock(data->mpresion));
        printf("\n\nEstado del sistema:\nTemperatura: <%d>\nPresion:<%d>\nInyeccion fria:<%d>\nValvula de presion: <%d>\n\n",data->temperatura, data->presion, data->inyeccion, data->valvula);
        check(pthread_mutex_unlock(data->mpresion));
        check(pthread_mutex_unlock(data->mtemperatura));
        dormir(1,0);
    }  


}

void *control_temp(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;

    while(1){
        check(pthread_mutex_lock(data->mtemperatura));
        if(data->temperatura> UMBRALSUPTEMP && data->inyeccion==0){
            printf("Activando la inyeccion de aire frio\n");
            data->inyeccion=1;
        }else if(data->temperatura < UMBRALINFTEMP && data->inyeccion==1){
            printf("Apagando la inyeccion de aire frio\n");
            data->inyeccion=0;
        }
        check(pthread_mutex_unlock(data->mtemperatura));
        dormir(0,500);
    }

}
void *sensor_temp(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;
    while(1){
        check(pthread_mutex_lock(data->mtemperatura));
        if(data->inyeccion==0){
            data->temperatura++;
        }else{
            data->temperatura-=2;
        }
        check(pthread_mutex_unlock(data->mtemperatura));
        dormir(0,400);
    }

}

void *control_presion(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;

    while(1){
        check(pthread_mutex_lock(data->mpresion));
        if(data->presion> UMBRALSUPPRES && data->valvula==0){
            printf("Abriendo la valula de presion\n");
            data->valvula=1;
        }else if(data->presion < UMBRALINFPRES && data->valvula==1){
            printf("Cerrando la valvula de presion\n");
            data->valvula=0;
        }
        check(pthread_mutex_unlock(data->mpresion));
        dormir(0,350);
    }
}

void *sensor_presion(void *ptr){
    struct Data *data;
    data= (struct Data *) ptr;
    while(1){
        check(pthread_mutex_lock(data->mpresion));
        if(data->valvula==0){
            data->presion+=10;
        }else{
            data->presion-=20;
        }
        check(pthread_mutex_unlock(data->mpresion));
        dormir(0,300);
    }

}