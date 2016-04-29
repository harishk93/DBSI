# DBSI Project 2 - Part 1

## Introduction
The purpose of the project is to implement a file storage system much like the B-Tree but by doing away with the pointers using intel SSE isntruction set. The tree structure will contain the keys and the result of a search operation will be the index of the range to which the key belongs. The structure supports fanouts of 5,9 and 17. The structure is implemented in C and uses the random generator. 

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

### Insertion
We use a recursive function `insert_element` to do the insertion. This function is invoked from the main `insert` function for each element. The function `insert_element`, when the node is full, recurses up the tree till it finds a non-empty node. The key is inserted at this node and the count is increased for the lower levels as well. The count increase at the lower level corresponds to the delimiter which is added between two successive nodes. Since the delimiter is added only when the node is full and the call for recursion happens when the node is full, the success of the recursion is used as a signal to add the delimiter. We use the reference code provided by thre professor and merely extend it for probing using SSE. The insertion is parsed into the tree structure availalbe in tree.h. 


###Probing
```
uint32_t probe_index2(Tree* tree, int32_t probe_key)
```
We use Intel’s SSE instruction set to perform probing on every level. 
we load delimiters of a node in that level into SIMD register of 128 bits. This register is of type __m128i. The probe key is broadcast to all lanes of another SIMD register. We do a SIMD comparison using the `_mm_cmplt_epi32(x,y)` and convert the result to a bit mask using `_mm_movemask_epi18`  which is responsible for creating the bit mask from high bits of 16 bytes. The bitmask will be a 0/-1 value per lane.  Based on the bitmask value we locate the node in the next level to search for. Using `_mm_packs_epi32` we pack the 8x32 bit integers from the two registers into 8x16 bit integers. We repeat this for n levels. 

### Probing Hardcoded 
```
void probe_hardcoded(Tree* tree, __m128i k, uint32_t* result2, uint32_t start):
```
We load the root node of index explicitly into register variable using the following code snippet. 
```
   __m128i cmp_01_a = _mm_cmplt_epi32(lvl_0_a,k1);
   __m128i cmp_01_b = _mm_cmplt_epi32(lvl_0_b,k1);
```
To load and broadcast four keys from input into register variable we use the following code snippet. 
```
  __m128i k=_mm_load_si128((__m128i*)&probe[4*i]) in main.c for each set of 4 keys

  register __m128i k1=_mm_shuffle_epi32(k,_MM_SHUFFLE(0,0,0,0));
  register __m128i k2=_mm_shuffle_epi32(k,_MM_SHUFFLE(1,1,1,1));
  register __m128i k3=_mm_shuffle_epi32(k,_MM_SHUFFLE(2,2,2,2));
  register __m128i k4=_mm_shuffle_epi32(k,_MM_SHUFFLE(3,3,3,3));
```
Rest of the function dollows what probe_index2() does but in this case the entrie loop across three levels is unrolled. 
## More Information
The detailed version of this file is available in form of a pdf file in this folder. 

## License
This is a project undertaken by `Harish Karthikeyan (HK2854)` and `Thiyagesh Viswanathan (TV2219)` as a course requirement for `COMSW 4112: Database System Implementation`. This is for Fall 2016. 


