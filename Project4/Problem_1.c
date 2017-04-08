#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "assert.h"
#include "time.h"

//global counts and vars

int n_cats = 0;
int n_dogs = 0;
int n_birds = 0;

volatile int playingRecord[3] = {0};

clock_t start;
#define MAX 99;
time_t gstart_t, gend_t;
double gdiff_t;
int gtimeProvided = 0;

//monitor stuff

volatile int catsinground = 0;
volatile int dogsinground = 0;
volatile int birdsinground = 0 ;

pthread_mutex_t lock;	// = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CV_birdsPlaying;	// = PTHREAD_COND_INITIALIZER;
pthread_cond_t CV_dogsPlaying;	// = PTHREAD_COND_INITIALIZER;
pthread_cond_t CV_catsPlaying;

void cat_exit();
void dog_exit();
void bird_exit();
void play();


void* cat_enter(void *arg) {
	time_t start_t, end_t;
  	time(&start_t);
  	int timeProvided = 10;
  	double diff_t;

	while (1) {
		int data = *(int*) arg;

		pthread_mutex_lock(&lock);


		while (dogsinground > 0) {
				//printf("%s\n", "inside here1");
				pthread_cond_wait(&CV_dogsPlaying, &lock);
		}
		while (birdsinground > 0) {
			//printf("%s\n", "inside here2");
			//printf("catsinground %d dogsinground %d birdsinground %d\n", catsinground, dogsinground, birdsinground);

				pthread_cond_wait(&CV_birdsPlaying, &lock);
		}
		//printf("catsinground %d dogsinground %d\n", catsinground, dogsinground);
		playingRecord[data]++;
		
		play();
		catsinground++;
		pthread_mutex_unlock(&lock);	

		time(&end_t);
		diff_t = difftime(end_t, start_t);
				if (diff_t > timeProvided) {
				            printf("Provided time = %d\n", timeProvided);
				            printf("Execution time = %f\n", diff_t);
				             cat_exit();
				            break;
		}

		cat_exit();
	
	}
	//pthread_mutex_unlock(&lock);	
	
	pthread_exit(NULL);
}

void* bird_enter(void *arg) {
	time_t start_t, end_t;
  	time(&start_t);
  	int timeProvided = 10;
  	double diff_t;
  	//printf("birds are entering");
	

	while (1) {
	int data = *(int*) arg;

	pthread_mutex_lock(&lock);

	while (catsinground > 0) {
		pthread_cond_wait(&CV_catsPlaying, &lock);
	}
	play();
	birdsinground++;
	playingRecord[data]++;
	
	pthread_mutex_unlock(&lock);
	

	time(&end_t);
	diff_t = difftime(end_t, start_t);
	if (diff_t > timeProvided) {
		printf("Provided time = %d\n", timeProvided);
		printf("Execution time = %f\n", diff_t);
		//   printf("Exiting of the program...\n");
		//pthread_mutex_unlock(&lock);
	   	bird_exit();
		break;

		}
	
    bird_exit();
	}	
	//pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
}

void* dog_enter(void *arg) {

	time_t start_t, end_t;
  	time(&start_t);
  	int timeProvided = 10;
  	double diff_t;

	//printf("dogs are entering" );

	int data = *(int*) arg;
	while (1) {

		pthread_mutex_lock(&lock);

		while (catsinground > 0) {
			pthread_cond_wait(&CV_catsPlaying, &lock);
		}
		play();
		playingRecord[data]++;
		dogsinground++;
		
		pthread_mutex_unlock(&lock);
		


		time(&end_t);

		diff_t = difftime(end_t, start_t);

		if (diff_t > timeProvided) {
			  printf("Provided time = %d\n", timeProvided);
			  printf("Execution time = %f\n", diff_t);
			  printf("Exiting of the program...\n");
			  //pthread_mutex_unlock(&lock);
			  dog_exit();
			  break;
		}
			
	dog_exit();

	}
	//pthread_mutex_unlock(&lock);
	//pthread_mutex_unlock(&lock);	
	pthread_exit(NULL);
}

void cat_exit(void) {
	pthread_mutex_lock(&lock);
	if (catsinground > 0)
		catsinground--;
	
	if (catsinground == 0){
		pthread_mutex_unlock(&lock);
		pthread_cond_broadcast(&CV_catsPlaying);
	} else {
		pthread_mutex_unlock(&lock);
	}
}

void dog_exit(void) {
	pthread_mutex_lock(&lock);
	
	if (dogsinground > 0)
		dogsinground--;

	if (dogsinground == 0) {
		pthread_mutex_unlock(&lock);
		pthread_cond_broadcast(&CV_dogsPlaying);
	} else {
		pthread_mutex_unlock(&lock);
	}

}

void bird_exit(void) {
	pthread_mutex_lock(&lock);
	if (birdsinground > 0)
		birdsinground--;
	if (birdsinground == 0){
		pthread_mutex_unlock(&lock);
		pthread_cond_broadcast(&CV_birdsPlaying);
	} else {
		pthread_mutex_unlock(&lock);
	}
}

void play(void) {
	for (int i = 0; i < 10;i++) {
	    assert(catsinground >= 0 && catsinground <= n_cats);
	    assert(dogsinground >= 0 && dogsinground <= n_dogs);
	    assert(birdsinground >= 0 && birdsinground <= n_birds);
	    assert(catsinground == 0 || dogsinground == 0);
	    assert(catsinground == 0 || birdsinground == 0);		
   	}
}

int main(int argc, char const *argv[]) {

	if (argc <3) {
		printf("%s\n", "provide proper number of arguments");
		exit(0);
	}
	n_cats = strtol(argv[1], NULL, 10);
	n_dogs =  strtol(argv[2], NULL, 10);
	n_birds =  strtol(argv[3], NULL, 10);
	//printf("%d\n", n_cats);

	pthread_t catThr[n_cats];
	pthread_t dogThr[n_dogs];
	pthread_t birdThr[n_birds];


	int catThreadData[2] = {0};
	int dogThreadData[2] = {1};
	int birdThreadData[2] = {2};

	time(&gstart_t);
	int i,rc;

	for (i = 0; i < n_cats; i++) {
		if ((rc = pthread_create(&catThr[i], NULL, cat_enter , &catThreadData[0])))
		{
			fprintf(stderr, "error:pthread creat, rc : %d\n",rc);
            return EXIT_FAILURE;

		}
	}

	for (i = 0; i < n_dogs; i++) {
		if ((rc = pthread_create(&dogThr[i], NULL, dog_enter , &dogThreadData[0])))
		{
			fprintf(stderr, "error:pthread creat, rc : %d\n",rc);
            return EXIT_FAILURE;

		}
	}

	for (i = 0; i < n_birds; i++) {
		if ((rc = pthread_create(&birdThr[i], NULL, bird_enter , &birdThreadData[0])))
		{
			fprintf(stderr, "error:pthread creat, rc : %d\n",rc);
            return EXIT_FAILURE;

		}
	}

	for (i = 0; i < n_cats; ++i) {
        pthread_join(catThr[i], NULL);
    }

    for (i = 0; i < n_dogs; ++i) {
        pthread_join(dogThr[i], NULL);
    }

    
    for (i = 0; i < n_birds; ++i) {
        pthread_join(birdThr[i], NULL);
    }

    for (i = 0; i < 1; ++i) {
    	printf("Cats %d Entered CS : %d\n",i , playingRecord[0]);
    	printf("dogs %d Entered CS : %d\n",i , playingRecord[1]);
    	printf("birds %d Entered CS : %d\n",i , playingRecord[2]);
    }

	return 0;
}