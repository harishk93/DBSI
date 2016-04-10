#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "p2random.h"

void insert(size_t a[], int num_levels, int fanout[], int* A[])
{
	printf("called");
	size_t size = 1;
	size_t j,i;
    for(j=0;j<num_levels;j++)
     {
        size = size*fanout[j];
        for(i=0;i<size;i++)
 	    printf("&A[%d][%d] = %p\n",j,i,&A[j][i]);
		   
     }
}
	

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
		return EXIT_FAILURE;
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
			return EXIT_FAILURE;
		}
	}
	min_keys=max_keys/fanout[num_levels-1] +1;
	if (n<min_keys)
	{
		fprintf(stderr,"Error:Insufficient Key Count");
		return EXIT_FAILURE;
	}
	else if(n>max_keys)
	{
		fprintf(stderr,"Error:Key Count Exceeded");
		return EXIT_FAILURE;
	}
	int32_t *a = generate_sorted_unique(n, gen);
	free(gen);
	for(i=0;i<n;i++)
		printf("%d:%d\n",i,a[i]);
	int error;
	size_t size=1;
	
   int* A[num_levels];
   //printf("%d", sizeof(float));
   for(j=0;j<num_levels;j++)
    {
       size = size*fanout[j];
       if (posix_memalign((void **)&A[j], 16, size*sizeof(float)) != 0)
       printf("Cannot align");
	   for(i=0;i<size;i++)
	   printf("&A[%d][%d] = %p\n",j,i,&A[j][i]);
		   
    }
	insert(a,num_levels,fanout,A);

	return EXIT_SUCCESS;
}
