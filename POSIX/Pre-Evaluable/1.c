#include <pthread.h>
#include <stdio.h>


void *print_thread(void *print_ptr){
    int i;
    for(i=0;i<100;i++){
        printf("My character is: %c\n",* (char *)print_ptr);
    }

}


int main(int argc, char const *argv[])
{
    char a='a';
    char b='b';
    char c='c';
    pthread_t print_thread_a;
    pthread_t print_thread_b;
    pthread_t print_thread_c;
    //Creating threads 
    if(pthread_create(&print_thread_a, NULL, print_thread, &a)) {

        fprintf(stderr, "Error creating thread a\n");
        return 1;

    }   
    if(pthread_create(&print_thread_b, NULL, print_thread, &b)) {

        fprintf(stderr, "Error creating thread b\n");
        return 1;

    }
    if(pthread_create(&print_thread_c, NULL, print_thread, &c)) {

        fprintf(stderr, "Error creating thread c\n");
        return 1;

    } 
    //Waiting threads to exit
    if(pthread_join(print_thread_a, NULL)) {

        fprintf(stderr, "Error joining thread a\n");
        return 2;

    }
    if(pthread_join(print_thread_b, NULL)) {

        fprintf(stderr, "Error joining thread b\n");
        return 2;

    }
    if(pthread_join(print_thread_c, NULL)) {

        fprintf(stderr, "Error joining thread c\n");
        return 2;

    }

} 

