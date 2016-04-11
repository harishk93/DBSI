#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "p2random.h"
#include <limits.h>
#define MAXINT 2147483647
#define MININT -2147483648

int bs(int* A[],int start_index, int end_index, int32_t key, int level)
{
	if(start_index==end_index)
	{
		if (A[level][start_index] < key)
		{
			return (1+start_index);
		}
		else
		{ 
			return start_index;
		}
	}
	
	else
	{
		int mid = (start_index + end_index)/2;
		    if(A[level][mid] >= key)
				return bs(A,start_index,mid,key,level);
		    else
				return bs(A,mid+1,end_index,key,level);	
	}
}

int search(int * A[], int num_levels, int* fanout, int32_t key )
{
    int result = -1;
	int start_index = -1;
	int end_index = -1;
	result = bs(A,0,fanout[0]-2,key,0);
	for(int i=1;i<num_levels;i++)
	{
		start_index = result*fanout[i];
		end_index = start_index+fanout[i]-2;
		result = bs(A,start_index,end_index,key,i); 		
	}	
	return result; 
	
}
int insert_element(size_t elem, int level_index, int level_count[], int fanout[], int* A[])
{
	if((level_count[level_index]+1)%(fanout[level_index])==0 && level_count[level_index]>0) //Node is Full. Recurse up the array
	{
		if(insert_element(elem,level_index-1,level_count,fanout,A))
		{
			level_count[level_index]++;
		}
		return 1;
	}
	else
	{
		int* y=&A[level_index][level_count[level_index]-1]+0x000000001;
		*(y)=elem;
		level_count[level_index]++;
		return 1;
	}
}
void insert(int32_t a[], size_t n, int num_levels, int fanout[], int* A[])
{
	size_t size = 1;
	size_t j,i;
	int inserted_count=0;
	int level_count[num_levels];
	int num_leaf_filled[num_levels];
	for(j=0;j<num_levels;j++)
	{
		level_count[j]=0 ;
		num_leaf_filled[num_levels]=0;
	}
	while(inserted_count<n)
	{
		if(insert_element(a[inserted_count],num_levels-1,level_count,fanout,A))
			inserted_count++;
	}
}
	

int main(int argc, char **argv)
{
	int num_levels;
	rand32_t *gen = rand32_init(time(NULL));
	size_t i, n = argc > 1 ? atoll(argv[1]) : 10;
	size_t p = argc > 1 ? atoll(argv[2]) : 10;
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
	int32_t *P = generate(p,gen);
	free(gen);
	int error;
	size_t size=1;
	int* A[num_levels];
    for(j=0;j<num_levels;j++)
    {
		size = size*fanout[j];
		if (posix_memalign((void **)&A[j], 16, (size-1)*sizeof(float)) != 0)
			printf("Cannot align");
		for(i=0;i<size-1;i++)
			A[j][i]=MAXINT;
		   
    }
	size_t print_size=1;
	insert(a,n,num_levels,fanout,A);
	free(a);
	for(j=0;j<num_levels;j++)
    {
		printf("\n");
		int t=0;
		print_size = print_size*fanout[j];
		for(i=0;i<print_size-1;i++)
		{
			if(A[j][i]==MAXINT && A[j][i+1]==MAXINT)
			{
				if(t==fanout[j]-1)
				{
					printf("||\n");
					t=0;
					if(j==num_levels-1)
					{
						A[j][i]=-1;
					}
				}
				else
				{
					printf("A[%d][%d]=MAXINT\n",j,i);
					t++;
				}
			}
			else if(A[j][i]==MAXINT && A[j][i+1]!=MAXINT)
			{
				if(i+1==print_size-1)
					printf("A[%d][%d]=MAXINT\n",j,i);
				printf("||\n");
				if(j==num_levels-1)
					{
						A[j][i]=-1;
					}
				t=0;
			}
			else
			{
				printf("A[%d][%d]=%d\n",j,i,A[j][i]);
				t++;
			}
		}
    }
	printf("Page ID \t Range\n");
	printf("0 \t (MININT,%d]\n",A[num_levels-1][0]);
	for(i=1;i<print_size-1;i++)
	{
		if(A[num_levels-1][i]==-1)
		{
			if(A[num_levels-1][i+1]==MAXINT)
			{
				printf("%d \t (%d,MAXINT]\n",i,A[num_levels-1][i-1],A[num_levels-1][i+1]);
				break;
			}
			printf("%d \t (%d,%d]\n",i,A[num_levels-1][i-1],A[num_levels-1][i+1]);
		}
		else if(A[num_levels-1][i-1]==-1)
		{
			printf("%d \t (%d,%d]\n",i,A[num_levels-1][i-2],A[num_levels-1][i]);
		}
		else if(A[num_levels-1][i]==MAXINT)
		{
			printf("%d \t (%d,MAXINT]\n",i,A[num_levels-1][i-1]);
			break;
		}
		else
		{
			printf("%d \t (%d,%d]\n",i,A[num_levels-1][i-1],A[num_levels-1][i]);
		}
		
	}
	printf("\nProbes: \t Page Location\n");
	for(i=0;i<p;i++)
		{ 
			printf("%d:%d \t %d\n",i,P[i],search(A,num_levels,fanout,P[i]));
		}	
	return EXIT_SUCCESS;
}
