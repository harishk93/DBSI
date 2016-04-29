#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h> 
#include <smmintrin.h>
#include <nmmintrin.h>
#include <ammintrin.h> 
#include <x86intrin.h>
#include "p2random.h"
#include "tree.h"


int main(int argc, char* argv[]) {
        // parsing arguments
        assert(argc > 3);
        size_t num_keys = strtoull(argv[1], NULL, 0);
        size_t num_probes = strtoull(argv[2], NULL, 0);
        size_t num_levels = (size_t) argc - 3;
        size_t* fanout = malloc(sizeof(size_t) * num_levels);
        assert(fanout != NULL);
        for (size_t i = 0; i < num_levels; ++i) {
                fanout[i] = strtoull(argv[i + 3], NULL, 0);
                assert(fanout[i] >= 2 && fanout[i] <= 17);
        }
			
        // building the tree index
        rand32_t* gen = rand32_init((uint32_t) time(NULL));
        assert(gen != NULL);
        int32_t* delimiter = generate_sorted_unique(num_keys, gen);
        assert(delimiter != NULL);
        Tree* tree = build_index(num_levels, fanout, num_keys, delimiter);
        free(delimiter);
        free(fanout);
        if (tree == NULL) {
                free(gen);
                exit(EXIT_FAILURE);
        }


        // generate probes
        int32_t* probe = generate(num_probes, gen);
        assert(probe != NULL);
        free(gen);
        uint32_t* result = malloc(sizeof(uint32_t) * num_probes);
		uint32_t* result1 = malloc(sizeof(uint32_t) * num_probes);
		uint32_t* result2 = malloc(sizeof(uint32_t) * num_probes);
        assert(result != NULL);
		
        // perform index probing (Phase 2)
		struct timeval stop_1, stop_2, start_1, start_2, start_3, stop_3;

		gettimeofday(&start_1, NULL);
		for (size_t i = 0; i < num_probes; ++i) {
                result[i] = probe_index(tree, probe[i]);
        }
		gettimeofday(&stop_1, NULL);	

		gettimeofday(&start_2, NULL);
		for (size_t i = 0; i < num_probes; ++i) {
				result1[i] = probe_index_sse(tree,probe[i]);
        }
		gettimeofday(&stop_2, NULL);	
		gettimeofday(&start_3, NULL);
		int flag=0;
		if(tree->num_levels==3 && (tree->node_capacity[0]==8) && (tree->node_capacity[1]==4) && (tree->node_capacity[2]==8))
		{
			__m128i lvl_0_a = _mm_load_si128(&(tree->key_array[0][0])); 
			__m128i lvl_0_b = _mm_load_si128(&(tree->key_array[0][0])+4); 
        
			for (size_t i = 0; i < num_probes/4; ++i) {
				__m128i k=_mm_load_si128((__m128i*)&probe[4*i]);
				probe_hardcoded(tree,k,result2,lvl_0_a, lvl_0_b, 4*i);
			}
			flag=1; //Setting flag to determine whether result2 needs to be published or not
		}
		gettimeofday(&stop_3, NULL);
		int ch=0;
		printf("Enter 1 if you want to print the results along with the time:");
		scanf("%d",&ch);
		if(ch==1){
		for (size_t i = 0; i < num_probes; ++i) {
                if(flag==1)
				fprintf(stdout, "%d:%d %u %u %u\n", i,probe[i], result[i], result1[i], result2[i]);
				else
				fprintf(stdout, "%d:%d %u %u\n", i,probe[i], result[i], result1[i]);	
        } 
		}
		printf("Method 1 took %lu.\n", stop_1.tv_usec - start_1.tv_usec);
		printf("Method 2 took %lu.\n", stop_2.tv_usec - start_2.tv_usec);
		if(flag==1)printf("Method 3 took %lu.\n", stop_3.tv_usec - start_3.tv_usec);
        free(result);
		free(result1);
		free(result2);
		free(probe);
        cleanup_index(tree);
        return EXIT_SUCCESS;
}
