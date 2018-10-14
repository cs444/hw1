#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h> 
#define buf_size 32
#include <unistd.h>

void *Producer();
void *Consumer();

int buf_index = 0;
int buf_c_index = 0;
int counter = 300;
int **buffer;
/*  Declaring thread signals and mutex */
pthread_cond_t buf_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t buf_not_full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

int main()
{
	pthread_t ptid, ctid;

//	buffer = (char *) malloc(sizeof(char) * buf_size); 
int r=32,c=2;

buffer = (int **)malloc(r * sizeof(int *)); 
for (int i=0; i<r; i++) 
         buffer[i] = (int *)malloc(c * sizeof(int)); 

	pthread_create(&ptid, NULL, Producer, NULL);
	pthread_create(&ctid, NULL, Consumer, NULL);

	pthread_join(ptid, NULL);
	pthread_join(ctid, NULL);

	return 0;

}


void *Producer()
{
int min,sec_i;
srand (time(NULL));
	for(int i = 1; i <= counter; i++){
		pthread_mutex_lock(&mux);
		if(buf_index == -1){ /* Creating constraints on the index so it doesnt go out of bound */
			buf_index++;
		}
		if(buf_index == buf_size){ /* Check if buffer is full, if so, wait for signal not full */
			pthread_cond_wait(&buf_not_full, &mux);
		} 
		//if(buf_index == -1){
		//	buf_index++;
		//}
		buffer[buf_index][0] = i; /* Puting a value into the buffer to produce */
                sec_i= rand() % 7  + 2;
		buffer[buf_index][1] = sec_i; /* Puting a value into the buffer to produce */

		printf("Producer produce, Index %d : %d  \n", buf_index, buffer[buf_index][0]);
	buf_index++; 	
		pthread_mutex_unlock(&mux);
		pthread_cond_signal(&buf_not_empty); /* Signal Comsumer that buffer is not empty */
min = rand() % 4  + 3;
sleep(min);
	}
	return NULL;

}

void *Consumer()
{
	int temp;
sleep(10);
	for(int j = 1; j <= counter; j++){
		pthread_mutex_lock(&mux);
		buf_index--; /* Decrement to get adjusted index */
                
		if(buf_index == -1){
			pthread_cond_wait(&buf_not_empty, &mux);
		}
		else if(buf_index == buf_size){ /*  Constraining the index */
			buf_index--;
		}
        else{
        buf_index++;
         printf("Consumer consume, Index: %d : %d  \n", buf_index, buffer[0][0]);
         temp=buffer[0][1];
        for(int i=0; i<buf_index-1;i++){
        	buffer[i][0]=buffer[i+1][0];
            //buffer[i+1]=11111; 
        	//printf("buffer[%d]: %d\n",i,buffer[i]);
        }
       
 //sleep(temp);
           buf_index--;
                
		}
            //    sleep(buffer[buf_index][1]);
                
		pthread_mutex_unlock(&mux);
		pthread_cond_signal(&buf_not_full);  /* Signal Producer that buffer is not full */
	 sleep(temp);
	}
	return NULL;
}

