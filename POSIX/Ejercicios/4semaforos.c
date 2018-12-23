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
typedef sem_t Semaphore;

//Funcion para crear el semaforo
Semaphore *make_semaphore(int value){
    Semaphore *chicos= check_malloc(sizeof(Semaphore));
    int n = sem_init(chicos, 0, value);
    if(n!=0){
        perror_exit("init failed");
    }
    return chicos;
}

//Funcion para hacer al semaforo esperar de manera segura
void semaphore_wait(Semaphore *chicos){
    int n = sem_wait(chicos);
    if(n!=0){
        perror_exit("wait failed");
    }

}
//Funcion para liberar el semaforo de manera segura
void semaphore_signal(Semaphore *chicos){
    int n = sem_post(chicos);
    if(n!=0){
        perror_exit("signal failed");
    }

}


struct Maquina{
    int m;
    Semaphore *sem;
};


void *maquina(void *add_ptr){
    //creamos el puntero a los datos.
    struct Maquina *data;
    //inicializamos con lo que nos llega desde la funcion create.
    data= (struct Maquina*)add_ptr;

    //extraemos el semaforo de la estructura
    Semaphore *sem = data->sem;

    while(1){
        semaphore_wait(sem);
        if(data->m<=0){
            
            printf("HE RELLENADO LA MAQUINA %d\n",data->m);
            data->m=MAX;
        }
        semaphore_signal(sem);
        //preguntar si se podrian quitar signal y wait 
        semaphore_wait(sem);
        data->m=data->m -1;
        printf("COMPRADO\n");
        semaphore_signal(sem);

    }

}




int main(int argc, char const *argv[])
{
    Semaphore *mutex0=make_semaphore(1);
    struct Maquina m;
    m.sem=mutex0;
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
