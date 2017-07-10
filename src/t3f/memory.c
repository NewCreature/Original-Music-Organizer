#include "t3f.h"

#define T3F_MAX_MEMORY_INFO 1000000

static ALLEGRO_MEMORY_INTERFACE t3f_memory_interface;
int t3f_alloc_count = 0;
unsigned long t3f_current_memory_usage = 0;
unsigned long t3f_max_memory_usage = 0;

typedef struct
{
	
	void * p;
	unsigned long size;

} MEM_BLOCK;

MEM_BLOCK mem_block[10000];
int mem_blocks = 0;

static int find_mem_block(void * p)
{
	int i;
	
	for(i = 0; i < mem_blocks; i++)
	{
		if(p == mem_block[i].p)
		{
			return i;
		}
	}
	return -1;
}

static int remove_mem_block(void * p)
{
	int i, j;
	
	i = find_mem_block(p);
	if(i < 0)
	{
		return 0;
	}
	for(j = i; j < mem_blocks - 1; j++)
	{
		memcpy(&mem_block[j], &mem_block[j + 1], sizeof(MEM_BLOCK));
	}
	mem_blocks--;
	return 1;
}

static int add_mem_block(void * p, unsigned long size)
{
	if(mem_blocks < 10000)
	{
		mem_block[mem_blocks].p = p;
		mem_block[mem_blocks].size = size;
		mem_blocks++;
		return 1;
	}
	else
	{
		printf("max memory tracking reached\n");
	}
	return 0;
}

static void update_max(void)
{
	printf("%05d: max memory usage: %lu\n", t3f_alloc_count, t3f_max_memory_usage);
	if(t3f_current_memory_usage > t3f_max_memory_usage)
	{
		t3f_max_memory_usage = t3f_current_memory_usage;
	}
}

void * t3f_malloc(size_t n, int line, const char * file, const char * func)
{
	void * p;
	
	p = malloc(n);
	if(p)
	{
		t3f_alloc_count++;
		add_mem_block(p, n);
		t3f_current_memory_usage += n;
		update_max();
	}
	return p;
}

void * t3f_realloc(void * ptr, size_t n, int line, const char * file, const char * func)
{
	void * p;
	int i;

	i = find_mem_block(ptr);
	p = realloc(ptr, n);
	if(p)
	{
		t3f_current_memory_usage += n - mem_block[i].size;
		if(i >= 0)
		{
			mem_block[i].size = n;
			mem_block[i].p = p;
		}
		update_max();
	}
	return p;
}

void * t3f_calloc(size_t count, size_t n, int line, const char * file, const char * func)
{
	void * p;
	
	p = calloc(count, n);
	if(p)
	{
		t3f_alloc_count++;
		add_mem_block(p, n);
		t3f_current_memory_usage += n;
		update_max();
	}
	return p;
}

void t3f_free(void * ptr, int line, const char * file, const char * func)
{
	int i;
	
	i = find_mem_block(ptr);
	if(i >= 0)
	{
		t3f_current_memory_usage -= mem_block[i].size;
		printf("freed: %lu\n", mem_block[i].size);
	}
	else
	{
		printf("no memory freed: %lu\n", ptr);
	}
	remove_mem_block(ptr);
	free(ptr);
	t3f_alloc_count--;
}

void t3f_setup_memory_interface(void)
{
	t3f_memory_interface.mi_malloc = t3f_malloc;
	t3f_memory_interface.mi_free = t3f_free;
	t3f_memory_interface.mi_realloc = t3f_realloc;
	t3f_memory_interface.mi_calloc = t3f_calloc;
	al_set_memory_interface(&t3f_memory_interface);
}
