#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>




void *sender1(void *arg);
void *sender2(void *arg);
void *receiver(void *arg);


sem_t send, rec, final_rec_1, final_rec_2;	
	
#define N 4096
struct msg_data {
  long type;
  char text[N];
  int mw2w;
};


int id;

int send_1_over = 0;
int send_2_over = 0;

void *sender1(void *arg)
{
    char s[500];                                     	
    struct msg_data tmp;			
    tmp.type = 1;
    tmp.mw2w = 1;
    while (1) {
        sem_wait(&send);

	   
        printf("Sender1 send: ");
        scanf("%s", s);
  
        if ((strcmp(s, "exit") == 0) ) {
            strcpy(tmp.text, "end1");
            msgsnd(id, &tmp, sizeof(tmp), 0);
            sem_post(&rec);
            break;
        }
	else{
        strcpy(tmp.text, s); 
        msgsnd(id, &tmp, sizeof(tmp), 0);
        sem_post(&rec);
	}
	
    }

    sem_wait(&final_rec_1);					
    msgrcv(id, &tmp, sizeof(tmp), 2, 0);	
    printf("Sender1 receive: %s\n", tmp.text);
    

    sem_post(&send);					
  
  	
    pthread_exit(NULL);					
}


void *sender2(void *arg)
{
    char s[500];                                     	
    struct msg_data tmp;			
    tmp.type = 1;
    tmp.mw2w = 2;
    while (1) {
        sem_wait(&send);

	   
        printf("Sender2 send: ");
        scanf("%s", s);
  
        if ((strcmp(s, "exit") == 0))  {
            strcpy(tmp.text, "end2");
            msgsnd(id, &tmp, sizeof(tmp), 0);
            sem_post(&rec);
            break;
        }
        strcpy(tmp.text, s);
        
        msgsnd(id, &tmp, sizeof(tmp), 0);
        sem_post(&rec);
	
    }
    sem_wait(&final_rec_2);					
    msgrcv(id, &tmp, sizeof(tmp), 2, 0);	
    printf("Sender2 receive: %s\n", tmp.text);
    

    sem_post(&send);					
  
 			
    pthread_exit(NULL);					
}

void *receiver(void *arg)
{
    struct msg_data tmp;				
    while (1) {
        sem_wait(&rec);
        msgrcv(id, &tmp, sizeof(tmp), 1, 0);
	if (tmp.mw2w == 1){		
            if (strcmp(tmp.text, "end1") == 0) {
            	strcpy(tmp.text, "over1");
            	tmp.type = 2;
		tmp.mw2w = 3;
            	msgsnd(id, &tmp, sizeof(tmp), 0);
            	printf("Receiver receive 'end1' from sender1, return 'over1'\n");
                send_1_over = 1;
            	sem_post(&final_rec_1);
             
            }
            else {
            	printf("Receiver receive: %s from Sender1\n", tmp.text);
	    	sem_post(&send);
	    }
	}
	else if (tmp.mw2w == 2 ){	
            if (strcmp(tmp.text, "end2") == 0) {
            	strcpy(tmp.text, "over2");
            	tmp.type = 2;
		tmp.mw2w = 4;
            	msgsnd(id, &tmp, sizeof(tmp), 0);
            	printf("Receiver receive 'end2' from Sender2, return 'over2'\n");
                send_2_over = 1;
            	sem_post(&final_rec_2);
		       
            	
            }
            else {
                printf("receiver receive: %s from Sender2\n", tmp.text);
	        sem_post(&send);
	    }
	}
	if(send_1_over && send_2_over)
	{
	     break;
	}

	
    }
    pthread_exit(NULL);
}

int main(void)
{
    sem_init(&send, 0, 1);				
    sem_init(&rec, 0, 0);
    sem_init(&final_rec_1, 0, 0);
    sem_init(&final_rec_2, 0, 0);

    
   id = msgget(0, 0666);
   if (id < 0) {
     printf("msgqueue build error\n");
     return 0;
   }

   pthread_t id1, id2,id3;
   int t1 = pthread_create(&id1, NULL, sender1, NULL);
   if (t1 < 0) {
     printf("t1 error\n");
     return 0;
   }
   int t2 = pthread_create(&id2, NULL, sender2, NULL);
   if (t2 < 0) {
     printf("t1 error\n");
     return 0;
   }
   int t3 = pthread_create(&id3, NULL, receiver, NULL);
   if (t3 < 0) {
     printf("t3 error\n");
     return 0;
  }

    pthread_join(id1, NULL);
    pthread_join(id2, NULL);
    pthread_join(id3, NULL);

    return 0;
}
