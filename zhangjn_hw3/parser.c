# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
#include  <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <signal.h>
#include <sys/wait.h>
//# include "parser.h"



int main(){
		int i=0;
		setupHandler(MAX_INTS);
	 	while(1){
			getCommandLine();
			int mode=getMode();
			tokenToArgs();
			executeSingleCommand(mode);
			fflush(stdout);
		}		
	return 0;
}


void setupHandler(int max){
	interrupts_remaining=max;
	signal(SIGINT,handler);
}

void handler(int signo){
	if(--interrupts_remaining<0){
		printf("now you will go back to real shell\n");
		signal(SIGINT,SIG_DFL);
	}
}

void executeSingleCommand(int mode){
	switch(mode){
		case 7:
			pipe_command();
			break;
		case 0:
			redirect1();
			break;
		case 1:
			redirect1();
			break;
		case 2: 
			redirect2();
			break;
		case 6:
			redirect7();
			break;
		case 3: 
			append3();
			break;
		case 4:
			appendError4();
			break;
		case 5:
			both5();
			break;
		}
}


void pipe_command(){
	int fd[2];
	pid_t pid1,pid2;

	pipe(fd);
	//char* args[]={"grep","c",NULL};

	/* create the first child and run parent_args */
	pid1=fork();
	if(pid1<0){
		perror("first fork error");
		return ;
	}
	/* set the process output to the input of pipe*/
	if(pid1==0){
		close(1);
		dup(fd[1]);
		close(fd[0]);
		close(fd[1]);
		execvp(parent_args[0],parent_args);
		perror("execvp() error");
		return ;
	}

	pid2=fork();
	if(pid2<0){
		perror("fork error");
		return ;
	}

	if(pid2==0){
		close(0);
		dup(fd[0]);
		close(fd[0]);
		close(fd[1]);
		execvp(child_args[0],child_args);
		//execvp("grep",args);
		perror("second execvp() failed");
		return ;
	}
	close(fd[0]);
	close(fd[1]);
	/*wait for the children to finish, then exit*/
	waitpid(pid1,NULL,0);
	waitpid(pid2,NULL,0);
	//return 0;    
}

//Redirect the standard output (stdout) of cmd to a file
//cmd 1> file and cmd> file
void redirect1(){
	pid_t pid;
	int fd;
	pid=fork();

	if(pid<0){
		perror("fork failed!");
	}

	if(pid==0){
		fd=open(child_args[0],O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
		close(1);
		dup(fd);
		close(fd);
		execvp(parent_args[0],parent_args);
	}
	waitpid(pid,NULL,WNOHANG);
	return ;
}

/* redirect standard error of cmd to a file */
/* cmd 2> file */
void redirect2(){
	pid_t pid;
	int fd;
	pid=fork();

	if(pid<0){
		perror("fork failed");
	}

	if(pid==0){
		fd=open(child_args[0],O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
		dup2(fd,STDERR_FILENO);
		close(fd);
		execvp(parent_args[0],parent_args);
	}
	waitpid(pid,NULL,WNOHANG);
	return ;
}

//redirect standard input to file
void redirect7(void){
	pid_t pid;
	int fd;
	pid=fork();

	if(pid<0){
		perror("fork failed");
		return;
	}

	if(pid==0){
		fd=open(child_args[0],O_RDONLY);
		close(0);
		dup(fd);
		close(fd);
		execvp(parent_args[0],parent_args);
	}
	waitpid(pid,NULL,WNOHANG);
	return ;
}

//append standard error cmd 2>> file
void appendError4(void){
	pid_t pid;
	int fd;
	pid=fork();

	if(pid<0){
		perror("fork failed");
	}

	if(pid==0){
		fd=open(child_args[0],O_WRONLY|O_APPEND);
		dup2(fd,STDERR_FILENO);
		close(fd);
		execvp(parent_args[0],parent_args);
	}
	waitpid(pid,NULL,WNOHANG);
	return ;

}

void both5(void){
	pid_t pid;
	int fd1,fd2;
	pid=fork();

	if(pid<0){
		perror("fork failed");
	}

	if(pid==0){
		fd1=open(child_args[0],O_WRONLY|O_APPEND);
		fd2=open(child_args[0],O_WRONLY|O_APPEND);
		
		dup2(fd1,2);
		dup2(fd2,1);

		close(fd1);
		close(fd2);

		execvp(parent_args[0],parent_args);
	}
	waitpid(pid,NULL,WNOHANG);
	return ;

}

//append standard output cmd >> file
void append3(void){
	pid_t pid;
	int fd;
	pid=fork();

	if(pid<0){
		perror("fork failed!");
	}

	if(pid==0){
		fd=open(child_args[0],O_WRONLY|O_APPEND);
		close(1);
		dup(fd);
		close(fd);
		execvp(parent_args[0],parent_args);
	}
	waitpid(pid,NULL,WNOHANG);
	return ;
}


int getMode(){
	//char* ret;
	int mode=0;
	int i=0;
	while(str[i]!='\0'){
		if(str[i]=='>'&&str[i+1]!='>')
			 return mode=0;
		if(str[i]=='>'&&str[i+1]=='>')
			 return mode=3;
		if(str[i]=='1'&&str[i+1]=='>')
			 return mode=1;
		if(str[i]=='2'&&str[i+1]=='>')
			return mode=2;
		if(str[i]=='&'&&str[i+1]=='>')
			return mode=5;
		if(str[i]=='<')
			return mode=6;
		if(str[i]=='|')
			return mode=7;
		if(str[i]=='2'&&str[i+1]=='>'&&str[i+2]=='>')
			return mode=4;
		i++;
	}

	return mode=1000;

}



void getCommandLine(){
	fgets(str,MAX_TOKEN_SIZE,stdin);
}

void tokenToArgs(){
	int temp=getToken();
	//char* token[0] parent token, char* token[1] child token
	int i=0;
	char* p0=token[0];
	char* p1=token[1];
	//split p1 p2 by space
	char dest0[50];
	char dest1[50];

	strncpy(dest0,p0,sizeof dest0-1);
	strncpy(dest1,p1,sizeof dest1-1);
	//get rid of \n
	char* newline=strchr(dest1,'\n');
	*newline=0;

	dest0[49]='\0';
	dest1[49]='\0';
	//remove all blanks

	char deli_t[]=" \n\t";
	char* copy=strdup(dest0);
	char* t;
	t=strtok(copy,deli_t);
	while(t!=NULL){
		parent_args[i++]=t;
		t=strtok(NULL,deli_t);
	}
	
	i=0;
	char* t1;
	char* copy1=strdup(dest1);
	t1=strtok(copy1,deli_t);
	while(t1!=NULL){
		child_args[i++]=t1;
		t1=strtok(NULL,deli_t);
	}	

	return;

}

int getToken(){
	char* p;
	int i=0;
	char* copy=strdup(str);

	p=strtok(copy,deli);

	while(p!=NULL){
		token[i++]=p;
		p=strtok(NULL,deli);
	}

	return i; //token number
}
