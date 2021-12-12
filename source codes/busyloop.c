//1900003421
//CANER EREN ÇALIŞKAN

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <threads.h> //library to use thrd_current() function , to get the thread id.


char banner[] = "\n\tbusyloop.c creates 2 threads running under the main thread\n"
		"  \t  -create- copies lines from stdin to a file until EOF(ctrl+d) is entered\n"
		"  \t  -display- lists records stored in the file on /dev/pts/1\n"
		"\topen a new terminal; verify that it is /dev/pts/1\n"
		"\t\tdisplay ther execution context using the commands:\n"
		"\t\t  ps -Lf -p 'PID'\n"
		"\t\t  pstree -p 'PPID'\n\n";
char fn[] = "log.txt"; //global fn variable , the name of a file to be created.
int count = 0; 	//global count variable to be used in both create and list threads.

void *createThread(void *filename) {

int fd;
char buf[150];
printf("create > TID <%lu> running\t\tPID <%d>\tPPID <%d>\n" , thrd_current() , getpid() , getppid());
//creating log.txt file
 if((fd = open( fn, O_WRONLY | O_TRUNC | O_CREAT | O_SYNC , S_IRUSR | S_IWUSR)) == -1) {
	perror("could not open/create log.txt file");
	return NULL; //because of we are in the void function (thread) we can only return NULL.
    }
 
 
 while(fgets(buf , sizeof(buf)-1 , stdin) != NULL) { //reading from stdin until the EOF , ctrl+d.
	write(fd, buf, strlen(buf)); //writing user inputs to log.txt file.
	count++; //incrementing count variable for the 'list' thread.
	
} 
 
 close(fd);//closing log.txt file.
 printf("\ncreate > TID <%lu> exits\n" , thrd_current()); //printing an termination message.
 
pthread_exit(NULL); //'create' thread is exiting.
}



void *listThread(void *filename) {
//printing thread id , process id and parent process id to stdout.
printf("list >   TID <%lu> running\t\tPID <%d>\tPPID <%d>\n", thrd_current() , getpid() , getppid());
long int fpos;	  //we will use fpos to check file size of log.txt.
FILE *ptw;	  //pseudo terminal window pointer.
FILE *inp;	  //pseudo terminal window pointer.
	
int i = 0;	  //while loop counter.
char buf[150];	  //local buffer to store log.txt records.
int checkcount = 0;
//opening the second terminal window /dev/pts/1
while((ptw = fopen("/dev/pts/1" , "w")) == NULL) {
	perror("/dev/pts/1 could not opened\n");
	sleep(1);
	return NULL;
 }
 
//local checkcount variable will be compared with global count variable. they both has 0 value as default. 

/*
int j=0;
//this while loop is only for checking if user is started entering records.
//if user is started entering records , it will print an info message and loop will break.
while(j < 4) {
if(count > checkcount) {
	fprintf(ptw , "\nlist thread starts now its monitoring ...\n");
	break; 
  }
else {
	sleep(3);
 }
j++;
}
*/


fprintf(ptw , "\nlist thread starts now its monitoring ...\n");
sleep(1);
//opening 'log.txt' file to read
if((inp = fopen(fn , "r")) == NULL) {
	perror("log.txt could not opened for reading\n");
	return NULL;
 }

while(i++ < 4) {
if(count > checkcount) { //check if there is new records; if user enter new records, 'create' thread will increment the global count variable by 1 and our 'if' statement will successfully work as needed.
  if((fgets(buf , 150-1 , inp)) == NULL) //reading from 'log.txt' file
	{perror("fgets error"); return NULL;} //printing an error message if fgets() fails.
  fputs(buf , ptw); //displaying the new record(s) to /dev/pts/1 terminal window.
  i = 0;	  //reseting the loop counter.
  checkcount++; //increasing local checkcount by 1 to keep the comparision continue.
 }
else
	sleep(3); //if there is no new records, waiting for 3 seconds
}
fprintf(ptw ,"list > Time out.. thread terminating..\n"); //displaying an timeout message on /dev/pts/1
printf("list >   TID <%lu> exits\n" , thrd_current());
sleep(1);
return NULL; 
}


int main() {
 puts(banner);
 printf("Main >   TID <%lu> master\t\tPID <%d>\tPPID <%d>\n" , thrd_current() , getpid() , getppid()); 
 
 //creating 'create' thread to get input from user.
 pthread_t create_thread_id;
 pthread_create(&create_thread_id , NULL , createThread, fn);
 
//creating 'list' thread to monitorizing 
 pthread_t list_thread_id;
 pthread_create(&list_thread_id , NULL , listThread , fn);

//making the main thread wait for termination of 'create' and 'list' threads.
 pthread_join(create_thread_id , NULL);
 pthread_join(list_thread_id , NULL); 

 printf("Main >   TID <%lu> exits\n" , thrd_current());
 sleep(1);
return 0;
}
