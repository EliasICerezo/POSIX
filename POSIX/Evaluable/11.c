#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>


void *tareaA(void *ptr);
void check(int n);
static volatile int cnt=0;
static void handler(int sig, siginfo_t *siginfo, void *context);

int main(int argc, char const *argv[])
{
    struct sigaction sigact;
    timer_t timerid;
    struct sigevent sgev;
    struct itimerspec its;
    struct timespec ts;
    int signum; 
    //struct timerspec ts = {1,0};

    sigact.sa_sigaction = handler;
    check(sigemptyset(&sigact.sa_mask));
    sigact.sa_flags= SA_SIGINFO | SA_RESTART;
    check(sigaction(SIGUSR1, &sigact, NULL));

    
    sgev.sigev_notify=SIGEV_SIGNAL;
    sgev.sigev_signo = SIGUSR1;
    sgev.sigev_value.sival_ptr = &timerid;
    check(timer_create(CLOCK_MONOTONIC, &sgev, &timerid));
    ts.tv_sec=1;
    ts.tv_nsec=0;
    its.it_interval=ts;
    its.it_value.tv_sec=1;
    its.it_value.tv_nsec=0;

    check(timer_settime(timerid, 0, &its, NULL));

    sigset_t sigset;
    check(sigemptyset(&sigset));
    check(sigaddset(&sigset, SIGUSR2));
    check(pthread_sigmask(SIG_BLOCK, &sigset, NULL));
    
    printf("Soy el main y mi PID es: <%d>\n",getpid());

    sigwait(&sigset, &signum);
    
    printf("Soy el main y he recibido SIGUSR2. Terminando...\n");

    return 0;
}

void check(int n){
    if(n!=0){
        printf("Error: <%d>",n);
        exit(-1);
    }
}

static void handler(int sig, siginfo_t *siginfo, void *context){
    cnt++;
    printf("Soy el manejador del contador y ahora la variable vale:<%d> \n",cnt);
}