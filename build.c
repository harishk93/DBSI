#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "p2random.h"

int main(int argc, char **argv)
{
	int num_levels;
	rand32_t *gen = rand32_init(time(NULL));
	size_t i, n = argc > 1 ? atoll(argv[1]) : 10;
	if(argc>1)
		num_levels=argc-3;
	else
	{
		fprintf(stderr,"Error:Insufficient Parameters");
		return;
	}
	size_t j;
	int fanout[num_levels];
	void* levels;
	int temp;
	size_t max_keys=1, min_keys;
	for(j=0;j<num_levels;j++)
	{
		temp=atoi(argv[3+j]);
		max_keys=max_keys*(temp-1);
		if(temp>=2&&temp<=17)
			fanout[j]=temp-1;
		else
		{
			fprintf(stderr,"Error:Fanout Limit Exceeded");	
			return;
		}
	}
	min_keys=max_keys/fanout[num_levels-1] +1;
	if (n<min_keys)
	{
		fprintf(stderr,"Error:Insufficient Key Count");
		return;
	}
	else if(n>max_keys)
	{
		fprintf(stderr,"Error:Key Count Exceeded");
		return;
	}
	int32_t *a = generate_sorted_unique(n, gen);
	free(gen);
	for(i=0;i<n;i++)
		printf("%d:%d\n",i,a[i]);
	int error;
	size_t size=fanout[0];
	for(j=0;j<num_levels;j++)
	{
		error=posix_memalign(&levels,16,20*sizeof(float));
	}
	
	for(j=0;j<20;j++)
		printf("&levels[0][%d] = %p\n",j,&levels[j]);
	// float *A = (float*) memalign(16,20*sizeof(float));

   //align
   int* A;
   printf("%d", sizeof(float));
   
   if (posix_memalign((void **)&A, 16, 20*sizeof(float)) != 0)
    printf("Cannot align");
*A=a[0];
    for(i = 0; i < 20; i++)
       printf("&A[%d] = %p\n",i,&A[i]);

        free(A);
printf("%d",*A);

	/*int32_t *a = generate_sorted_unique(n, gen);
	free(gen);
	for (i = 1 ; i < n ; ++i)
	{
		assert(a[i - 1] < a[i]);
	}
	for(i=0;i<n;i++)
		printf("%d:%d\n",i,a[i]);
	//ratio_per_bit(a, n);
	free(a);*/
	return EXIT_SUCCESS;
}