#include "tree.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h> 
#include <smmintrin.h>
#include <nmmintrin.h>
#include <ammintrin.h> 
#include <x86intrin.h>

extern int posix_memalign(void** memptr, size_t alignment, size_t size);
size_t alignment = 16;

void print128_num(__m128i var)
{
    uint32_t *val = (uint32_t*) &var;
    printf("Numerical: %i %i %i %i \n", 
           val[0], val[1], val[2], val[3]);
}

Tree* build_index(size_t num_levels, size_t fanout[], size_t num_keys, int32_t key[]) {
        // return null pointer for invalid tree configuration
        size_t min_num_keys = 1;
        for (size_t i = 0; i < num_levels - 1; ++i) {
                min_num_keys *= fanout[i];
        }
        size_t max_num_keys = min_num_keys * fanout[num_levels - 1] - 1;
        if (num_keys < min_num_keys || num_keys > max_num_keys) {
                fprintf(stderr, "Error: incorrect number of keys, min %zu, max %zu\n", min_num_keys, max_num_keys);
                return NULL;
        }

        // initialize the tree index
        Tree* tree = malloc(sizeof(Tree));
        assert(tree != NULL);
        tree->num_levels = num_levels;
        tree->node_capacity = malloc(sizeof(size_t) * num_levels);
        assert(tree->node_capacity != NULL);
        for (size_t i = 0; i < num_levels; ++i) {
                tree->node_capacity[i] = fanout[i] - 1;
        }
        tree->key_array = malloc(sizeof(int32_t*) * num_levels);
        assert(tree->key_array != NULL);
        size_t* key_count = malloc(sizeof(size_t) * num_levels);
        assert(key_count != NULL);
        size_t* array_capacity = malloc(sizeof(size_t) * num_levels);
        assert(array_capacity != NULL);
        for (size_t i = 0; i < num_levels; ++i) {
                size_t size = sizeof(int32_t) * tree->node_capacity[i];         // allocate one node per level
                int error = posix_memalign((void**) &(tree->key_array[i]), alignment, size);
                assert(error == 0);
                key_count[i] = 0;
                array_capacity[i] = tree->node_capacity[i];     // array_capacity[i] is always a multiple of node_capacity[i]
        }

        // insert sorted keys into index
        for (size_t i = 1; i < num_keys; ++i) {
                assert(key[i - 1] < key[i]);
        }
        for (size_t i = 0; i < num_keys; ++i) {
                size_t level = num_levels - 1;
                while (key_count[level] == array_capacity[level])
                        level -= 1;
                tree->key_array[level][key_count[level]] = key[i];
                key_count[level] += 1;
                while (level < num_levels - 1) {
                        level += 1;
                        size_t new_capacity = array_capacity[level] + tree->node_capacity[level];
                        size_t size = sizeof(int32_t) * new_capacity;           // allocate one more node
                        int32_t* new_array = NULL;
                        int error = posix_memalign((void**) &new_array, alignment, size);
                        assert(error == 0);
                        memcpy(new_array, tree->key_array[level], sizeof(int32_t) * key_count[level]);
                        free(tree->key_array[level]);
                        tree->key_array[level] = new_array;
                        array_capacity[level] = new_capacity;
                }
        }

        // pad with INT32_MAXs
        for (size_t i = 0; i < num_levels; ++i) {
                for (size_t j = key_count[i]; j < array_capacity[i]; ++j)
                        tree->key_array[i][j] = INT32_MAX;
                key_count[i] = array_capacity[i];
        }

        // print the tree
        // for (size_t i = 0; i < num_levels; ++i) {
        //         printf("Level %zu:", i);
        //         for (size_t j = 0; j < key_count[i]; ++j)
        //                 printf(" %d", tree->key_array[i][j]);
        //         printf("\n");
        // }

        free(array_capacity);
        free(key_count);
		return tree;
}

uint32_t probe_index(Tree* tree, int32_t probe_key) {
        size_t result = 0;
        for (size_t level = 0; level < tree->num_levels; ++level) {
                size_t offset = result * tree->node_capacity[level];
                size_t low = 0;
                size_t high = tree->node_capacity[level];
                while (low != high) {
                        size_t mid = (low + high) / 2;
                        if (tree->key_array[level][mid + offset] < probe_key)
                                low = mid + 1;
                        else
                                high = mid;
                }
                size_t k = low;       // should go to child k
                result = result * (tree->node_capacity[level] + 1) + k;
        }
        return (uint32_t) result;
}

uint32_t probe_index_sse(Tree* tree, int32_t probe_key)
{
    size_t n = tree->num_levels;
	size_t i = 0; 
	int result=0, prev_result=0;
	__m128i key = _mm_cvtsi32_si128(probe_key);
	key = _mm_shuffle_epi32(key,_MM_SHUFFLE(0,0,0,0));
	for (i=0; i<n; i++)
	{
		prev_result=result;
		size_t capacity = tree->node_capacity[i];
		if(capacity==8)
		{
			__m128i lvl_2_a = _mm_load_si128(&(tree->key_array[i][prev_result << 3])); 
			__m128i lvl_2_b = _mm_load_si128(&(tree->key_array[i][(prev_result << 3) + 4])); 
			
			__m128i cmp_2_a = _mm_cmplt_epi32(lvl_2_a,key);
			__m128i cmp_2_b = _mm_cmplt_epi32(lvl_2_b,key);
			
			__m128i cmp_2 = _mm_packs_epi32(cmp_2_a,cmp_2_b);
			cmp_2 = _mm_packs_epi16(cmp_2,_mm_setzero_si128());
			
			result = _mm_movemask_epi8(cmp_2);
			result=_bit_scan_forward(result^ 0x1FFFF);
			result+=(prev_result<<3)+prev_result;
		}
		else if(capacity==4)
		{
			__m128i lvl_1 = _mm_load_si128(&(tree->key_array[i][prev_result << 2])); 
				
			__m128i cmp_1 = _mm_cmplt_epi32(lvl_1,key);
				
			result = _mm_movemask_ps((__m128)cmp_1);
			result=_bit_scan_forward(result^ 0x1FF);

			result+=(prev_result<<2)+prev_result;
		}
		else if(capacity==16)
		{
			__m128i lvl_2_a = _mm_load_si128(&(tree->key_array[i][prev_result << 4])); 
			__m128i lvl_2_b = _mm_load_si128(&(tree->key_array[i][(prev_result << 4)+4])); 
			__m128i lvl_2_c = _mm_load_si128(&(tree->key_array[i][(prev_result << 4)+8])); 
			__m128i lvl_2_d = _mm_load_si128(&(tree->key_array[i][(prev_result << 4)+12]));

			__m128i cmp_2_a = _mm_cmplt_epi32(lvl_2_a,key);
			__m128i cmp_2_b = _mm_cmplt_epi32(lvl_2_b,key);
			__m128i cmp_2_c = _mm_cmplt_epi32(lvl_2_c,key);
			__m128i cmp_2_d = _mm_cmplt_epi32(lvl_2_d,key);

			__m128i cmp_2_ab = _mm_packs_epi32(cmp_2_a,cmp_2_b);
			__m128i cmp_2_cd = _mm_packs_epi32(cmp_2_c,cmp_2_d);
			
			__m128i cmp_2 = _mm_packs_epi16(cmp_2_ab,cmp_2_cd);
			
			result = _mm_movemask_epi8(cmp_2);
				//printf("Result:%d\n",result);
			result=_bit_scan_forward(result ^ 0x1FFFFFF);
			result+=(prev_result<<4)+prev_result;
				//printf("r_17:%d\n",result);
		}
	}  
	return result;
}
void probe_hardcoded(Tree* tree, __m128i k, uint32_t* result2, __m128i lvl_0_a, __m128i lvl_0_b, uint32_t start)
{
	//lvl_0_a = _mm_load_si128(&(tree->key_array[0][0])); 
	//lvl_0_b = _mm_load_si128(&(tree->key_array[0][0])+4); 
        
	register __m128i k1=_mm_shuffle_epi32(k,_MM_SHUFFLE(0,0,0,0));
	register __m128i k2=_mm_shuffle_epi32(k,_MM_SHUFFLE(1,1,1,1));
	register __m128i k3=_mm_shuffle_epi32(k,_MM_SHUFFLE(2,2,2,2));
	register __m128i k4=_mm_shuffle_epi32(k,_MM_SHUFFLE(3,3,3,3));

	//Loading of Level 0 into Register - 9 Way
	__m128i cmp_01_a = _mm_cmplt_epi32(lvl_0_a,k1);
	__m128i cmp_01_b = _mm_cmplt_epi32(lvl_0_b,k1);
	
	__m128i cmp_02_a = _mm_cmplt_epi32(lvl_0_a,k2);
	__m128i cmp_02_b = _mm_cmplt_epi32(lvl_0_b,k2);
	
	__m128i cmp_03_a = _mm_cmplt_epi32(lvl_0_a,k3);
	__m128i cmp_03_b = _mm_cmplt_epi32(lvl_0_b,k3);
	
	__m128i cmp_04_a = _mm_cmplt_epi32(lvl_0_a,k4);
	__m128i cmp_04_b = _mm_cmplt_epi32(lvl_0_b,k4);
	
	__m128i cmp_01 = _mm_packs_epi32(cmp_01_a,cmp_01_b);
	cmp_01 = _mm_packs_epi16(cmp_01,_mm_setzero_si128());
	
	__m128i cmp_02 = _mm_packs_epi32(cmp_02_a,cmp_02_b);
	cmp_02 = _mm_packs_epi16(cmp_02,_mm_setzero_si128());
	
	__m128i cmp_03 = _mm_packs_epi32(cmp_03_a,cmp_03_b);
	cmp_03 = _mm_packs_epi16(cmp_03,_mm_setzero_si128());
	
	__m128i cmp_04 = _mm_packs_epi32(cmp_04_a,cmp_04_b);
	cmp_04 = _mm_packs_epi16(cmp_04,_mm_setzero_si128());
	
	int r_01 = _mm_movemask_epi8(cmp_01);
	int r_02 = _mm_movemask_epi8(cmp_02);
	int r_03 = _mm_movemask_epi8(cmp_03);
	int r_04 = _mm_movemask_epi8(cmp_04);
	
	r_01=_bit_scan_forward(r_01^ 0x1FFFF);
	r_02=_bit_scan_forward(r_02^ 0x1FFFF);
	r_03=_bit_scan_forward(r_03^ 0x1FFFF);
	r_04=_bit_scan_forward(r_04^ 0x1FFFF);
	
	//Level 0 Processing Ends here and Level 1 Begins
	//Level 1 Processing - 5 way
	__m128i lvl_11 = _mm_load_si128(&(tree->key_array[1][r_01 << 2])); 
	__m128i lvl_12 = _mm_load_si128(&(tree->key_array[1][r_02 << 2])); 
	__m128i lvl_13 = _mm_load_si128(&(tree->key_array[1][r_03 << 2])); 
	__m128i lvl_14 = _mm_load_si128(&(tree->key_array[1][r_04 << 2])); 
	
	__m128i cmp_11 = _mm_cmplt_epi32(lvl_11,k1);
	__m128i cmp_12 = _mm_cmplt_epi32(lvl_12,k2);
	__m128i cmp_13 = _mm_cmplt_epi32(lvl_13,k3);
	__m128i cmp_14 = _mm_cmplt_epi32(lvl_14,k4);
	
	int r_11 = _mm_movemask_ps((__m128)cmp_11);
	int r_12 = _mm_movemask_ps((__m128)cmp_12);
	int r_13 = _mm_movemask_ps((__m128)cmp_13);
	int r_14 = _mm_movemask_ps((__m128)cmp_14);
	
	r_11=_bit_scan_forward(r_11^ 0x1FF);
	r_12=_bit_scan_forward(r_12^ 0x1FF);
	r_13=_bit_scan_forward(r_13^ 0x1FF);
	r_14=_bit_scan_forward(r_14^ 0x1FF);

	r_11+=(r_01<<2)+r_01;
	r_12+=(r_02<<2)+r_02;
	r_13+=(r_03<<2)+r_03;
	r_14+=(r_04<<2)+r_04;
			
	//Level 1 Processing Ends here and Level 2 Begins
	//Level 2 Processing - 9 way
	__m128i lvl_21_a = _mm_load_si128(&(tree->key_array[2][r_11 << 3])); 
	__m128i lvl_21_b = _mm_load_si128(&(tree->key_array[2][(r_11 << 3) + 4])); 
	__m128i lvl_22_a = _mm_load_si128(&(tree->key_array[2][r_12 << 3])); 
	__m128i lvl_22_b = _mm_load_si128(&(tree->key_array[2][(r_12 << 3) + 4])); 
	__m128i lvl_23_a = _mm_load_si128(&(tree->key_array[2][r_13<< 3])); 
	__m128i lvl_23_b = _mm_load_si128(&(tree->key_array[2][(r_13 << 3) + 4])); 
	__m128i lvl_24_a = _mm_load_si128(&(tree->key_array[2][r_14<< 3])); 
	__m128i lvl_24_b = _mm_load_si128(&(tree->key_array[2][(r_14 << 3) + 4])); 

	__m128i cmp_21_a = _mm_cmplt_epi32(lvl_21_a,k1);
	__m128i cmp_21_b = _mm_cmplt_epi32(lvl_21_b,k1);

	__m128i cmp_22_a = _mm_cmplt_epi32(lvl_22_a,k2);
	__m128i cmp_22_b = _mm_cmplt_epi32(lvl_22_b,k2);

	__m128i cmp_23_a = _mm_cmplt_epi32(lvl_23_a,k3);
	__m128i cmp_23_b = _mm_cmplt_epi32(lvl_23_b,k3);

	__m128i cmp_24_a = _mm_cmplt_epi32(lvl_24_a,k4);
	__m128i cmp_24_b = _mm_cmplt_epi32(lvl_24_b,k4);

	__m128i cmp_21 = _mm_packs_epi32(cmp_21_a,cmp_21_b);
	__m128i cmp_22 = _mm_packs_epi32(cmp_22_a,cmp_22_b);
	__m128i cmp_23 = _mm_packs_epi32(cmp_23_a,cmp_23_b);
	__m128i cmp_24 = _mm_packs_epi32(cmp_24_a,cmp_24_b);

	cmp_21 = _mm_packs_epi16(cmp_21,_mm_setzero_si128());
	cmp_22 = _mm_packs_epi16(cmp_22,_mm_setzero_si128());
	cmp_23 = _mm_packs_epi16(cmp_23,_mm_setzero_si128());
	cmp_24 = _mm_packs_epi16(cmp_24,_mm_setzero_si128());

	int r_21 = _mm_movemask_epi8(cmp_21);
	r_21=_bit_scan_forward(r_21^ 0x1FFFF);
	r_21+=(r_11<<3)+r_11;

	int r_22 = _mm_movemask_epi8(cmp_22);
	r_22=_bit_scan_forward(r_22^ 0x1FFFF);
	r_22+=(r_12<<3)+r_12;

	int r_23 = _mm_movemask_epi8(cmp_23);
	r_23=_bit_scan_forward(r_23^ 0x1FFFF);
	r_23+=(r_13<<3)+r_13;

	int r_24 = _mm_movemask_epi8(cmp_24);
	r_24=_bit_scan_forward(r_24^ 0x1FFFF);
	r_24+=(r_14<<3)+r_14;

	*(result2+start+0)=r_21;
	*(result2+start+1)=r_22;
	*(result2+start+2)=r_23;
	*(result2+start+3)=r_24;
}

void cleanup_index(Tree* tree) {
        free(tree->node_capacity);
        for (size_t i = 0; i < tree->num_levels; ++i)
                free(tree->key_array[i]);
        free(tree->key_array);
        free(tree);
}
