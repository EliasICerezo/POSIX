#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

//Funciones derivadas del tutorial que estoy siguiendo
void check(int x){
    if (x!=0){
        printf("Error: %d\n",x);
    }
}



// aqui vamos a tener el struct de los datos 
struct Data{
    pthread_mutex_t *mutex;
    int n;
};




void *tareaA(void *ptr);
void *tareaB(void *ptr);
void esperaActiva();





int main(int argc, char const *argv[])
{

    if(argc!=4){
        printf("Error. el comando correcto de ejecucion es:\n %s P1 P2 politica(0=fifo,1=rr)\n",argv[0]);
        exit(-1);
    }else{
    int p1=atoi(argv[1]), p2=atoi(argv[2]);

    //Bloqueamos la memoria que vayamos a usar ahora y en el futuro.
    check(mlockall(MCL_CURRENT | MCL_FUTURE));
    //Aqui creamos las hebras
    //creamos el struct de datos compartidos y lo inicializamos
    struct Data data;
    pthread_mutex_t mutex1;
    pthread_mutex_init(&mutex1, NULL);
    data.mutex=&mutex1;
    data.n=0;
    int pol;

    
    struct sched_param param;
    int policy=atoi(argv[3])==0? SCHED_FIFO : SCHED_RR;
    pthread_t t1,t2;
    pthread_attr_t attr;
    
    
    //Obtenemos el parametro, le ponemos la prioridad para el main y volvemos a hacer un set. 
    
    
  

   
    check(pthread_attr_init(&attr));
    check(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));
    
    int my_pid = getpid();
    param.sched_priority = p1+p2;
    check(sched_setscheduler(my_pid, policy, &param));
    
    check(mlockall(MCL_CURRENT | MCL_FUTURE));

    param.sched_priority=p1;
    check(pthread_attr_setschedpolicy(&attr, policy));
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&t1, &attr, &tareaA, &data));

    //por ultimo añadomos la prioridad a la tarea 2:
    param.sched_priority=p2;
    check(pthread_attr_setschedparam(&attr,&param));
    check(pthread_create(&t2, &attr, &tareaB, &data));

    //destruimos el attr que hemos creado ya que no nos hace falta
    //pthread_attr_destroy(&attr);
    

    check(pthread_join(t1, NULL));
    check(pthread_join(t2, NULL));

    pthread_mutex_destroy(data.mutex);
    
    
    return 0;
    }
}

void esperaActiva(){
  time_t t = time(0)+1;
  while(time(0)<t){}
}

void *tareaB(void *ptr){
    struct Data *data;
    data=(struct Data*) ptr;
    int i;
    pthread_mutex_t *sem= data->mutex;
    //declaramos una estructura de tiempo para poder dormir al thread
    struct timespec t;

    while(1){ 
        for(i=0; i<40; i++){
            pthread_mutex_lock(sem);
            data->n=data->n+1;
            pthread_mutex_unlock(sem);
            printf("Ejecutando tarea B, valor de la variable: %d\n", data->n);
            esperaActiva();
        }
        //obtenemos el tiempo
        clock_gettime(CLOCK_MONOTONIC, &t);
        //le añadimos a la estructura lo necesario para completar el periodo
        t.tv_sec=t.tv_sec+60;
        t.tv_nsec=t.tv_nsec+300;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
    }   
}

//Funciones que realizan los incrementos
void *tareaA(void *ptr){
    struct Data *data;
    data=(struct Data*) ptr;
    int i;
    pthread_mutex_t *sem= data->mutex;
    
    struct timespec t;
    while(1){
        for(i=0; i<40; i++){
            pthread_mutex_lock(sem);
            data->n=data->n+100;
            printf("Ejecutando tarea A, valor de la variable: %d\n", data->n);
            pthread_mutex_unlock(sem);
            esperaActiva();
            
        }
        //obtenemos el tiempo
        clock_gettime(CLOCK_MONOTONIC, &t);
        //le añadimos a la estructura lo necesario para completar el periodo
        t.tv_sec=t.tv_sec+50;
        t.tv_nsec=t.tv_nsec+200;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
        }
}
