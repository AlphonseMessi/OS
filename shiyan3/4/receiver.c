#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/shm.h>
#include "common.h"


sem_t *sem_send;
sem_t *sem_recive;

int main()
{
    char msg[512];    
    int shmid = shmget(key, sizeof(message), IPC_CREAT | 0666);
    check_error(shmid);
    void *shmp = shmat(shmid, NULL, 0);
    sem_send = sem_open(name, O_CREAT, 0666, 1);
    sem_recive = sem_open(name, O_CREAT, 0666, 1);
    int i; 
    if(sem_getvalue(sem_recive,&i)==0){
		while(i>2)
			sem_wait(sem_recive);
		if(i==0)
			sem_post(sem_recive);
	}
    while(1){
    	sem_wait(sem_recive);
    	strcpy(msg, ((message*)shmp)->text);
	if(strcmp(msg,"exit")==0){
		strcpy(((message*)shmp)->text,"over");
		break;
	}else{
		if(strcmp(msg,"")!=0){
			printf("receive is : %s\n", msg);
			printf("len=%ld\n",strlen(msg));	
			strcpy(((message*)shmp)->text,"");
		}
	}
	sem_post(sem_recive); 
	
    }

    sem_close(sem_send);

    printf("receiver end\n");
    return 0;
}
