#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>


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

//El struct con el semaforo y la variable que le vamos a pasar al 
struct Data{
    int x;
    Semaphore *sem;
};

//funcion para cada una de las hebras a usar en el ejercicio
void *add_thread(void *add_ptr){
    //creamos el puntero a los datos.
    struct Data *data;
    //inicializamos con lo que nos llega desde la funcion create.
    data= (struct Data*)add_ptr;

    //extraemos el semaforo de la estructura
    Semaphore *sem = data->sem;

    int i;
    for(i=0;i<1000;i++){
        //cogemos el mutex 
        semaphore_wait(sem);
        //incrementamos la variable
        data->x=data->x+1;
        //soltamos el mutex
        semaphore_signal(sem);

    }

}


int main(int argc, char const *argv[])
{

    Semaphore *mutex = make_semaphore(1);


    //Declaramos el struct que contiene tanto la variable como el semaforo a usar.
    struct Data data;
    data.x=0;
    data.sem=mutex;

    pthread_t thread_1;
    pthread_t thread_2;

    //creamos 2 procesos que incrementen la misma variable y que tengan el mismo mutex
    if(pthread_create(&thread_1, NULL, add_thread,  &data)) {

        fprintf(stderr, "Error creating thread 1\n");
        return 1;

    } 

    if(pthread_create(&thread_2, NULL, add_thread,  &data)) {

        fprintf(stderr, "Error creating thread 2\n");
        return 1;

    } 


    //esperamos a que terminen con la instruccion join
    if(pthread_join(thread_1, NULL)) {

        fprintf(stderr, "Error joining thread 1\n");
        return 2;

    }

    if(pthread_join(thread_2, NULL)) {

        fprintf(stderr, "Error joining thread 2\n");
        return 2;

    }
    //finalmente imprimimos el valor por pantalla
    printf("El valor de la variable x es : %d\n",data.x);






    return 0;
}




