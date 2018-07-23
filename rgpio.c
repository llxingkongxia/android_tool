#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#if __LP64__
#define strtoptr strtoull
#else
#define strtoptr strtoul
#endif

static int usage()
{
    fprintf(stderr,"rgpio [<gpio number n>,n=0..141] 135 .. 138 can't read!\n");
    return -1;
}

int read_gpio_n(uint32_t n)
{
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(fd < 0) {
        fprintf(stderr,"cannot open /dev/mem\n");
        return -1;
    }

    uintptr_t addr = 0x1000000 + n * 0x1000;
    off64_t mmap_start = addr & ~(PAGE_SIZE - 1);
    size_t mmap_size = 8;
    mmap_size = (mmap_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    void* page = mmap64(0, mmap_size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, mmap_start);

    if(page == MAP_FAILED){
        fprintf(stderr,"cannot mmap region\n");
        return -1;
    }

    volatile uint32_t* value = (uint32_t*) (((uintptr_t) page) + (addr & 4095));
    printf("    %03d     ", n);
    if ((*value) & 0x200)
	printf(" O  ");
    else
	printf(" I  ");
    printf("  %02dmA  ", (((*value) & 0x1c0) >> 6) * 2 + 2);
    printf("    %d    ", ((*value) & 0x3c) >> 2);
    switch ((*value) & 0x3) {
	case 0:
		printf(" no_pull  ");
		break;
	case 1:
		printf("pull_down ");
		break;
	case 2:
		printf("  keeper  ");
		break;
	case 3:
		printf(" pull_up  ");
		break;
    }
    volatile uint32_t* value1 = (uint32_t*) (((uintptr_t) page) + ((addr + 4) & 4095));
    if ((*value) & 0x200) {
	if ((*value1) & 0x2)
		printf("   high  \n");
	else
		printf("   low   \n");
    }
    else {
	if ((*value1) & 0x1)
		printf("   high  \n");
	else
		printf("   low   \n");
    }

    return 0;
}

int main(int argc, char *argv[])
{
    uint32_t number = 0;
    if(argc > 2) return usage();
    else {
    	if(argc == 2) {
		number = strtoptr(argv[1], 0, 10);
    		if(number > 141) return usage();
			if(number > 134 && number < 139) return usage();
		printf("gpio_number I/O driving func_sel gpio_pull gpio_value\n");
		read_gpio_n(number);
	}
	else {
		printf("gpio_number I/O driving func_sel gpio_pull gpio_value\n");
		for (number = 0;number <= 134;number++)
			read_gpio_n(number);

		for(number = 139;number <= 141;number++)
			read_gpio_n(number);
	}
    }

    return 0;
}
