#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "p2random.h"
int insert_element(size_t elem, int level_index, int* level_count, int fanout[], int* A[])
{
	if(level_count[level_index]%(fanout[level_index]-1)==0 && level_count[level_index]>0) //Node is Full. Recurse up the array
	{
		if(insert_element(elem,level_index-1,level_count,fanout,A))
		{
			level_count[level_index]++;
		}
		return 1;
	}
	else
	{
	//	printf("Level_Index=%d\nLevel_count=%d\n",level_index,level_count[level_index]);
	//	int count=level_count[level_index];
	//	printf("%p",&A[level_index][level_count[level_index]]);
		A[level_index][level_count[level_index]]=elem;
	//	printf("%d",A[level_index][level_count[level_index]]);
		level_count[level_index]++;
		return 1;
	}
}
void insert(int32_t a[], size_t n, int num_levels, int fanout[], int* A[])
{
	printf("Called_Insert\n");
	size_t size = 1;
	size_t j,i;
	int inserted_count=0;
	int level_count[num_levels];
	int num_leaf_filled[num_levels];
	for(j=0;j<num_levels;j++)
	{
		level_count[j]=0;
		num_leaf_filled[num_levels]=0;
	}
	//insert_element(a[0],1,level_count,fanout,A);
	//printf("%d",A[1][0]);
	/*while(inserted_count<n)
	{
		printf("Inserting Element: %d\n",a[inserted_count]);
		/*if(level_count[num_levels-1]%(fanout[num_levels]-1)==0 && level_count[num_levels-1]>0)
		{
			if(insert_element(a[inserted_count],num_levels-2,level_count,fanout,A))
			{
				level_count[num_levels-1]++;
				inserted_count++;
			}
		}		
		else
		{
			for(i=0;i<fanout[num_levels]-1;i++)
			{
				level_count[num_levels-1]=i;
				A[num_levels-1][level_count[num_levels-1]]=a[inserted_count];
				inserted_count++;
			}
		}*/
		//inserted_count++;
	//}	
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
	int num_keys[num_levels];
	//void* levels;
	int temp;
	size_t max_keys=1, min_keys=1;
	for(j=0;j<num_levels;j++)
	{
		temp=atoi(argv[3+j]);
		if(temp>=2&&temp<=17)
			fanout[j]=temp;
		else
		{
			fprintf(stderr,"Error:Fanout Limit Exceeded");	
			return EXIT_FAILURE;
		}
	}
	for(j=0;j<num_levels;j++)
	{
		max_keys=max_keys*fanout[j];
		if(j>0)
			min_keys=min_keys*fanout[j];
	}

	max_keys=max_keys-1;
	//printf("Max:%d\nMin:%d\n",max_keys,min_keys);
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
	//for(i=0;i<n;i++)
		//printf("%d:%d\n",i,a[i]);
	int error;
	size_t size=1;
	int* A[num_levels];
    for(j=0;j<num_levels;j++)
    {
       size = size*fanout[j];
       if (posix_memalign((void **)&A[j], 16, (size-1)*sizeof(float)) != 0)
       printf("Cannot align");
	   //for(i=0;i<size-1;i++)
	 //  printf("&A[%d][%d] = %p\n",j,i,&A[j][i]);
		   
    }
	size_t print_size=1;
	insert(a,n,num_levels,fanout,A);
	/*for(j=0;j<num_levels;j++)
    {
       print_size = print_size*fanout[j];
       for(i=0;i<print_size-1;i++)
		printf("A[%d][%d] = %d\n",j,i,A[j][i]);
		   
    }*/
	return EXIT_SUCCESS;
}
