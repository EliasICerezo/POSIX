#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#define MAX 10
//Funciones derivadas del tutorial que estoy siguiendo
void perror_exit (char *s)
{
  perror (s);  exit (-1);
}

void *check_malloc(int size)
{
  void *p = malloc (size);
  if (p == NULL) perror_exit ("malloc failed");
  return p;
}

//Para que quede mas claro que estoy usando uin semaforo



struct Maquina{
    int m;
    pthread_mutex_t *mutex;
};


void *maquina(void *add_ptr){
    //creamos el puntero a los datos.
    struct Maquina *data;
    //inicializamos con lo que nos llega desde la funcion create.
    data= (struct Maquina*)add_ptr;

    //extraemos el semaforo de la estructura
    pthread_mutex_t *mutex = data->mutex;

    while(1){
        pthread_mutex_lock(mutex);
        if(data->m<=0){
            
            printf("HE RELLENADO LA MAQUINA %d\n",data->m);
            data->m=MAX;
        }
        pthread_mutex_unlock(mutex);
        //preguntar si se podrian quitar signal y wait 
        pthread_mutex_lock(mutex);
        data->m=data->m -1;
        printf("COMPRADO\n");
        pthread_mutex_unlock(mutex);

    }

}


pthread_mutex_t* mutex_init(pthread_mutex_t *mutex){
    mutex= (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if(pthread_mutex_init(mutex,NULL)!=0){
        perror("MUTEX INIT FAILED\n");
    }
    return mutex;
}

int main(int argc, char const *argv[])
{
    pthread_mutex_t *mutex0;

    mutex0=mutex_init(mutex0);
    
    struct Maquina m;
    m.mutex=mutex0;
    m.m=MAX;

    pthread_t threads[50];
    int i;
    for(i=0;i<50;i++){
        if(pthread_create(&threads[i], NULL, maquina,  &m)) {

           fprintf(stderr, "Error creating thread %d\n",i);
            return 1;

        } 
    }


    pthread_join(threads[0],NULL);

    return 0;
}
