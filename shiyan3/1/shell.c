#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<wait.h>
#include<string.h>

#define CMD_MAX_LEN 20
#define CMD_CASE_NUM 4

#define INVALID_CMD -1
#define EXIT 0
#define CMD_1 1
#define CMD_2 2
#define CMD_3 3

char *cmdStr[CMD_CASE_NUM]={"exit","cmd1","cmd2","cmd3"};

int getCmdIndex(char *cmd);
void runCmd(int cmdIndex);
void forkChild(int cmdIndex);

int main(){
	pid_t pid;
	char cmdString[CMD_MAX_LEN];
	int cmd_index;

	while(1)
	{
	printf("please input your command:\n");
	scanf("%s",cmdString);
	cmd_index = getCmdIndex(cmdString);
	runCmd(cmd_index);

	wait(0);
	printf("www waiting for next command\n");
	
	}


}

int getCmdIndex(char *cmd)
{
 	int i;
	for (i=0;i<CMD_CASE_NUM;i++)
	{
	 	if(strcmp(cmd,cmdStr[i])==0)
			return i;
	}
	return -1;

}

void runCmd(int cmdIndex)
{
 	switch(cmdIndex)
 	{
          case INVALID_CMD:
		  printf("Command not found\n");
		  break;
	  case EXIT:
		  exit(0);
		  break;
	  default:
		  forkChild(cmdIndex);
		  break;
 
 	}	

}

void forkChild(int cmdIndex){
	pid_t pid;
	pid = fork();
	if(pid<0)
	{
		printf("error in fork!\n");
	}

	else if(pid == 0)
	{
		printf("child process is running!!\n");

		int execl_status= -1;
		switch(cmdIndex)
		{
			case CMD_1:
				execl_status = execl("./cmd1","cmd1",NULL);
				break;
			case CMD_2:
				execl_status = execl("./cmd2","cmd2",NULL);
				break;
			case CMD_3:
				execl_status = execl("./cmd3","cmd3",NULL);
				break;
			default:
				printf("error in cmd index!!!\n");
				break;
		}

		if(execl_status < 0)
		{
		 printf("error in cmd index!\n");
		 exit(0);
		
		}

		printf("child process is done!\n");
		exit(0);
	
	
	
	}
}
