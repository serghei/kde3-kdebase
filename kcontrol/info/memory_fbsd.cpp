
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/vmmeter.h>

#include <vm/vm_param.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void KMemoryWidget::update()
{
    char blah[10], buf[80], *used_str, *total_str;
    /* Stuff for sysctl */
    int memory;
    size_t len;
    /* Stuff for swap display */
    int used, total, _free;
    FILE *pipe;

    len = sizeof(memory);
    sysctlbyname("hw.physmem", &memory, &len, NULL, 0);

    snprintf(blah, 10, "%d", memory);
    // Numerical values

    // total physical memory (without swap space)
    Memory_Info[TOTAL_MEM] = MEMORY(memory);

    // added by Brad Hughes bhughes@trolltech.com
    struct vmtotal vmem;

#warning "FIXME: Memory_Info[CACHED_MEM]"
    Memory_Info[CACHED_MEM] = NO_MEMORY_INFO;

    // The sysctls don't work in a nice manner under FreeBSD v2.2.x
    // so we assume that if sysctlbyname doesn't return what we
    // prefer, assume it's the old data types.   FreeBSD prior
    // to 4.0-R isn't supported by the rest of KDE, so what is
    // this code doing here.

    len = sizeof(vmem);
    if(sysctlbyname("vm.vmmeter", &vmem, &len, NULL, 0) == 0)
        Memory_Info[SHARED_MEM] = MEMORY(vmem.t_armshr) * PAGE_SIZE;
    else
        Memory_Info[SHARED_MEM] = NO_MEMORY_INFO;

    int buffers;
    len = sizeof(buffers);
    if((sysctlbyname("vfs.bufspace", &buffers, &len, NULL, 0) == -1) || !len)
        Memory_Info[BUFFER_MEM] = NO_MEMORY_INFO;
    else
        Memory_Info[BUFFER_MEM] = MEMORY(buffers);

    // total free physical memory (without swap space)
    int free;
    len = sizeof(buffers);
    if((sysctlbyname("vm.stats.vm.v_free_count", &free, &len, NULL, 0) == -1) || !len)
        Memory_Info[FREE_MEM] = NO_MEMORY_INFO;
    else
        Memory_Info[FREE_MEM] = MEMORY(free) * getpagesize();

    // Q&D hack for swap display. Borrowed from xsysinfo-1.4
    if((pipe = popen("/usr/sbin/pstat -ks", "r")) == NULL)
    {
        used = total = 1;
        return;
    }

    fgets(buf, sizeof(buf), pipe);
    fgets(buf, sizeof(buf), pipe);
    fgets(buf, sizeof(buf), pipe);
    fgets(buf, sizeof(buf), pipe);
    pclose(pipe);

    strtok(buf, " ");
    total_str = strtok(NULL, " ");
    used_str = strtok(NULL, " ");
    used = atoi(used_str);
    total = atoi(total_str);

    _free = total - used;

    // total size of all swap-partitions
    Memory_Info[SWAP_MEM] = MEMORY(total) * 1024;

    // free memory in swap-partitions
    Memory_Info[FREESWAP_MEM] = MEMORY(_free) * 1024;
}
