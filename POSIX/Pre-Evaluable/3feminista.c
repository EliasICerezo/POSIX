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
    Semaphore *chicos= check_malloc(sizeof(Semaphore));
    int n = sem_init(chicos, 0, value);
    if(n!=0){
        perror_exit("chicos init failed :(");
    }
    return chicos;
}

//Funcion para hacer al semaforo esperar de manera segura
void semaphore_wait(Semaphore *chicos){
    int n = sem_wait(chicos);
    if(n!=0){
        perror_exit("chicos wait failed :(");
    }

}
//Funcion para liberar el semaforo de manera segura
void semaphore_signal(Semaphore *chicos){
    int n = sem_post(chicos);
    if(n!=0){
        perror_exit("chicos post failed :(");
    }

}


//Struct compartido por todos los threads 
struct Data{
    Semaphore *chicos;
    Semaphore *chicas;
    int nchicos;

};

void *chicos(void *add_ptr){
    //creamos el puntero a los datos.
    struct Data *data;
    //inicializamos con lo que nos llega desde la funcion create.
    data= (struct Data*)add_ptr;

    //extraemos el semaforo de la estructura
    Semaphore *chicos = data->chicos;
    Semaphore *chicas = data->chicas;
    
    while(1){
        //cojo y suelto el semaforo de las chicas para que en caso de que una chica quiera entrar me quede bloqueado
        semaphore_wait(chicas);
        semaphore_signal(chicas);


        //pido permiso para entrar
        semaphore_wait(chicos);
        printf("Soy un chico y he entrado en el baño\n");
        semaphore_signal(chicos);
    }


}

void *chicas(void *add_ptr){
    //creamos el puntero a los datos.
    struct Data *data;
    //inicializamos con lo que nos llega desde la funcion create.
    data= (struct Data*)add_ptr;

    //extraemos el semaforo de la estructura
    Semaphore *chicas = data->chicas;

    while(1){
        //pido permiso para entrar
        semaphore_wait(chicas);
        printf("Soy una chica y he entrado en el baño\n");
        semaphore_signal(chicas);
    }

}

int main(int argc, char const *argv[])
{
    //vamos a necesitar 1 semaforo con valor 5 para poder tener hasta 5 chicos en el baño
    //tambien vamos a necesitar otro semaforo por el que van a tener que esperar los chicos en caso de que una chica consiga entrar al baño
    
    Semaphore *mutex0=make_semaphore(1);
    Semaphore *mutex1=make_semaphore(1);
    int n=1;

    struct Data data;
    data.chicas=mutex0;
    data.chicos=mutex1;
    data.nchicos=n;

    pthread_t threads[10];
    int i;
    
    //creamos 2 procesos que incrementen la misma variable y que tengan el mismo mutex
    if(pthread_create(&threads[0], NULL, chicas,  &data)) {

        fprintf(stderr, "Error creating thread 1\n");
        return 1;

    } 

    for(i=1;i<10;i++){
        if(pthread_create(&threads[i], NULL, chicos,  &data)) {

           fprintf(stderr, "Error creating thread 2\n");
            return 1;

        } 
    }

    //si estamos esperando a que acabe 1 quye no acaba nunca obtenemos la ejecucion infinita
    if(pthread_join(threads[0], NULL)) {

        fprintf(stderr, "Error joining thread 1\n");
        return 2;

    }

    


    return 0;
}


