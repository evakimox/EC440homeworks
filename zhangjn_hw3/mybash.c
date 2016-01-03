#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define READ_END 0
#define WRITE_END 1
int interrupts_remaining;   // number of interrupts remaining
int maxints=3;
char handler_message[50]="Quit the terminal.";   // the message from the handler
int new_message_flag;       // is there a new_message?

void redirect1(){
  pid_t pid;
  int fd;
  pid=fork();
  if(pid<0){
    perror("fork failed!");
  }

  if(pid==0){
    fd=open("filename.txt",O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
    close(1);
    dup(fd);
    close(fd);
 char *st1 = "/bin/open";
 char *st2 = "open";
 execlp(st1, st2, (char *) NULL);
  } //execvp(st1,st2);
  waitpid(pid,NULL,0);
  return ;
}

void handler(int signo) { /* generic form of a simpler signal handler */
  if (--interrupts_remaining>1){
    sprintf(handler_message,"You have %d chances remaining.",interrupts_remaining);
  } else if (interrupts_remaining==1){
    sprintf(handler_message,"You have ONE chance remaining.",interrupts_remaining);
  } else {
    sprintf(handler_message,"Now you are on your own!");
    signal(SIGINT,SIG_DFL);  // restore default handler
  }
  new_message_flag=1;
}

void setup_handler(int max){
  interrupts_remaining=max;
  new_message_flag=0;
  signal(SIGINT,handler);
}


int pipexec(){
  int fd[2];
  pid_t pid;
  char write_msg[50];
  char read_msg[50];
  char *SCRIPTPATH = "/bin/ls";
  char *SCRIPTNAME = "ls";
  //create pipe
  if (pipe(fd) == -1) {
    fprintf(stderr,"Pipe failed");
    return 1;
  }

  /* now fork a child process */
  pid = fork();

  if (pid < 0) {
    fprintf(stderr, "Fork failed");
    return 1;
  }

  if (pid > 0) {  /* parent process */
    /* close the unused end of the pipe */
    close(fd[READ_END]);

    /* write ONLY the non-0 characters to the pipe */
    write(fd[WRITE_END], write_msg, strlen(write_msg));

    /* close the write end of the pipe */
    close(fd[WRITE_END]);
  }
  else { /* child process */
    /* close the unused end of the pipe */
    close(fd[WRITE_END]);
    /* duplicate the read end into stdin for the process */
    dup2(fd[READ_END],0);
    /* execute another program inheriting the new environment*/
    execlp(SCRIPTPATH, SCRIPTNAME, (char *) NULL);
  }
  waitpid(pid,NULL,0);
  return 1;
}




int main(void)
{
  char c;
  char *sen[10];
  int should_run=1;
  int usless;
  
  setup_handler(maxints);
//this one handles "^C" for 3 times
 jump:
  while(should_run){
    fflush(stdout);
    printf("Jingning > ");
  for(int i=0;i<10;i++)
    sen[i]=malloc(100*sizeof(char));
  int i=0;
  int j=0;
  
  //get token starts here:
  c=getchar();
  while(c!='\n')
    {
      if(c=='\n'){
	break;
      }
      else if(c==' '){
      	j=0;
	i++;
      }
      else if(c=='|'||c=='&'||c=='<'||c=='>'){
	i++;j=0;sen[i][j]=c;
	i++;
      }
      else{
	sen[i][j]=c;
	j++;
      }
      c=getchar();
      if(should_run==2){
	should_run=1;
	goto jump;
      }
    }//put the tokens into an array sen[]
  // all token are ready
  usless = pipexec();
  //  redirect1();  
  
  /* for(int k=0;k<=i;k++)
    {
      printf("here is token \n %s\n",sen[k]);
    }*/
  }//Everything in this bracket
  
}
