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
	char msgBuf[512];
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
	
     /*	if(strlen(((message*)shmp)->text)+strlen(msg)>=MAX_SIZE){
		printf("buffer is full! please wait!\n");
		int len=MAX_SIZE-strlen(((message*)shmp)->text);
		if(strlen(msg)>MAX_SIZE){
			int count=((strlen(msg)-len)/MAX_SIZE)+1+1;
			int index=0;
			printf("This message will be dived %d to send!!!\n",count);
			while(index<count){
				len=MAX_SIZE-strlen(((message*)shmp)->text);
				printf("Sending the number of %d message\n",index+1);
				strncat(((message*)shmp)->text,msg,len);
				strncpy(msg,msg+len,512);
				index++;
				if(index<count){
					sem_wait(sem_send);
				}else sleep(1);
			}
		}else{
			strncat(((message*)shmp)->text,msg,len);
			sem_wait(sem_send);
		}
		
	}*/
	
		strcat(((message*)shmp)->text,msg);
		printf("msg:%s,len:%ld\n",((message*)shmp)->text,strlen(((message*)shmp)->text));		
	
		
	sem_post(sem_send);
    }

	sem_unlink(name);

    	shmctl(shmid, IPC_RMID, shmp);
    	printf("sender end\n");
    	return 0;
}
