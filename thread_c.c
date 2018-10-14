#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h> 
#include <unistd.h>


#include <stdint.h>
#define buf_size 32


#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */



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

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

int X86SYSTEM;

void init_genrand(unsigned long s)
{
   mt[0]= s & 0xffffffffUL;
   for (mti=1; mti<N; mti++) {
      mt[mti] = 
	 (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
      /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
      /* In the previous versions, MSBs of the seed affect   */
      /* only MSBs of the array mt[].                        */
      /* 2002/01/09 modified by Makoto Matsumoto             */
      mt[mti] &= 0xffffffffUL;
      /* for >32 bit machines */
   }
}



int checkSystemType()
{
   unsigned int eax = 0x01;
   unsigned int ebx;
   unsigned int ecx;
   unsigned int edx;

   __asm__ __volatile__(
	 "cpuid;"
	 : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	 : "a"(eax)
	 );


   if (ecx & 0x40000000)
   {
      X86SYSTEM = 1;
   }
   else{
      X86SYSTEM = 0;
   }

   return X86SYSTEM;
}

unsigned long genrand_int32(void)
{
   unsigned long y;
   static unsigned long mag01[2]={0x0UL, MATRIX_A};
   /* mag01[x] = x * MATRIX_A  for x=0,1 */

   if (mti >= N) { /* generate N words at one time */
      int kk;

      if (mti == N+1)   /* if init_genrand() has not been called, */
	 init_genrand(5489UL); /* a default initial seed is used */

      for (kk=0;kk<N-M;kk++) {
	 y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
	 mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
      }
      for (;kk<N-1;kk++) {
	 y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
	 mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
      }
      y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
      mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

      mti = 0;
   }

   y = mt[mti++];

   /* Tempering */
   y ^= (y >> 11);
   y ^= (y << 7) & 0x9d2c5680UL;
   y ^= (y << 15) & 0xefc60000UL;
   y ^= (y >> 18);

   return y;
}

int genRandomNumber(int floor, int ceiling)
{
   int num = 0;

   if(X86SYSTEM == 0)
   {

      num =  (int)genrand_int32();//floor
   }
   else
   {

      __asm__ __volatile__("rdrand %0":"=r"(num));
   }

   num = abs(num % (ceiling - floor));
   if(num < floor)
   {
      return floor;
   }


   return num;
}




int rdrand16_step (uint16_t *rand)
{
   unsigned char ok;

   asm volatile ("rdrand %0; setc %1"
	 : "=r" (*rand), "=qm" (ok));

   return (int) ok;
}

int rdrand32_step (uint32_t *rand)
{
   unsigned char ok;

   __asm__ __volatile__ ("rdrand %0; setc %1"
	 : "=r" (*rand), "=qm" (ok));

   return (int) ok;
}

int rdrand64_step (uint64_t *rand)
{
   unsigned char ok;

   asm volatile ("rdrand %0; setc %1"
	 : "=r" (*rand), "=qm" (ok));

   return (int) ok;
}




int main()
{
	pthread_t ptid, ctid;


int r=32,c=2,i;

buffer = (int **)malloc(r * sizeof(int *)); 
for ( i=0; i<r; i++) 
         buffer[i] = (int *)malloc(c * sizeof(int)); 

	pthread_create(&ptid, NULL, Producer, NULL);
	pthread_create(&ctid, NULL, Consumer, NULL);

	pthread_join(ptid, NULL);
	pthread_join(ctid, NULL);

	return 0;

}


void *Producer()
{
int min,sec_i,i;
srand (time(NULL));
	for( i = 1; i <= counter; i++){
		pthread_mutex_lock(&mux);
		if(buf_index == -1){ 
			buf_index++;
		}
		if(buf_index == buf_size){ 
			pthread_cond_wait(&buf_not_full, &mux);
		} 
		
		buffer[buf_index][0] = i; /* Puting a value into the buffer to produce */
              //  sec_i= rand() % 7  + 2;
                sec_i=genRandomNumber(2, 9);
		buffer[buf_index][1] = sec_i; /* Puting a value into the buffer to produce */

		printf("Producer produce, Index %d : %d  \n", buf_index, buffer[buf_index][0]);
	buf_index++; 	
		pthread_mutex_unlock(&mux);
		pthread_cond_signal(&buf_not_empty); /* Signal Comsumer that buffer is not empty */

//min = rand() % 4  + 3;
min=genRandomNumber(3, 7);

sleep(min);
	}
	return NULL;

}

void *Consumer()
{
	int temp;
sleep(10);
int  j; 
	for( j = 1; j <= counter; j++){
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
	 int i;
        for( i=0; i<buf_index-1;i++){
        	buffer[i][0]=buffer[i+1][0];
            //buffer[i+1]=11111; 
        	//printf("buffer[%d]: %d\n",i,buffer[i]);
        }
       
 //sleep(temp);
           buf_index--;
                
		}
           
                
		pthread_mutex_unlock(&mux);
		pthread_cond_signal(&buf_not_full);  /* Signal Producer that buffer is not full */
	 sleep(temp);
	}
	return NULL;
}

