# DBSI Project 2 - Part 1

## Introduction
The purpose of the project is to implement a file storage system much like the B-Tree but by doing away with the pointers. The tree structure will contain the keys and the result of a search operation will be the index of the range to which the key belongs. The structure supports multiple fanouts between values of 2 and 17. The structure is implemented in C and uses the random generator. 

## Execution

The project comes with a makefile called "Makefile". This is invoked by the command 
```
make Makefile
```
The Makefile automatically compiles the program to generate the executable file "build". A `GCC` compiler is used and needed for this program to run. 

The program is run through 
```
./build K P fanouts
```
where K is the number of keys that need to be inserted in the structure, P is the number of probes, fanouts are the space separated fanout values for each level starting from the root. Note that in a node with fanout f, there can be f-1 keys. 

## Components

### Random Function and Generating Sample Input
A `p2random.h` is used to generate the random input. The functions `generate_sorted_unique` takes the number of random numbers `n` as a parameter to generate an array containing `n` random 32-bit integers, each unique and the array is sorted. The function `generate` also takes `n` as parameter to generate `n` 32-bit integer values. We use the former to generate the keys and the latter ot generate the probes. 

### Key Count Validation

```C
	for(j=0;j<num_levels;j++)
	{
		max_keys=max_keys*fanout[j];
		if(j>0)
			min_keys=min_keys*fanout[j];
	}
	max_keys=max_keys-1;
```
This loop is used to generate the maximum number of keys and the minimum number of keys that a given tree can hold. The key idea is that for the minimum number of keys you would need the tree at level 1 to have maximum number of keys and then you need a extra key to fill up the root at level 0. This is the purpose of the if statement. 

### Memory Allocation

We use `posix_memalign` function to orient the values at 16-byte addresses. The memory is stored in the array `A`. This is a two dimensional array corresponding to the array of keys at each level. 
```C
	int* A[num_levels];
    for(j=0;j<num_levels;j++)
    {
		size = size*fanout[j];
		if (posix_memalign((void **)&A[j], 16, (size-1)*sizeof(float)) != 0)
			printf("Cannot align");
		for(i=0;i<size-1;i++)
			A[j][i]=MAXINT;		   
    }
```
This block does the memory assignment and initializes to `MAXINT`. 

### Insertion
We use a recursive function `insert_element` to do the insertion. This function is invoked from the main `insert` function for each element. The function `insert_element`, when the node is full, recurses up the tree till it finds a non-empty node. The key is inserted at this node and the count is increased for the lower levels as well. The count increase at the lower level corresponds to the delimiter which is added between two successive nodes. Since the delimiter is added only when the node is full and the call for recursion happens when the node is full, the success of the recursion is used as a signal to add the delimiter.  

###Probing
We perform Binary Search on a node. The Binary Search returns a value from `0+start_index,1+Start_index,...,fanout-1+start_index` corresponding to the fanout further nodes available to search. The algorithm assumes that we return the left of the current key if `search_key <= current_key`. This Binary Search function `bs` is invoked from the function search. The search function searches for the key across all the levels. It uses the result from the previous level to calculate the `start_index` and `end_index` for the current level. 
```C
	start_index = result*fanout[i];
	end_index = start_index+fanout[i]-2;
	result = bs(A,start_index,end_index,key,i); 
```
The result in the first line is the result of the `bs` call in the previous level. Note that we multiply by fanout value as each node will have fanout-1 keys but will have a extra location for the delimiter. So each of the index from previous level will correspond to a node with fanout entries. The new start_index will take that value. This is called repeatedly and the function returns the final result. 

## More Information
The marked down version of this readme file is available at https://github.com/harishk93/DBSI. 

## License
This is a project undertaken by `Harish Karthikeyan (HK2854)` and `Thiyagesh Viswanathan (TV2219)` as a course requirement for `COMSW 4112: Database System Implementation`. This is for Fall 2016. 

