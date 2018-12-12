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
    // init
	char msg[512];
	
	strcpy(msg,"");
    	int shmid = shmget(key, sizeof(message), 0666 | IPC_CREAT);
	check_error(shmid);
    	void* shmp = shmat(shmid, NULL, 0);
    	sem_send = sem_open(name, O_CREAT, 0666, 1);
	sem_recive = sem_open(name, O_CREAT, 0666, 1);
	int i;
	if(sem_getvalue(sem_send,&i)==0){
		while(i>2)
			sem_wait(sem_send);
		if(i==0)
			sem_post(sem_send);
	}
   		
    while(1){
	sem_wait(sem_send);	
        sleep(2);
        if(strcmp(((message*)shmp)->text,"over")==0){
		printf("sender receive:over\n");
		sem_close(sem_send);
		break;
	}	
        printf("input:");
		
	scanf("%s",msg);
	
	strcat(((message*)shmp)->text,msg);
	printf("msg:%s,len:%ld\n",((message*)shmp)->text,strlen(((message*)shmp)->text));		
	
		
	sem_post(sem_send);
    }

	sem_unlink(name);

    	shmctl(shmid, IPC_RMID, shmp);
    	printf("sender end\n");
    	return 0;
}
