#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

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
    Semaphore *sem= check_malloc(sizeof(Semaphore));
    int n = sem_init(sem, 0, value);
    if(n!=0){
        perror_exit("sem init failed :(");
    }
    return sem;
}

//Funcion para hacer al semaforo esperar de manera segura
void semaphore_wait(Semaphore *sem){
    int n = sem_wait(sem);
    if(n!=0){
        perror_exit("sem wait failed :(");
    }

}
//Funcion para liberar el semaforo de manera segura
void semaphore_signal(Semaphore *sem){
    int n = sem_post(sem);
    if(n!=0){
        perror_exit("sem post failed :(");
    }

}

// aqui vamos a tener el struct de los datos 
struct Data{
    Semaphore *mutex;
    int n;
};

//Funciones que realizan los incrementos
void *tareaA(void *ptr){
    struct Data *data;
    data=(struct Data*) ptr;
    int i;
    Semaphore *sem= data->mutex;
    for(i=0; i<40; i++){
        semaphore_wait(sem);
        data->n=data->n+100;
        semaphore_signal(sem);
        printf("Ejecutando tarea A, valor de la variable: %d\n", data->n);
        sleep(1);
    }
}

void *tareaB(void *ptr){
    struct Data *data;
    data=(struct Data*) ptr;
    int i;
    Semaphore *sem= data->mutex;
    for(i=0; i<40; i++){
        semaphore_wait(sem);
        data->n=data->n+1;
        semaphore_signal(sem);
        printf("Ejecutando tarea B, valor de la variable: %d\n", data->n);
        sleep(1);
    }
}






int main(int argc, char const *argv[])
{
    //Aqui creamos las hebras
    pthread_t hebra1;
    pthread_t hebra2;


    Semaphore * sem;
    sem=make_semaphore(1);
    int var=0;

    struct Data data;
    data.mutex=sem;
    data.n=var;


    //Inicializacion de cada una de las hebras.
    if(pthread_create(&hebra1, NULL, tareaA,  &data)) {
        fprintf(stderr, "Error creating thread 1\n");
        return 1;
    } 
    if(pthread_create(&hebra2, NULL, tareaB,  &data)) {
        fprintf(stderr, "Error creating thread 1\n");
        return 1;
    } 
    //Esperamos a que las distintas hebras terminen.
    if(pthread_join(hebra1, NULL)) {
        fprintf(stderr, "Error joining thread 1\n");
        return 2;
    }
    if(pthread_join(hebra2, NULL)) {
        fprintf(stderr, "Error joining thread 1\n");
        return 2;
    }


    return 0;
}


