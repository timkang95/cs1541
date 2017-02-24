#include <stdio.h>
#include "trace_item.h"
#include "skeleton.h"

#define TRACE_BUFSIZE 1024*1024

static FILE *trace_fd;
static int trace_buf_ptr;
static int trace_buf_end;
static struct trace_item *trace_buf;

// to keep statistics
unsigned int accesses = 0;
unsigned int read_accesses = 0;
unsigned int write_accesses = 0;
unsigned int hits = 0;
unsigned int misses = 0;
unsigned int misses_with_writeback = 0;

int checkPowerOfTwo(int x){
  while(x > 1){
    if(x % 2 != 0){
      return 0;
    }
    x /= 2;
  }
  return 1;
}

void trace_init()
{
  trace_buf = malloc(sizeof(struct trace_item) * TRACE_BUFSIZE);

  if (!trace_buf) {
    fprintf(stdout, "** trace_buf not allocated\n");
    exit(-1);
  }

  trace_buf_ptr = 0;
  trace_buf_end = 0;
}

void trace_uninit()
{
  free(trace_buf);
  fclose(trace_fd);
}

int trace_get_item(struct trace_item **item)
{
  int n_items;

  if (trace_buf_ptr == trace_buf_end) {
    // get new data
    n_items = fread(trace_buf, sizeof(struct trace_item), TRACE_BUFSIZE, trace_fd);
    if (!n_items) return 0;

    trace_buf_ptr = 0;
    trace_buf_end = n_items;
  }

  *item = &trace_buf[trace_buf_ptr];
  trace_buf_ptr++;

  return 1;
}

int main(int argc, char **argv)
{
  struct trace_item *tr_entry;
  size_t size;
  char *trace_file_name;
  int trace_view_on;
  int result;
  
  if (argc == 1) {
    fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - any character>\n");
    fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
    exit(0);
  }

  trace_file_name = argv[1];
  trace_view_on = atoi(argv[2]);
  size_t cache_size = atoi(argv[3]);
  size_t block_size = atoi(argv[4]);
  int associativity = atoi(argv[5]);
  enum cache_policy policy = atoi(argv[6]);

  //pc counter to hold for replacement algorithms
  long long cycle = 0;

  //checks for powers of 2 requirements
  if(checkPowerOfTwo(cache_size) == 0 || checkPowerOfTwo(block_size) == 0 || checkPowerOfTwo(associativity) == 0 && (associativity != 0 || associativity != 1)){
    printf("\nMake sure your settings are appropriate \n");
    return -1;
  }

  // here you should extract the cache parameters from the command line

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
    fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
    exit(0);
  }

  trace_init();
  struct cache_t *cp = cache_create(cache_size, block_size, associativity, policy);

  while(1) {
    size = trace_get_item(&tr_entry);
    cycle++;

    if (!size) {       /* no more instructions to simulate */
      printf("+ number of accesses : %d \n", accesses);
      printf("+ number of reads : %d \n", read_accesses);
      printf("+ number of writes : %d \n", write_accesses);
      printf("+ number of hits: %d \n", hits);
      printf("+ number of misses: %d \n", misses);
      printf("+ number of misses with writebacks: %d \n", misses_with_writeback);
      break;
    }

   

    else{              /* process only loads and stores */;
      if (tr_entry->type == ti_LOAD) {
        if (trace_view_on) printf("LOAD %X \n",tr_entry->Addr) ;
        accesses ++;
        read_accesses++ ;
        result = cache_access(cp, tr_entry->Addr, (char)tr_entry->type, cycle, trace_view_on); 

        if(result == 0){
          hits++;
        }
        else if(result == 1){
          misses++;
        }
        else{
          misses_with_writeback++;
        }
      }
      if (tr_entry->type == ti_STORE) {
        if (trace_view_on) printf("STORE %X \n",tr_entry->Addr) ;
        accesses ++;
        write_accesses++ ;
        result = cache_access(cp, tr_entry->Addr, (char)tr_entry->type, cycle, trace_view_on); 

        if(result == 0){
          hits++;
	  //if (trace_view_on) printf("HIT") ;
        }
        else if(result == 1){
          misses++;
	  //if (trace_view_on) printf("MISS") ;
        }
        else{
          misses_with_writeback++;
	  //if (trace_view_on) printf("MISS WITH WRITEBACK") ;
        }
      }
    }  
  }

  trace_uninit();
  exit(0);
}


