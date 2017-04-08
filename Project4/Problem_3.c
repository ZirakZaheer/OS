#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "crc32.c"
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <pthread.h>
#include "assert.h"
#include "time.h"

int directoryType = 0;
char* dirName = NULL;
char arrDir[100][100];
char returnArr1[100][100];
char fileName[1000]  = "";
char filedata[1000000];
struct stat filestat;
int fileSize =0;
volatile int fillingIndex = 0;
volatile uint32_t crc1 = 0;
FILE* fd;
volatile int fdesc = 0;
volatile int arrayAccessIndex = 0;
pthread_mutex_t lock; // = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CV_workComplete = PTHREAD_COND_INITIALIZER;

static int compareFunction(const void *text1, const void *text2)
{
    const char *left = text1;
    const char *right = text2;
    return  strcmp (left, right);
}


void* calculateCRC(void* arg) {
  
   while (arrayAccessIndex < fillingIndex) {
      int fdesc1 = 0;
      char filedata1[1000000];
      uint32_t crc = 0;
      pthread_mutex_lock(&lock);
      if (arrayAccessIndex >= fillingIndex) {
         pthread_mutex_unlock(&lock);
        break;
      }
      crc = 0;


      
      stat((arrDir[arrayAccessIndex]), &filestat);
      pthread_mutex_unlock(&lock);
      fileSize = filestat.st_size;
      pthread_mutex_lock(&lock);
      fdesc1 = open((arrDir[arrayAccessIndex]), O_RDONLY);
      size_t sizefile = read(fdesc1, filedata1, fileSize);
       
      pthread_mutex_unlock(&lock);
     


      crc =  crc32(0, filedata1, sizefile);
      

      pthread_mutex_lock(&lock);
      printf(" %s  ", (arrDir[arrayAccessIndex]));
      printf("%X\n", crc);

      
      arrayAccessIndex++;
      pthread_mutex_unlock(&lock);         
     }
      pthread_mutex_lock(&lock);
   
     if (arrayAccessIndex >= fillingIndex) {  

       

       pthread_cond_signal(&CV_workComplete);
       pthread_mutex_unlock(&lock);

     }  else {
        pthread_mutex_unlock(&lock);
     }

   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  //struct stat info;
  DIR *dirport;
  struct dirent* dirPointer;
  if (argc < 2) {
    printf("%s\n", "Wrong Number of Inputs");
    exit(0);
  }
  int i,rc;
  int numofThreads = strtol(argv[2], NULL,10);
  printf("%d\n", numofThreads);
  pthread_t thr[numofThreads];
 
  if (argc > 1)
    {
    dirport = opendir(argv[1]); 
    char* fixed = argv[1];
    do
    {

      dirPointer = readdir(dirport);
      dirName = fixed;
 
      if (dirPointer)
      {
        directoryType = dirPointer->d_type;       
        if (directoryType == 4) {
          //doNothing
        } else if (directoryType == 8)

        {         
          strcpy(fileName,dirName);


          if (dirName[strlen(argv[1])-1] == '/') {
            strcat(fileName, dirPointer->d_name); 
            
            fd = fopen(fileName, "r");
            fdesc = open(fileName, O_RDONLY);
          } else {
            strcat(fileName, "/");          
            strcat(fileName, dirPointer->d_name);
          }
          strcpy(arrDir[fillingIndex], fileName);
          fillingIndex++;
        }
      }

    } while (dirPointer);
    qsort(arrDir,fillingIndex, sizeof arrDir[0],compareFunction);

    for (i = 0; i < numofThreads; i++) {
      if ((rc = pthread_create(&thr[i], NULL, calculateCRC , NULL)))
      {
        fprintf(stderr, "error:pthread creat, rc : %d\n",rc);
        return EXIT_FAILURE;
      }
    }


    pthread_mutex_lock(&lock);
    if (arrayAccessIndex < fillingIndex) {
      pthread_cond_wait(&CV_workComplete, &lock);
    }   
    pthread_mutex_unlock(&lock);


  }

  return 0;
}
