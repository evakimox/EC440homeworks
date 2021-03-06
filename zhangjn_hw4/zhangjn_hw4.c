#define N 12000000
#define N_MOMENTS 50

// seed to use for random number generated in main
#define SEED 123
#define XMIN 0.0
#define XLIMIT 2.0
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#define NUMTHRDS 3  

typedef struct
{
  double      a[N/NUMTHRDS];
  double    globalsum[N_MOMENTS];
  int     veclen;
} DOTDATA;

/* Define globally accessible variables and a mutex */
DOTDATA dotstr;
pthread_t callThd[NUMTHRDS];
pthread_mutex_t mutexsum;

double x[N]; // the main data array
double moments[N_MOMENTS]; // array for moment sums and for the final answers
//spilted x array and the small moments


void generate_data(double *destination, int n, double xmin, double xlimit, int seed){
  // fills the destination with random numbers x, xmin<=x<xlimit using random seed
  int i;
  double scale;

  srandom(seed);   // set the random number seed so that we can reproduce same data
  scale=(xlimit-xmin)/RAND_MAX; // scale for random number

  for (i=0;i<n;++i){
    destination[i]= xmin+scale*random();
  }
}

void sum_moments(double *data, int n, double *result, int m){
  // calculate m moment sums for n element array data and place sums in result
  double term;
  int i,j;

  for (j=0;j<m;++j) result[j]=0; // initialize result

  for (i=0;i<n;++i){ // main loop over data points
    for (j=0, term=x[i];    // loop over moments, start with term as x[i]
	 j<m;               // continue for m steps
	 ++j,term*=x[i]){   // increment count and increase the power

      result[j]+= term;   // accumulate result for power j+1
    }
  }
}

// some helper functions for dealing with times

double time_in_seconds(struct timespec *t){
  // a timespec has integer values for seconds and nano seconds
  return (t->tv_sec + 1.0e-9 * (t->tv_nsec));
}

void *thrdcalldis(void *arg){
 int i, start,end,len;
 long offset;
 double *mysum;//[N_MOMENTS];
 double *anotherx;//[N/NUMTHRDS];
 offset=(long) arg;
 len=dotstr.veclen;
 start=offset*len;
 end=start+len;
 anotherx=(double*)malloc(N/NUMTHRDS*sizeof(double));
 mysum = (double*)malloc(N_MOMENTS*sizeof(double));
 for(int iter=0;iter<N/NUMTHRDS;iter++){
 anotherx[iter]=dotstr.a[iter];
 }
 sum_moments(anotherx,(N/NUMTHRDS-1),mysum,N_MOMENTS);  //what it does is basically call another

 pthread_mutex_lock(&mutexsum);
 for(int it=0;it<N_MOMENTS;it++){
 dotstr.globalsum[it]+= mysum[it];
 }
 //printf("Thread %ld did one sum = %f global sum is %f \n",offset,mysum,dotstr.globalsum);
 pthread_mutex_unlock(&mutexsum);
 pthread_exit((void*)0);
}
   



int main(){
  struct timespec start, finish, resolution, calculation;  // timing info
  double calculation_time, start_time, finish_time;
  int err; // error number for system calls

  long i;

  time_t startsec;
  err=clock_getres(CLOCK_THREAD_CPUTIME_ID,&resolution);
  if (err){
    perror("Failed to get clock resolution");
    exit(1);
  }
  printf("Main: thread clock resolution = %-16.9g seconds\n",time_in_seconds(&resolution));


  generate_data(x,N,XMIN,XLIMIT,SEED); // generate data
  printf("Main: generated %d data values in the range %5.3f to %5.3f, seed=%d\n",
	 N, XMIN, XLIMIT, SEED);
   
  err=clock_gettime(CLOCK_THREAD_CPUTIME_ID,&start);
  if (err){
    perror("Failed to read thread_clock with error = %d\n");
    exit(1);
  }
  startsec = time(NULL);
  printf("Main: calculation start time = %-20.9g seconds\n",time_in_seconds(&start));

  /*Here start the threads:*/
  //long i;
  double *a;
  void *status;
  pthread_attr_t attr;
  time_t endsec;
  a=(double*)malloc(N*sizeof(double));
  for(int iterator=0;iterator<N;iterator++){
    dotstr.a[iterator]=x[iterator];
  }     

  dotstr.veclen=N;
  for(int iterator=0;iterator<N_MOMENTS;iterator++){
  dotstr.globalsum[iterator]=0;
  }
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
  for(i=0;i<NUMTHRDS;i++){
    pthread_create(&callThd[i],&attr,thrdcalldis,(void*)i);
  }
   pthread_attr_destroy(&attr);
  for(i=0;i<NUMTHRDS;i++){
    pthread_join(callThd[i],&status);
   }
  for(i=0;i<N_MOMENTS;++i){
    moments[i]=dotstr.globalsum[i];
  }

  for (i=0;i<N_MOMENTS;++i) moments[i] /= N;  // convert sums to averages
  free(a);
  endsec = time(NULL);
  err=clock_gettime(CLOCK_THREAD_CPUTIME_ID,&finish);
  if (err){
    perror("Failed to read thread_clock with error = %d\n");
    exit(1);
  }
  printf("\n\n Number of threads used %d \n\nMain: calculation finish time = %-20.9g seconds\n",NUMTHRDS,time_in_seconds(&start)+endsec-startsec);

  calculation_time = time_in_seconds(&finish)-time_in_seconds(&start);
  printf("Calculation elapsed time = %ld seconds\n",endsec-startsec);

  printf("\n==================== MOMENT RESULTS ======================\n");
  printf("%5s %30s %30s\n","m","moment","Expected");
  printf("%5s %30s %30s\n","-","------","--------");
  for(i=0;i<N_MOMENTS;++i){
    printf("%5d %30.16e %30.16e\n",i+1,moments[i],
	   0.5*(pow(XLIMIT,i+2)-pow(XMIN,i+2))/(i+2));
  }
  //  printf("Calculation time = %20.9g seconds\n ",calculation_time);
  printf("\n\n Number of threads used %d \n\nMain: time elapsed = %ld seconds\n",NUMTHRDS,endsec-startsec);
  
  pthread_mutex_destroy(&mutexsum);
  pthread_exit(NULL);

  
}
  
