/* shmp1.cpp */

/*
assignment #4
I have decided to use the POSIX method defined in <semaphore.h>
this includes these functions:
sem_wait()
   decrement the semphaphore by one, if 0 then wait will block until value is greater than 0.
sem_post()
   increments the semaphore by one
sem_open()
   allows two processes to operate the same semaphore. it can also create a new named semaphore. operated using post and wait.

I will implement an named semaphare which will include these functions:
sem_close()
   closes the semaphore
sem_unlink()
   removes semaphore from the system when all processes have finished
*/

/*
import libraries
use our defined registration.h header file
sys/types.h includes data types
sys/ipc.h includes interprocess communication access structure
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

//semaphore variables
const char semName[] = "/enrollment";
#define semPerms 0644
#define initVal 1

//allows you to define the context in which names are defined
using namespace std;

/*
create object of type CLASS
class number is 1001
date is 120186
title is Operating Systems
seats left is 15
*/
CLASS myclass = { "1001", "120186", "Operating Systems", 15 };

/*
defines a macro that tells the preprocessor to find all
instances of NCHILD and replace it with 3
*/
#define NCHILD  3

//declare methods shm_init, wait_and_wrap_up, and rpterror
int shm_init( void * );
void    wait_and_wrap_up( int [], void *, int );
void    rpterror( char *, char * );

/*
initialize main
argc specifies the number of parameters in argv
argv is an array or c-string pointers
*/
main(int argc, char *argv[])
{
    //declare necessary variables
    int     child[NCHILD], i, shmid, semid;
    void    *shm_ptr;
    char    ascshmid[10], ascsemid[10], pname[14];
   //semid unused
   //ascsemid[10] unused

   //initialize semaphore and check if successful
   //using O_CREAT which will create the semaphore if it does not already exist
   //in our case 5 will represent sem_open failure
   sem_t *semaphore = sem_open(semName, O_CREAT, semPerms, initVal);
  
   if (semaphore == SEM_FAILED){
       fprintf(stderr, "parent: sem_open() failed.  errno:%d\n", errno);
       exit(5);
   }

   //close semaphore and unlink if failed to prevent it from existing forever
   if (sem_close(semaphore) < 0){
       perror("sem_close() failed");
       sem_unlink(semName);
       exit(6);
   }
   /*
   copies Cstring pointed by source, argv[0]
   to destination, pname.
   */
    strcpy (pname, argv[0]);
   /*
   initialize shmid to be the value returned by our defined method
   shm_init by passing ine shm_ptr
   */
    shmid = shm_init(shm_ptr);
   /*
   compose a string from shmid
   store it as a C string in the buffer pointed to by, ascshmid
   format specified with %d as a signed decimal for shmid
   */
    sprintf (ascshmid, "%d", shmid);

   //fork up to NCHILD children and check if fork was successful
    for (i = 0; i < NCHILD; i++) {
        child[i] = fork();
        switch (child[i]) {
       //if fork failed call method rpterror and throw passed message as error  
        case -1:
            rpterror ("fork failure", pname);
            exit(1);
       //if for succeeded
        case 0:
           /*
           use sprint f to compose a string from i+1,
           store it as a C string in the buffer pointed to by, pname
           format specified with %d as a signed decimal for i+1
           and concatenated to "shmc"
           */
            sprintf (pname, "shmc%d", i+1);
           /*
           use execl to execute a shell command
           on the path shmc1
           using command ascshmid
           make zero as a character array
           */
            execl("shmc1", pname, ascshmid, (char *)0);
           /*
           use perror to interpret the vallue of errno and prints it to
           stderr with custom message
           */ 
            perror ("execl failed");
           //exit return 2
            exit (2);
        }
    }
   //end main by calling wait_and_wrap_up method
    wait_and_wrap_up (child, shm_ptr, shmid);

   //after waiting for processes to complete, unlink to prevent it from existing forever
   //throw error if unsuccessful
   if(sem_unlink(semName) < 0){
       perror("sem_unlink() failed");
       exit(7);
   }
}
//begin initialization of method, shm_init
int shm_init(void *shm_ptr)
{
   //declare necessary variables
    int shmid;
   /*
   initialize shmid by using shmget which allocates a System V shared memory segment
   using key_t key: ftok(".",'u')
   ftok converts a pathname and a project identifier to a System V IPC key
   "." represents the current directory
   'u' represents the value used to generate an IPC key
   size_t size: sizeof(CLASS)
   and int shmflg: 0600 | IPC_CREAT
   giving permissions to owner to read and write and creating a new segment
   */
    shmid = shmget(ftok(".",'u'), sizeof(CLASS), 0600 | IPC_CREAT);
   //check if shmid was created successfully
   /*
   if shmid is -1, the creation of a shared memory segment has failed
   perror will interpret the vallue of errno and prints it to
   stderr with custom message

   exit return 3
   */
   if (shmid == -1) {
        perror ("shmget failed");
        exit(3);
    }
   /*
   initialize shm_ptr using shmat which attaches the shared memory segment
   using int shmid: shmid
   const void *shmaddr: (void * ) 0
   and int shmflg: 0

   in our case, shmaddr is a null pointer so the segment is attached at the first
   available address as selected by the system
    */
   shm_ptr = shmat(shmid, (void * ) 0, 0);
    if (shm_ptr == (void *) -1) {
        perror ("shmat failed");
       //exit return 4
        exit(4);
    }
   /*
   memcpy copies block of memory
   at destination: shm_ptr
   source: (void *) &myclass
   and number of bytes: sizeof(CLASS)
   */
    memcpy (shm_ptr, (void *) &myclass, sizeof(CLASS) );
   //return value shmid
    return (shmid);
}
//begin initialization of method wait_and_wrap_up
void wait_and_wrap_up(int child[], void *shm_ptr, int shmid)
{
   //declare necessary variables
    int wait_rtn, w, ch_active = NCHILD;
   //if a child is still active
    while (ch_active > 0) {
       //initialize wait_rtn to wait value
        wait_rtn = wait( (int *)0 );
        for (w = 0; w < NCHILD; w++)
           //wait until each child finishes execution
            if (child[w] == wait_rtn) {
                ch_active--;
                break;
            }
    }
   //output indicated that shared memory will be removed
    cout << "Parent removing shm" << endl;
   /*
   shmdt detaches the shared memory segment located at address shm_ptr
   from the address space of the calling process
   */
    shmdt (shm_ptr);
   /*
   performs the control operation on the system V shared memory segment with identifier 
   shmid
   with cmd IPC_RMID which marks the segment to be destroyed. this segment will be
   destroyed after the last process detaches it.
   shmid_ds is the pointer defined in <sys/shm.h>
   */
    shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0);
   //exit return 0
    exit (0);
}
//begin initialization of method rpterror
void rpterror(char *string, char *pname)
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
