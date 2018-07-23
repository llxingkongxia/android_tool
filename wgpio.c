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
    fprintf(stderr,"usage :  \n"
                   "(tip:gpio_number = [0...134],[139...141] ,because [135...138] can't write!)\n"
                   " wgpio  <options>  <gpio_number>  <val>\n"
                   "options include:\n"
                   " 1 <gpio_number> <direction>     set direction 0 for input,1 for out\n"
                   " 2 <gpio_number> <drv_stre>      set drv_stre 0 for 2mA,1 for 4mA,\n"
                   "                                 2 for 6mA... 7 for 16mA\n"
                   " 3 <gpio_number> <gpio_pull>     set gpio_pull 0 for no-pull,1 for pull-down,\n"
                   "                                 2 for keeper,3 for pull-up\n"
                   " 4 <gpio_number> <value>         set value 0 for low,1 for high\n"
                   " 5 <gpio_number> <func_sel>      set func_sel 0 for gpio mode,[1-15] for others\n");
    return -1;
}

int main(int argc, char *argv[])
{
    if(argc != 4) return usage();
    int  width = 4;
    int  case_num = strtoptr(argv[1], 0, 10);
    int  gpio_number = strtoptr(argv[2], 0, 10);

    if(gpio_number > 141) return usage();
    if(gpio_number > 134 && gpio_number < 139) return usage();

    uintptr_t addr = 0x1000000 + gpio_number * 0x1000;
    uintptr_t endaddr = 0;
    char* end = strchr(argv[2], '-');
    if (end)
        endaddr = strtoptr(end + 1, 0, 16);

    if (!endaddr)
        endaddr = addr + width - 1;

    if (endaddr <= addr) {
        fprintf(stderr, "end address <= start address\n");
        return -1;
    }
    
    uint32_t value = 0;
    value = strtoul(argv[3], 0, 10) ;

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if(fd < 0) {
        fprintf(stderr,"cannot open /dev/mem\n");
        return -1;
    }

    off64_t mmap_start = addr & ~(PAGE_SIZE - 1);
    size_t mmap_size = endaddr - mmap_start + 1;
    mmap_size = (mmap_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    void* page = mmap64(0, mmap_size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, mmap_start);

    if(page == MAP_FAILED){
        fprintf(stderr,"cannot mmap region\n");
        return -1;
    }
   
    volatile uint32_t* x = (uint32_t*) (((uintptr_t) page) + (addr & 4095));
    switch (case_num) {
    case 1:
    {  
        if((value!=0)&&(value!=1))
        {
            fprintf(stderr,"value wrong!!  \n");
            return -1;
        }
        *x = ( (*x) & 0xfffffdff ) + (value << 9);
        fprintf(stderr,"%08"PRIxPTR": %08x\n", addr, *x);
        break;
    }
    case 2:
    {    
        if(value>7)
        {
            fprintf(stderr,"value wrong!!  \n");
            return -1;
        }
        *x = ((*x) & 0xfffffe3f ) + (value << 6);
        fprintf(stderr,"%08"PRIxPTR": %08x\n", addr, *x);
        break;
    }
    case 3:
    {
        if(value>3)
        {
            fprintf(stderr,"value wrong!!  \n");
            return -1;
        }
        *x = ((*x ) & 0xfffffffc ) + value ;
        fprintf(stderr,"%08"PRIxPTR": %08x\n", addr, *x);
        break;
    }
    case 4:
    {
        if((value!=0)&&(value!=1))
        {
            fprintf(stderr,"value wrong!!  \n");
            return -1;
        }
        volatile uint32_t* x1 = (uint32_t*) (((uintptr_t) page) + ((addr+4) & 4095));
        if ((*x) & 0x200)
        {
            *x1 = ( (*x1) & 0xfffffffc )+(value *2+1) ;
            fprintf(stderr,"%08"PRIxPTR": %08x\n", addr, *x1);
        }
        else
        printf("it is input!\n");
        break;
    }
    case 5:
    {
        if(value>15)
        {
            fprintf(stderr,"value wrong!!  \n");
            return -1;
        }
        *x = ((*x ) & 0xffffffc3 ) + (value << 2) ;
        fprintf(stderr,"%08"PRIxPTR": %08x\n", addr, *x);
        break;
    }
  }    
  close(fd);
  return 0;
}
