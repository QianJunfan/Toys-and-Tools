#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>
#include <time.h>

char* peek_uptime() {
    struct timeval boottime;
    
    size_t len = sizeof(boottime);

    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    

    if (sysctl(mib, 2, &boottime, &len, NULL, 0) < 0) {
        char* error = malloc(6);
        
        strcpy(error, "unknown");
        return error;
    }
    

    time_t now;
    time(&now);
    long uptime = now - boottime.tv_sec;
    

    long days    =  uptime / (24 * 3600);
    long hours   = (uptime % (24 * 3600)) / 3600;
    long minutes = (uptime % 3600) / 60;
    long seconds =  uptime % 60;
    

    char* buffer = malloc(64);

    if (days > 0) {
        snprintf(buffer, 64, "%ld days, %ld hours, %ld minutes, %ld seconds", days, hours, minutes, seconds);
    } else if (hours > 0) {
        snprintf(buffer, 64, "%ld hours, %ld minutes, %ld seconds", hours, minutes, seconds);
    } else if (minutes > 0) {
        snprintf(buffer, 64, "%ld minutes, %ld seconds", minutes, seconds);
    } else {
        snprintf(buffer, 64, "%ld seconds", seconds);
    }
    
    return buffer;
}
char* peek_memory() {
    uint64_t memsize;
    size_t size = sizeof(memsize);
    
    if (sysctlbyname("hw.memsize", &memsize, &size, NULL, 0) == -1) {
        char* error = malloc(8);
        if (error) {
            strcpy(error, "Unknown");
        }
        return error;
    }
    
    char* buffer = malloc(32);
    if (!buffer) {
        return NULL;
    }
    

    if (memsize >= 1024 * 1024 * 1024) {
        double mem_gb = (double)memsize / (1024 * 1024 * 1024);
        snprintf(buffer, 32, "%.1f GB", mem_gb);
    } else {
        double mem_mb = (double)memsize / (1024 * 1024);
        snprintf(buffer, 32, "%.0f MB", mem_mb);
    }
    
    return buffer;
}
char* peek_cpu   () {
    char str[256] = {0};
    size_t size = sizeof(str);
    
    if (sysctlbyname("machdep.cpu.brand_string", str, &size, NULL, 0) == -1) {
        return "Unknown";
    }
    
    return strdup(str);
}

int main() {
    char* memory = peek_memory();
    char* uptime = peek_uptime();
    char* cpu    = peek_cpu();
    
    printf("System uptime: %s\n", uptime);
    printf("System memory: %s\n", memory);
    printf("System cpu: %s\n", cpu);
    
    free(uptime);
    free(memory);
    free(cpu);
    
    return 0;
}