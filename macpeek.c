#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>

// Peek info.
char* peek_kernel()   {
    char osrelease[256];
    size_t size = sizeof(osrelease);
    
    if (sysctlbyname("kern.osrelease", osrelease, &size, NULL, 0) == -1) {
        return strdup("Unknown");
    }
    
    return strdup(osrelease);
}
char* peek_display()  {
    char *display = getenv("DISPLAY");
    return display ? strdup(display) : strdup(":0");
}
char* peek_terminal() {
    char *term = getenv("TERM");
    if (!term) term = getenv("TERMINAL");
    return term ? strdup(term) : strdup("Unknown");
}
char* peek_font()     {

    char *font = getenv("FONT");
    if (!font) {

        font = getenv("TERM_FONT");
    }
    return font ? strdup(font) : strdup("SF Mono 12");
}
char* peek_uptime()   {
    struct timeval boottime;
    size_t len = sizeof(boottime);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    
    if (sysctl(mib, 2, &boottime, &len, NULL, 0) < 0) {
        return strdup("unknown");
    }
    
    time_t now;
    time(&now);
    long uptime = now - boottime.tv_sec;
    
    long days = uptime / (24 * 3600);
    long hours = (uptime % (24 * 3600)) / 3600;
    long minutes = (uptime % 3600) / 60;
    long seconds = uptime % 60;
    
    char* buffer = malloc(64);
    if (!buffer) return NULL;
    
    if (days > 0) {
        snprintf(buffer, 64, "%ld days, %ld hours, %ld minutes, %ld seconds",
                 days, hours, minutes, seconds);
    } else if (hours > 0) {
        snprintf(buffer, 64, "%ld hours, %ld minutes, %ld seconds",
                 hours, minutes, seconds);
    } else if (minutes > 0) {
        snprintf(buffer, 64, "%ld minutes, %ld seconds",
                 minutes, seconds);
    } else {
        snprintf(buffer, 64, "%ld seconds", seconds);
    }
    
    return buffer;
}
char* peek_memory()   {
    uint64_t memsize;
    size_t size = sizeof(memsize);
    
    if (sysctlbyname("hw.memsize", &memsize, &size, NULL, 0) == -1) {
        return strdup("Unknown");
    }
    
    char* buffer = malloc(32);
    if (!buffer) return NULL;
    
    if (memsize >= 1024 * 1024 * 1024) {
        double mem_gb = (double)memsize / (1024 * 1024 * 1024);
        snprintf(buffer, 32, "%.1f GB", mem_gb);
    } else {
        double mem_mb = (double)memsize / (1024 * 1024);
        snprintf(buffer, 32, "%.0f MB", mem_mb);
    }
    
    return buffer;
}
char* peek_cpu()      {
    char* buffer = NULL;
    size_t size = 0;
    

    if (sysctlbyname("machdep.cpu.brand_string", NULL, &size, NULL, 0) == -1) {
        return NULL;
    }
    
    buffer = malloc(size);
    if (!buffer) return NULL;
    
    if (sysctlbyname("machdep.cpu.brand_string", buffer, &size, NULL, 0) == -1) {
        free(buffer);
        return NULL;
    }
    
    return buffer;
}
char* peek_hostname() {
    size_t len = 0;
    
    if (sysctlbyname("kern.hostname", NULL, &len, NULL, 0) == -1) {
        return strdup("Unknown");
    }
    
    char* hostname = malloc(len);
    if (!hostname) return NULL;
    
    if (sysctlbyname("kern.hostname", hostname, &len, NULL, 0) == -1) {
        free(hostname);
        return strdup("Unknown");
    }
    
    return hostname;
}

// Print info.

int main() {
    char* memory   = peek_memory();
    char* uptime   = peek_uptime();
    char* cpu      = peek_cpu();
    char* hostname = peek_hostname();
    char* kernel   = peek_kernel();
    char* terminal = peek_terminal();
    char* font     = peek_font();
    
    printf("Uptime: %s\n", uptime);
    printf("Memory: %s\n", memory);
    printf("CPU: %s\n", cpu);
    printf("Hostname: %s\n", hostname);
    printf("Kernel version: %s\n", kernel);
    printf("Terminal: %s\n", terminal);
    printf("Font: %s\n", font);
    
    free(uptime);
    free(memory);
    free(cpu);
    free(hostname);
    free(kernel);
    free(terminal);
    free(font);
    
    return 0;
}