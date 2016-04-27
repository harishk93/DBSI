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

//Global Declaration of Level 0 for Optimization of Hardcoded Segment 
__m128i lvl_0_a;
__m128i lvl_0_b;

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
		lvl_0_a = _mm_load_si128(&(tree->key_array[0][0])); 
		lvl_0_b = _mm_load_si128(&(tree->key_array[0][0])+4); 
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

/*uint32_t probe_index2(Tree* tree, int32_t probe_key)
{
    size_t n = tree->num_levels;
	size_t i = 0; 
	for (i=0; i<n; i++)
	{
		size_t capacity = tree->node_capacity[i];
		
	}
	     	
}*/


uint32_t probe_harcoded(Tree* tree, int32_t probe_key)
{
	size_t result = 0; 
	printf("probe:%d\n",probe_key);
	
	//Loading of Probe Key into a 128 register
	__m128i key = _mm_cvtsi32_si128(probe_key);
	key = _mm_shuffle_epi32(key,_MM_SHUFFLE(0,0,0,0));
	
	//Loading of Level 0 into Register - 9 Way
	__m128i cmp_0_a = _mm_cmplt_epi32(lvl_0_a,key);
	__m128i cmp_0_b = _mm_cmplt_epi32(lvl_0_b,key);
	__m128i cmp_0 = _mm_packs_epi32(cmp_0_a,cmp_0_b);
	cmp_0 = _mm_packs_epi16(cmp_0,_mm_setzero_si128());
	int r_0 = _mm_movemask_epi8(cmp_0);
			//print128_num(cmp_0_a);
			//print128_num(cmp_0_b);
			//print128_num(cmp_0);
	r_0=_bit_scan_forward(r_0^ 0x1FFFF);
	printf("r_0:%d\n",r_0);
	
	//Level 0 Processing Ends here and Level 1 Begins
	//Leveel 1 Processing - 5 way
	__m128i lvl_1 = _mm_load_si128(&(tree->key_array[1][r_0 << 2])); 
		print128_num(lvl_1);
	__m128i cmp_1 = _mm_cmplt_epi32(lvl_1,key);
		print128_num(cmp_1);
	int r_1 = _mm_movemask_epi8(cmp_1);
	r_1=_bit_scan_forward(r_1^ 0x1FF);
	printf("r_1:%d\n",r_1);
	r_1+=(r_0<<2)+r_0;
	printf("r_1:%d\n",r_1);
	
	
}

void cleanup_index(Tree* tree) {
        free(tree->node_capacity);
        for (size_t i = 0; i < tree->num_levels; ++i)
                free(tree->key_array[i]);
        free(tree->key_array);
        free(tree);
}
