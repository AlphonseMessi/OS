#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<semaphore.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include <fcntl.h>

#define check_error(err)                                                 \
    do                                                                   \
    {                                                                    \
        if (err < 0)                                                     \
        {                                                                \
            fprintf(stderr, "warning is in  %d  ,waring code is %d\n", __LINE__, err); \
            exit(1);                                                     \
        }                                                                \
    } while (0);

void read_from_pipe(int fd){
    char buf[50];
    int n = read(fd,buf,sizeof(buf));
    buf[n] = '\0';
    printf("message is :\n%s\n", buf);
    printf("the true bytes : %d\n", n);
    printf("=====================================\n");
    //sleep(1);
}

void write_to_pipe1(int fd){
    char *buf="I'm child 1!\n";
    int n = write(fd,buf,strlen(buf));
    printf("the true bytes: %d\n", n);
    printf("=====================================\n");
    //sleep(1);
}

void write_to_pipe2(int fd){
    char *buf="I'm child 2!\n";
    int n = write(fd,buf,strlen(buf));
    printf("the true bytes: %d\n", n);
    printf("=====================================\n");
    //sleep(1);
}

void write_to_pipe3(int fd){
    char *buf="I'm child 3!\n";
    int n = write(fd,buf,strlen(buf));
    printf("the true bytes: %d\n", n);
    printf("=====================================\n");
    //sleep(1);
}

void wait_child(pid_t pid,int * status){
	waitpid(pid,status,0);
	printf("kill zombie child,pid:%d status:%d\n",pid,*status);
	printf("=====================================\n");
        //sleep(1);
}

#define name1 "mutex"
#define name2 "count"
int main(){
    pid_t pid1, pid2, pid3;
    int fd[2];
    int err;
    int status=-1;
    sem_t *mutex = sem_open(name1, O_CREAT, 0666, 1);
    sem_t *count = sem_open(name1, O_CREAT, 0666, 1);

    err = pipe(fd);
    check_error(err);
    
    // child 1
    pid1=fork();
    check_error(pid1);
    if(pid1 == 0){
        sem_wait(mutex);
	printf("on child 1,pid:%d\n",getpid());
        write_to_pipe1(fd[1]);
        sem_post(mutex);
	sem_post(count);
        return 0;
    }

    // child 2
    pid2 = fork();
    check_error(pid2);
    if(pid2 == 0){
        sem_wait(mutex);
	printf("on child 2,pid:%d\n",getpid());
        write_to_pipe2(fd[1]);
        sem_post(mutex);
	sem_post(count);
        return 0;
    }

    // child 3
    pid3 = fork();
    check_error(pid3);
    if(pid3 == 0){
        sem_wait(mutex);
	printf("on child 3,pid:%d\n",getpid());
        write_to_pipe3(fd[1]);
        sem_post(mutex);
	sem_post(count);
        return 0;
    }
    wait_child(pid1,&status);
    wait_child(pid2,&status);
    wait_child(pid3,&status);
    
    sem_wait(count);
    read_from_pipe(fd[0]);

    sem_unlink(name1);

    return 0;
}
