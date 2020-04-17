// R. Jesse Chaney
// jesse.chaney@osucascades.edu

#include "beavalloc.h"

typedef struct mem_block_s {
    uint8_t free;

    size_t capacity;
    size_t size;

    struct mem_block_s *prev;
    struct mem_block_s *next;
} mem_block_t;

#define BLOCK_SIZE (sizeof(mem_block_t))
#define BLOCK_DATA(__curr) (((void *) __curr) + (BLOCK_SIZE))

static mem_block_t *block_list_head = NULL;

static void *lower_mem_bound = NULL;
static void *upper_mem_bound = NULL;

static uint8_t isVerbose = FALSE;
static FILE *beavalloc_log_stream = NULL;

// This is some gcc magic.
static void init_streams(void) __attribute__((constructor));

static void 
init_streams(void)
{
    beavalloc_log_stream = stderr;
}

void 
beavalloc_set_verbose(uint8_t verbosity)
{
    isVerbose = verbosity;
    if (isVerbose) {
        fprintf(beavalloc_log_stream, "Verbose enabled\n");
    }
}

void 
beavalloc_set_log(FILE *stream)
{
    beavalloc_log_stream = stream;
}

void *
beavalloc(size_t size)
{
    void *ptr = NULL;
    void putintothisblock(mem_block_t *block){
	mem_block_t *newblock = block + BLOCK_SIZE + block->size;
	newblock->capacity = block->capacity - block->size - BLOCK_SIZE;
	newblock->size = size;
	newblock->prev = block;
	newblock->next = block->next;
	if(newblock->next != NULL){
		newblock->next->prev = newblock;
	}
	block->next = newblock;
	ptr = (void *)newblock + BLOCK_SIZE;	
		
    }
    if(block_list_head == NULL){
	int increase_max_value = (((size + BLOCK_SIZE)/1024) + 1) * 1024;
        lower_mem_bound = sbrk(0);
        brk(lower_mem_bound + increase_max_value);
	upper_mem_bound = lower_mem_bound + increase_max_value;
	block_list_head  = lower_mem_bound;	
	block_list_head->free = 0;
	block_list_head->size = size;
	block_list_head->capacity = (increase_max_value - BLOCK_SIZE);
	block_list_head->next = NULL;
	block_list_head->prev = NULL;
	ptr = lower_mem_bound + BLOCK_SIZE;
	
    }else{
        mem_block_t* current = block_list_head;
	if(current->capacity - current->size >= size){
	    putintothisblock(current);	
	}else{
	    while(current->next != NULL){
                current = current->next;
	        if(current->capacity - current->size >= size){
		    putintothisblock(current);
		    break;
		}
            }
        }

	//need to put adding more sbrk here to fit more memory
    }
    return ptr;
}

void 
beavfree(void *ptr)
{
    mem_block_t* free_block = (mem_block_t*) ptr;
    free_block->free = 1;
    return;
}


void 
beavalloc_reset(void)
{
    brk(lower_mem_bound);

}

void *
beavcalloc(size_t nmemb, size_t size)
{
    void *ptr = NULL;

    return ptr;
}

void *
beavrealloc(void *ptr, size_t size)
{
    void *nptr = NULL;

    return nptr;
}

void 
beavalloc_dump(void)
{
    mem_block_t *curr = NULL;
    unsigned i = 0;
    unsigned user_bytes = 0;
    unsigned capacity_bytes = 0;
    unsigned block_bytes = 0;
    unsigned used_blocks = 0;
    unsigned free_blocks = 0;

    fprintf(beavalloc_log_stream, "Heap map\n");
    fprintf(beavalloc_log_stream
            , "  %s\t%s\t%s\t%s\t%s" 
              "\t%s\t%s\t%s\t%s\t%s"
            "\n"
            , "blk no  "
            , "block add "
            , "next add  "
            , "prev add  "
            , "data add  "
            
            , "blk size "
            , "capacity "
            , "size     "
            , "excess   "
            , "status   "
        );
    for (curr = block_list_head, i = 0; curr != NULL; curr = curr->next, i++) {
        fprintf(beavalloc_log_stream
                , "  %u\t\t%9p\t%9p\t%9p\t%9p\t%u\t\t%u\t\t"
                  "%u\t\t%u\t\t%s\t%c"
                , i
                , curr
                , curr->next
                , curr->prev
                , BLOCK_DATA(curr)

                , (unsigned) (curr->capacity + BLOCK_SIZE)
                , (unsigned) curr->capacity
                , (unsigned) curr->size
                , (unsigned) (curr->capacity - curr->size)
                , curr->free ? "free  " : "in use"
                , curr->free ? '*' : ' '
            );
        fprintf(beavalloc_log_stream, "\n");
        user_bytes += curr->size;
        capacity_bytes += curr->capacity;
        block_bytes += curr->capacity + BLOCK_SIZE;
        if (curr->free == TRUE) {
            free_blocks++;
        }
        else {
            used_blocks++;
        }
    }
    fprintf(beavalloc_log_stream
            , "  %s\t\t\t\t\t\t\t\t"
              "%u\t\t%u\t\t%u\t\t%u\n"
            , "Total bytes used"
            , block_bytes
            , capacity_bytes
            , user_bytes
            , capacity_bytes - user_bytes
        );
    fprintf(beavalloc_log_stream
            , "  Used blocks: %4u  Free blocks: %4u  "
              "Min heap: %9p    Max heap: %9p   Block size: %lu bytes\n"
            , used_blocks
            , free_blocks
            , lower_mem_bound
            , upper_mem_bound
            , BLOCK_SIZE
        );
}
