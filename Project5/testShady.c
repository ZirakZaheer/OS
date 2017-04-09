#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>


int main(int argc, char *argv[]) {
	int fdesc = open("oldsleep.c", O_RDONLY);
	return 0;
}
