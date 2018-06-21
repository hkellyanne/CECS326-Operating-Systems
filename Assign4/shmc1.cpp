/* shmc1.cpp */
/*
import libraries
use our defined registration.h header file
sys/types.h includes data types
sys/ipc.h includes interprocess communication access structure
sys/sem.h defines semaphore constants and structures
sys/shm.h defines shared memory symbolic constants
sys/wait.h are includes declarations for waiting
Unistd.h is the standard symbolic constants and types
Stdlib.h defines several general purpose functions including dynamic memory management, random number generation, communication with the environment, integer arithmetics, searching, sorting and converting
Iostream defines the standard input/output string objects
stdio.h includes input/output operations
memory.h includes the dynamic memory management library
*/
#include "registration.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <cerrno>

//semaphore headers
#include <semaphore.h>       /* For semaphore functions */
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */

//semaphore variable
#define semName "/enrollment"

//allows you to define the context in which names are defined
using namespace std;

//declare necessary variables, objects and methods
CLASS   *class_ptr;
void    *memptr;
char    *pname;
int shmid, ret;
void rpterror(char *), srand(), perror(), sleep();
void sell_seats();
/*
initialize main
argc specifies the number of elements in argv
argv is an array or c-string pointers
*/
main(int argc, char* argv[])
{
   //open semaphore with same name as parent
   //O_RDWR allows child to read/write
   sem_t *semaphore = sem_open(semName, O_RDWR);
   //check if successful
   //in our case 5 will represent sem_open failure
   if (semaphore == SEM_FAILED){
       fprintf(stderr, "child: sem_open() failed.  errno:%d\n", errno);
       exit(5);
   }
   /*
   if there are less than 2 elements in the argv array,
   use fprintf to write formatted data to stream
   using stream, stderror being the pointer to an output file stream where contents are 
   written
   using format, "Usage:, %s shmid\n" pointing to a null terminated string that is written to 
   the file stream
   format %s specified as a string of characters for argv[0]
   exit return 1
   */
    if (argc < 2) {
        fprintf (stderr, "Usage:, %s shmid\n", argv[0]);
        exit(1);
    }
 //initialize pname to the first value stored in argv
    pname = argv[0];
   /*
   scanf reads formatted data from string and stores them according to parameters
   argv[1] is the cstring that the function processes as its source to retrieve the data
   format is specified with %d as a signed decimal for &shmid
   */
    sscanf (argv[1], "%d", &shmid);
   /*
   initialize memptr using shmat which attaches the shared memory segment
   using int shmid: shmid
   const void *shmaddr: (void * ) 0
   and int shmflg: 0
   in our case, shmaddr is a null pointer so the segment is attached at the first
   available address as selected by the system
   */
    memptr = shmat (shmid, (void *)0, 0);
   /*
   check if memptr failed
   if yes, call our defined rpterror method passing error message
   exit return 2
   */
    if (memptr == (char *)-1 ) {
        rpterror ("shmat failed");
        exit(2);
    }
   //initialize class_ptr as pointer to CLASS object
    class_ptr = (struct CLASS *)memptr;
   //call method sell_seats
    sell_seats();
   /*
   shmdt detaches the shared memory segment located at address memptr
   from the address space of the calling process

   exit return 0
   */
    ret = shmdt(memptr);
    exit(0);
}
//begin initialization of method sell_seats
void sell_seats()
{
   //open semaphore with same name as parent
   //O_RDWR allows child to read/write
   sem_t *semaphore = sem_open(semName, O_RDWR);
  
   //check if successful
   //in our case 5 will represent sem_open failure
   if (semaphore == SEM_FAILED){
       perror("sem_open() failed");
       exit(5);
   }

//initialize necessary variable
    int all_out = 0;

   /*
   srand seeds the pseudo random number generator with our the unsigned value of getpid()
   */
    srand ( (unsigned) getpid() );
    while ( !all_out) {   /* loop to sell all seats */
       //call sem_wait and check if successful
       if(sem_wait(semaphore) < 0){
           perror(strcat("sem_wait() failed on child process ", pname));
           continue;
       }
       //if there are still seats left
        if (class_ptr->seats_left > 0) {
           //sleep for calculated amount of time
            sleep ( (unsigned)rand()%5 + 1);
           //decrease seats_left by 1
            class_ptr->seats_left--;
           //sleep for calculated amount of time
            sleep ( (unsigned)rand()%5 + 1);
           //indicates seat has been taken and output the amount of seats left
            cout << pname << " SOLD SEAT -- "
         	    << class_ptr->seats_left << " left" << endl;
        }
        else {
           //exit out of the loop by increases all_out by 1
           //indicate that class is full
            all_out++;
            cout << pname << " sees no seats left" << endl;
        }

       //call sem_post and check if successful
       if(sem_post(semaphore) < 0){
           perror(strcat("sem_post() error on child ", pname));
       }
       //sleep for calculated amount of time
        sleep ( (unsigned)rand()%10 + 1);
    }
}
//begin initialization of method rpterror
void rpterror(char* string)
{
   //declare char array
    char errline[50];
   /*
   compose a string from errline
   store it as a C string in the buffer pointed to by, errline
   format specified with %s %s as a string of characters
   for string and pname respectively
   */
    sprintf (errline, "%s %s", string, pname);
   //throw error with given message
    perror (errline);
}
