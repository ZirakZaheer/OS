#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "crc32.c"
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>

int directoryType = 0;
char* dirName = NULL;
char arrDir[100][100];
char returnArr1[100][100];
char fileName[1000]  = "";

struct stat filestat;
int fileSize =0;

 FILE* fd;
 int fdesc = 0;
 int arrayAccessIndex = 0;


static int compareFunction(const void *p, const void *q)
{
    const char *l= p;
    const char *r= q;
    int cmp;

    cmp= strcmp (l, r);
    return cmp;
}


int main(int argc, char *argv[])
{
  DIR *dirport;
  struct dirent* dirPointer;
  int i = 0;

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
            fd = fopen(fileName, "r"); 
            fdesc = open(fileName, O_RDONLY);
          }
          if (!fd || fdesc == -1) {
            //printf("%s\n", "ACCESS ERROR");
          } else {
            fclose(fd);            
          }
          strcpy(arrDir[i], fileName);
          i++;
        }
      }

    } while (dirPointer);

    // crc calc
     qsort(arrDir,i, sizeof arrDir[0],compareFunction);
     while (arrayAccessIndex < i) {
      int fdesc1 = 0;
      char filedata1[1000000];
      uint32_t crc = 0;
      
      crc = 0;

      stat((arrDir[arrayAccessIndex]), &filestat);
      fileSize = filestat.st_size;
      fdesc1 = open((arrDir[arrayAccessIndex]), O_RDONLY);
       printf(" %s  ", (arrDir[arrayAccessIndex]));
       if (fdesc1 == -1) {
            printf("%s\n", "ACCESS ERROR");
          } else {
            size_t sizefile = read(fdesc1, filedata1, fileSize);

            crc =  crc32(0, filedata1, sizefile);

           
            //printf(" %s  ", (arrDir[arrayAccessIndex]));
            printf("%X\n", crc);
            //int crc = 0;
            
            close(fdesc1);            
          }     
      arrayAccessIndex++;     
     }
  }
  return 0;
}
