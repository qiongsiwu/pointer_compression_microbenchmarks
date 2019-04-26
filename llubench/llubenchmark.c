/*
 * LLUBENCHMARK
 * Craig Zilles (zilles@cs.wisc.edu)
 * http://www.cs.wisc.edu/~zilles/llubenchmark.html
 *
 * This program is a linked list traversal micro-benchmark, which can
 * be used (among other things) to approximate the non-benchmark
 * Health.
 *
 * The benchmark executes for a proscribed number of iterations (-i),
 * and on every iteration the lists are traversed and potentially
 * extended.  The number of lists can be specified (-n) as well as the
 * size of the elements in the list (-s).  The initial length of the
 * lists can be set (-l) as well as the growth rate (-g).  The growth
 * rate must be non-negative, but can be a floating point number, in
 * which case random numbers are used to determine whether a list is
 * extended on a particular cycle (all lists are extended
 * independently).  If the -t option is specified, the insertion
 * occurs at the tail, otherwise at the head.  If the -d option is
 * specified, the elements are dirtied during the traversal (which
 * will necessitate a write-back when the data is evicted from the
 * cache).
 *
 * To approximate the non-benchmark Health, use the options:
 *     -i <num iterations> -g .333 -d -t -n 341
 *
 * (the growth rate of the lists in health is different for different
 * levels of the hierarchy and the constant .333 is just my
 * approximation of the growth rate).
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* This file should compile stand alone */

struct element {
    struct element *next;
    int count;
};

void usage(char *name) {
    printf("%s:\n", name);
    printf("-i <number of (I)terations>\n");
    printf("[-l <initial (L)ength of list, in elements>] (default 1)\n");
    printf("[-n <(N)umber of lists>] (default 1 list)\n");
    printf("[-s <(S)ize of element>] (default 32 bytes)\n");
    printf("[-g <(G)rowth rate per list, in elements per iteration>] (default "
           "0)\n");
    printf("[-d] ((D)irty each element during traversal, default off)\n");
    printf("[-t] (insert at (T)ail of list, default off)\n");
    printf("[-v] (record all allocated and accessed pointers, default off)\n");
}

#define ALLOC_SIZE 127 /* pick wierd num to break strides */
struct element *free_list = NULL;
int next_free = ALLOC_SIZE;
int element_size = 64;
int num_allocated = 0;
int verbose = 0; 

struct element *allocate(FILE *ptr_file) {
    if (next_free == ALLOC_SIZE) {
        next_free = 0;
        free_list = (struct element *)malloc(ALLOC_SIZE * element_size);
        assert(free_list != 0);
    }
    num_allocated++;
    struct element *ptr = (struct element *)(((char *)free_list) +
                                             ((next_free++) * element_size));
    if (verbose) {
        fprintf(ptr_file, "%p\n", ptr); 
    }
    
    return ptr; 
}

int main(int argc, char *argv[]) {
    int max_iterations = 0, dirty = 0, num_lists = 1, tail = 0,
        initial_length = 1;
    float growth_rate = 0.0;
    char c = 0;
    int i = 0, j = 0, k = 0;
    int accumulate = 0;

    struct element **lists = NULL;
    float growth = 0.0;

    int arg = 1;
    while (arg < argc) {
        if ((argv[arg][0] != '-') || (argv[arg][2] != 0)) {
            printf("parse error in %s\n", argv[arg]);
            usage(argv[0]);
            return (-1);
        }
        c = argv[arg][1];
        arg++;
        switch (c) {
        case 'd':
            dirty = 1;
            break;
        case 'g':
            growth_rate = atof(argv[arg++]);
            break;
        case 'i':
            max_iterations = atoi(argv[arg++]);
            break;
        case 'l':
            initial_length = atoi(argv[arg++]);
            break;
        case 'n':
            num_lists = atoi(argv[arg++]);
            break;
        case 's':
            element_size = atoi(argv[arg++]);
            break;
        case 't':
            tail = 1;
            break;
        case 'v': 
            verbose = 1; 
            break; 
        default:
            printf("unrecognized option: %c\n", c);
            usage(argv[0]);
            return (-1);
        }
    }

    assert(element_size > sizeof(struct element));
    assert(initial_length > 0);

    FILE *ptr_alloca_file = fopen("ptr_alloca_file", "w");
    FILE *ptr_access_file = fopen("ptr_access_file", "w"); 
    if (!ptr_alloca_file || !ptr_access_file) {
        perror("Cannot open file"); 
        exit(-1); 
    }

    /* build lists */
    lists = (struct element **)malloc(num_lists * sizeof(struct element *));
    assert(lists != 0);

    for (i = 0; i < num_lists; i++) {
        lists[i] = NULL;
    }

    for (i = 0; i < initial_length; i++) {
        for (j = 0; j < num_lists; j++) {
            struct element *e = allocate(ptr_alloca_file);
            e->next = lists[j];
            if (verbose) {
                fprintf(ptr_access_file, "%p\n", e->next); 
            }
            lists[j] = e;
        }
    }

    /* iterate */
    for (i = 0; i < max_iterations; i++) {
        if ((i % 10) == 0) {
            printf("%d\n", i);
        }
        /* traverse lists */
        for (j = 0; j < num_lists; j++) {
            struct element *trav = lists[j];
            while (trav != NULL) {
                accumulate += trav->count;
                if (dirty) {
                    trav->count++;
                }
                if (verbose) {
                    fprintf(ptr_access_file, "%p\n", trav); 
                }
                trav = trav->next;
            }
        }

        /* grow lists */
        growth += growth_rate;
        j = growth;
        growth -= j;
        for (; j > 0; j--) {
            for (k = 0; k < num_lists; k++) {
                struct element *e = allocate(ptr_alloca_file);
                if (tail) {
                    struct element *trav = lists[k];
                    while (trav->next != NULL) {
                        if (verbose) {
                            fprintf(ptr_access_file, "%p\n", trav); 
                        }
                        trav = trav->next;
                    }
                    trav->next = e;
                    e->next = NULL;
                } else {
                    e->next = lists[k];
                    lists[k] = e;
                }
            }
        }
    }
    printf("num allocated %d\n", num_allocated);
    fclose(ptr_alloca_file); 
    fclose(ptr_access_file); 
    return 0;
}
