#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
//constantes

#define CICLO_MONITOR 1
#define CICLO_CONTROL_TEMP 500
#define CICLO_SENSOR_TEMP 400
#define CICLO_CONTROL_PRESION 350
#define CICLO_SENSOR_PRESION 300

#define UMBRAL_SUP_TEMP 100
#define UMBRAL_INF_TEMP 80
#define UMBRAL_SUP_PRESION 1000
#define UMBRAL_INF_PRESION 800




//TAREAS
void *tarea_monitor(void *ptr);
void *tarea_control_presion(void *ptr);
void *tarea_control_temperatura(void *ptr);
void *tarea_sensor_temp(void* ptr);
void *tarea_sensor_presion(void* ptr);
//FUNCIONES ADICIONALES
void check(int x);
void esperar(int s, int ms);

struct Data{
    int presion;
    int temperatura;
    int inyeccionfria;
    int valvulapresion;
    pthread_mutex_t *presion_mutex;
    pthread_mutex_t *temp_mutex;

};

int main(int argc, char const *argv[])
{

    int policy = SCHED_FIFO;
    pthread_mutex_t m2,m3;
    pthread_mutex_init(&m2,NULL);
    pthread_mutex_init(&m3,NULL);
    //genero la estructura de datos para todo el programa

    struct Data data;
    data.inyeccionfria=0;
    data.presion=850;
    data.presion_mutex=&m2;
    data.temp_mutex=&m3;
    data.temperatura=80;
    data.valvulapresion=0;



    
    //Aqui vamos a definir un atributo para poner las distintas prioridades a los distintos procesos.
    struct sched_param param;
    pthread_t t[5];
    pthread_attr_t attr;
    
    check(pthread_attr_init(&attr));
    check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));

    param.sched_priority = 30;
    check(sched_setscheduler(getpid(), policy, &param));
    check(mlockall(MCL_CURRENT | MCL_FUTURE));

    //A partir de aqui vamos a inicializar los distintos threads
    //MONITOR
    param.sched_priority=20;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&t[0], &attr, &tarea_monitor, &data));

    //CONTROL DE TEMP
    param.sched_priority=25;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&t[1], &attr, &tarea_control_temperatura, &data));
    
    //CONTROL PRESION
    param.sched_priority=25;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&t[2], &attr, &tarea_control_presion, &data));
    
    //SENSOR TEMP
    param.sched_priority=28;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&t[3], &attr, &tarea_sensor_temp, &data));

    //SENSOR TEMP
    param.sched_priority=28;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&t[4], &attr, &tarea_sensor_presion, &data));

    int i;
    for(i=0; i<5; i++){
        check(pthread_join(t[i],NULL));
    }
    
    return 0;
}

void check(int x){
    if (x!=0){
        printf("Error: %d\n",x);
    }
}

void esperar(int s, int ms){
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    t.tv_sec=t.tv_sec+s;
    t.tv_nsec=t.tv_nsec+(ms*1000000);
    t.tv_sec += t.tv_nsec / 1000000000;
    t.tv_nsec = t.tv_nsec % 1000000000;
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
}

void *tarea_monitor(void *ptr){
    struct Data *data;
    data=(struct Data*) ptr;
    
    pthread_mutex_t *sem1=data->temp_mutex;
    pthread_mutex_t *sem2=data->presion_mutex;
    while(1){
        pthread_mutex_lock(sem1);
        pthread_mutex_lock(sem2);
        printf("\nEstado del sistema: \n");
        printf("Temperatura: %d\nPresion: %d\nInyeccion de aire frio: %d\nValvulaPresion: %d\n",data->temperatura,data->presion, data->inyeccionfria, data->valvulapresion);
        pthread_mutex_unlock(sem1);
        pthread_mutex_unlock(sem2);
        esperar(CICLO_MONITOR,0);

    }
}

void *tarea_control_temperatura(void *ptr){
    struct Data *data;
    data=(struct Data*) ptr;
    struct timespec t;
    pthread_mutex_t *mutex_temp=data->temp_mutex;

    while(1){

        //Ahora bloqueamos nuestro semafoto puesto que vamos a leer la temperatura y no queremos que el sensor escriba mientras leemos
        pthread_mutex_lock(mutex_temp);
        if(data->temperatura > 100 && data->inyeccionfria==0){
            printf("\n\n-------------ACTIVANDO INYECCION DE AIRE FRIO------------\n\n");
            data->inyeccionfria=1;
        }
        if(data->temperatura < 80 && data->inyeccionfria==1){
            printf("\n\n-------------DESACTIVANDO INYECCION DE AIRE FRIO------------\n\n");
            data->inyeccionfria=0;
        }
        pthread_mutex_unlock(mutex_temp);
        esperar(0,CICLO_CONTROL_TEMP);
    }

}  

void *tarea_sensor_temp(void* ptr){
    struct Data *data;
    data=(struct Data*) ptr;
    struct timespec t;
    pthread_mutex_t *mutex_temp=data->temp_mutex;

    while(1){


        pthread_mutex_lock(mutex_temp);
        if(data->inyeccionfria==0){
            data->temperatura=data->temperatura+1;
        }else{
            data->temperatura=data->temperatura-2;
        }
        pthread_mutex_unlock(mutex_temp);
        esperar(0,CICLO_SENSOR_TEMP);
        
    }


}

void *tarea_control_presion(void *ptr){
    struct Data *data;
    data=(struct Data*) ptr;
    struct timespec t;
    pthread_mutex_t *mutex_presion=data->presion_mutex;
    
    while(1){


        pthread_mutex_lock(mutex_presion);
        if(data->presion > UMBRAL_SUP_PRESION && data->valvulapresion==0){
            printf("\n\n-------------------ABRIENDO VALVULA DE PRESION---------------\n\n");
            data->valvulapresion=1;
        }
        if(data->presion < UMBRAL_INF_PRESION && data->valvulapresion==1){
            printf("\n\n-------------------CERRANDO VALVULA DE PRESION---------------\n\n");
            data->valvulapresion=0;
        }


        pthread_mutex_unlock(mutex_presion);
        esperar(0,CICLO_CONTROL_PRESION);

    }
}

void *tarea_sensor_presion(void* ptr){
    struct Data *data;
    data=(struct Data*) ptr;
    struct timespec t;

    pthread_mutex_t *mutex_presion=data->presion_mutex;

    while(1){


        pthread_mutex_lock(mutex_presion);
        if(data->inyeccionfria==0){
            data->presion=data->presion+10;
        }else{
            data->presion=data->presion-20;
        }
        pthread_mutex_unlock(mutex_presion);
        esperar(0,CICLO_SENSOR_PRESION);
        
    }


}


