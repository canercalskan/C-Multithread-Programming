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
#include <semaphore.h> //library to use semaphores.

int createalive; //this global variable will be reporting the activation and termination of the 'create' thread.
sem_t fileopen; //this semaphore will be posting the creation event of the output file.
sem_t recnb;   //this semaphore will be counting the records written to the file.


char banner[] = "\n\n\tsemsolution.c uses semaphores to build thread coordination\n";
char fn[] = "log.txt"; //global fn variable , the name of a file to be created.
int count = 0; 	//global count variable to be used in both create and list threads.

void *createThread(void *filename) {
createalive = 1; //reporting the 'active' status.
int fd;
char buf[150];
printf("create > TID <%lu> running\t\tPID <%d>\tPPID <%d>\n" , thrd_current() , getpid() , getppid());
//creating log.txt file

 if((fd = open( fn, O_WRONLY | O_TRUNC | O_CREAT | O_SYNC , S_IRUSR | S_IWUSR)) == -1) {
	perror("could not open/create log.txt file");
	return NULL; //because of we are in the void function (thread) we can only return NULL.
    }
 sem_post(&fileopen); //output file has created , posting this event.
 
 
 while(fgets(buf , sizeof(buf)-1 , stdin) != NULL) { //reading from stdin until the EOF , ctrl+d.
	write(fd, buf, strlen(buf)); //writing user inputs to log.txt file.
	count++; //incrementing count variable for the 'list' thread.
	sem_post(&recnb); //counting the records.
} 
 
 createalive = 0; //reporting the 'terminated' status.
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
 }
 
sem_wait(&fileopen); //waiting for opening post of a file from 'create' thread.
//opening 'log.txt' file to read
if((inp = fopen(fn , "r")) == NULL) {
	perror("log.txt could not opened for reading\n");
	return NULL;
}
int infomsg = 0;
 
do {
   while(sem_trywait(&recnb) == 0) {
	 if(infomsg == 0)
	    {fprintf(ptw , "\nlist thread starts now its monitoring ...\n"); infomsg++;}
	   
	 if((fgets(buf , 150-1 , inp)) == NULL) //reading from 'log.txt' file
	    {perror("fgets error"); return NULL;} //printing an error message if fgets() fails.
	   sleep(2);
	   fputs(buf , ptw); //displaying the new record(s) to /dev/pts/1 terminal window.
         }
   sleep(1); //delaying busy loop.
 } while(createalive > 0);

fprintf(ptw ,"list > thread terminating..\n"); //displaying an timeout message on /dev/pts/1
printf("list >   TID <%lu> exits\n" , thrd_current());
fclose(inp);
sleep(1);
return NULL; 
}


int main() {
 puts(banner);
 printf("Main >   TID <%lu> master\t\tPID <%d>\tPPID <%d>\n" , thrd_current() , getpid() , getppid()); 
 
 //creating 'create' thread to get input from user.
 pthread_t create_thread_id;
 pthread_create(&create_thread_id , NULL , createThread, fn);
 
 sem_init(&fileopen , 0 , 0); //initializing fileopen status semaphore.
 sem_init(&recnb , 0 , count); //initializing recnb semaphore with 'count' shared variable, because we will count user inputs every time he/she enters. and the 'list' thread will continue until counting is finished, in other words , user has stopped entering new records.
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
