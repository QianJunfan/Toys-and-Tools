//      This is a simplified and concise neofetch-like tool written in C. Hope you like it. :)
//      Code it for fun. :)
//                                  On vacation in Yunnan, China, August 10, 2025

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define BOLD    "\033[1m"
#define WHITE   "\033[37m"


// Init.
char* peek_username (void);
char* peek_kernel   (void);
char* peek_terminal (void);
char* peek_font     (void);
char* peek_uptime   (void);
char* peek_memory   (void);
char* peek_cpu      (void);
char* peek_hostname (void);
char* peek_os       (void);
char* peek_isa(void);
char* peek_shell(void);
char* peek_gpu(void);

typedef struct Info {
    char* User;
    char* OS;
    char* Host;
    char* ISA;
    char* Kernel;
    char* Uptime;
    char* Display;
    char* Shell;
    char* Terminall;
    char* Font;
    char* CPU;
    char* GPU;
    char* Memory;
} Info;

typedef struct Version{
    int major_version;
    int minor_version;
    const char *codename;
    const char *os_name;
} Version;



Version version[] = {
    {26, 0,  "Tahoe",       "macOS"},
    {15, 0,  "Sequoia",     "macOS"},
    {14, 0,  "Sonoma",      "macOS"},
    {13, 0,  "Ventura",     "macOS"},
    {12, 0,  "Monterey",    "macOS"},
    {11, 0,  "Big Sur",     "macOS"},
    {10, 15, "Catalina",    "macOS"},
    {10, 14, "Mojave",      "macOS"},
    {10, 13, "High Sierra", "macOS"},
    {10, 12, "Sierra",      "macOS"},
    {10, 11, "El Capitan",    "OS X"},
    {10, 10, "Yosemite",      "OS X"},
    {10, 9,  "Mavericks",     "OS X"},
    {10, 8,  "Mountain Lion", "OS X"},
    {10, 7,  "Lion",          "OS X"},
    {10, 6,  "Snow Leopard", "Mac OS X"},
    {10, 5,  "Leopard",      "Mac OS X"},
    {10, 4,  "Tiger",        "Mac OS X"},
    {10, 3,  "Panther",      "Mac OS X"},
    {10, 2,  "Jaguar",       "Mac OS X"},
    {10, 1,  "Puma",         "Mac OS X"},
    {10, 0,  "Cheetah",      "Mac OS X"},

    {-1, -1, NULL, NULL}
};

// Peek Info.
char* peek_gpu(void) {
    FILE *fp = popen("system_profiler SPDisplaysDataType | grep 'Chipset Model' | head -n 1", "r");
    if (!fp) {
        return strdup("Unknown");
    }

    char line[256];
    if (fgets(line, sizeof(line), fp) != NULL) {
        char* model = strchr(line, ':');
        if (model) {
            model += 2;
            char* newline = strchr(model, '\n');
            if (newline) *newline = '\0';

            pclose(fp);
            return strdup(model);
        }
    }

    pclose(fp);
    return strdup("Unknown");
}
char* peek_kernel(void)   {
    char osrelease[256];
    size_t size = sizeof(osrelease);
    
    if (sysctlbyname("kern.osrelease", osrelease, &size, NULL, 0) == -1) {
        return strdup("Unknown");
    }
    
    return strdup(osrelease);
}
char* peek_terminal(void) {
    char *term = getenv("TERM");
    if (!term) term = getenv("TERMINAL");
    return term ? strdup(term) : strdup("Unknown");
}
char* peek_font(void)     {

    char *font = getenv("FONT");
    if (!font) {

        font = getenv("TERM_FONT");
    }
    return font ? strdup(font) : strdup("SF Mono 12");
}
char* peek_uptime(void)   {
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
char* peek_username(void) {
    struct passwd *pw = getpwuid(geteuid());
    
    if (pw == NULL || pw->pw_name == NULL) {
        return NULL;
    }
    
    size_t len = strlen(pw->pw_name) + 1;
    char *username = (char*)malloc(len);
    
    if (username == NULL) {
        return NULL;
    }
    
    strcpy(username, pw->pw_name);
    
    return username;
}
char* peek_memory(void)   {
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
char* peek_cpu(void)      {
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
char* peek_hostname(void) {
    size_t len = 0;
    
    if (sysctlbyname("kern.hostname", NULL, &len, NULL, 0) == -1) {
        return strdup("Unknown");
    }

    char *hostname = malloc(len);
    if (!hostname) return NULL;
    
    if (sysctlbyname("kern.hostname", hostname, &len, NULL, 0) == -1) {
        free(hostname);
        return strdup("Unknown");
    }
    
    return hostname;
}

char* peek_os(void) {
    int major, minor;
    char product_version[256];
    char build_version[256];
    size_t size = sizeof(product_version);

    if (sysctlbyname("kern.osproductversion", product_version, &size, NULL, 0) == -1) {
        return NULL;
    }
    
    size = sizeof(build_version);
    if (sysctlbyname("kern.osversion", build_version, &size, NULL, 0) == -1) {
        return NULL;
    }

    if (sscanf(product_version, "%d.%d", &major, &minor) != 2) {
        return NULL;
    }

    const char* codename = "Unknown";
    const char* os_name = "macOS";
    for (int i = 0; version[i].codename != NULL; i++) {
        if (version[i].major_version == major) {
            codename = version[i].codename;
            os_name = version[i].os_name;
            break;
        }
    }

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s %s %d.%d %s",
             os_name, codename, major, minor, build_version);
    
    char* result = (char*)malloc(strlen(buffer) + 1);
    if (result) {
        strcpy(result, buffer);
    }
    
    
    return result;
}
char* peek_shell(void) {
    char* shell_path = getenv("SHELL");
    if (shell_path == NULL) {
        return NULL;
    }

    char* shell_name = strrchr(shell_path, '/');
    if (shell_name == NULL) {
        shell_name = shell_path;
    } else {
        shell_name++;
    }

    char command[512];
    snprintf(command, sizeof(command), "%s --version 2>&1", shell_path);

    FILE* fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to run command to get shell version.\n");
        char* result_name_only = (char*)malloc(strlen(shell_name) + 1);
        if (result_name_only) {
            strcpy(result_name_only, shell_name);
        }
        return result_name_only;
    }

    char version_output[256];
    if (fgets(version_output, sizeof(version_output), fp) == NULL) {
        pclose(fp);
        char* result_name_only = (char*)malloc(strlen(shell_name) + 1);
        if (result_name_only) {
            strcpy(result_name_only, shell_name);
        }
        return result_name_only;
    }

    pclose(fp);

    char* version_start = NULL;
    char* final_version = NULL;

    version_start = strstr(version_output, shell_name);
    if (version_start) {
        version_start += strlen(shell_name) + 1;
        char* version_end = strpbrk(version_start, " (");
        if (version_end) {
            *version_end = '\0';
            final_version = version_start;
        } else {
            final_version = version_start;
        }
    } else {
        version_start = strstr(version_output, "version ");
        if (version_start) {
            version_start += strlen("version ");
            char* version_end = strchr(version_start, ',');
            if (version_end) {
                *version_end = '\0';
            }
            final_version = version_start;
        }
    }

    char buffer[512];
    if (final_version) {
        snprintf(buffer, sizeof(buffer), "%s %s", shell_name, final_version);
    } else {
        snprintf(buffer, sizeof(buffer), "%s", shell_name);
    }

    char* result = (char*)malloc(strlen(buffer) + 1);
    if (result) {
        strcpy(result, buffer);
    }
    
    return result;
}
char* peek_isa(void) {
    int arch_type;
    size_t size = sizeof(arch_type);

    if (sysctlbyname("hw.cputype", &arch_type, &size, NULL, 0) == -1) {
        return NULL;
    }

    if (arch_type == 0x0100000c) {
        char* result = (char*)malloc(4);
        if (result) {
            strcpy(result, "arm");
        }
        return result;

    } else if (arch_type == 0x01000007) {
        char* result = (char*)malloc(4);
        if (result) {
            strcpy(result, "x86");
        }
        return result;
    }

    return NULL;
}


// Print info.


void UI(Info info) {
    // Logo.
    printf(
        "\n\n\n"
        "%s"
        " ███▄ ▄███▓ ▄▄▄       ▄████▄   ██▓███  ▓█████ ▓█████  ██ ▄█▀\n"
        " ▓██▒▀█▀ ██▒▒████▄    ▒██▀ ▀█  ▓██░  ██▒▓█   ▀ ▓█   ▀  ██▄█▒ \n"
        " ▓██    ▓██░▒██  ▀█▄  ▒▓█    ▄ ▓██░ ██▓▒▒███   ▒███   ▓███▄░ \n"
        " ▒██    ▒██ ░██▄▄▄▄██ ▒▓▓▄ ▄██▒▒██▄█▓▒ ▒▒▓█  ▄ ▒▓█  ▄ ▓██ █▄ \n"
        " ▒██▒   ░██▒ ▓█   ▓██▒▒ ▓███▀ ░▒██▒ ░  ░░▒████▒░▒████▒▒██▒ █▄\n"
        " ░ ▒░   ░  ░ ▒▒   ▓▒█░░ ░▒ ▒  ░▒▓▒░ ░  ░░░ ▒░ ░░░ ▒░ ░▒ ▒▒ ▓▒\n"
        " ░  ░      ░  ▒   ▒▒ ░  ░  ▒   ░▒ ░      ░ ░  ░ ░ ░  ░░ ░▒ ▒░\n"
        " ░      ░     ░   ▒   ░        ░░          ░      ░   ░ ░░ ░ \n"
        "        ░         ░  ░░ ░                  ░  ░   ░  ░░  ░   \n"
        "                      ░                                      \n"
        "                                                             \n"
        "%s",
        CYAN,
        RESET
    );
    
    printf("\n        from - \033[1m%s@%s\033[0m\n\n", (info.User != NULL)  ? info.User : "Unkown", (info.Host != NULL) ? info.Host : "Unkown");
    printf("        \033[1mOS\033[0m: %s\n",         (info.OS         != NULL)  ? info.OS         : "Unkown");
    printf("        \033[1mISA:\033[0m %s\n",        (info.ISA        != NULL)  ? info.ISA        : "Unkown");
    printf("        \033[1mKernel\033[0m: %s\n",     (info.Kernel     != NULL)  ? info.Kernel     : "Unkown");
    printf("        \033[1mUptime\033[0m: %s\n",     (info.Uptime     != NULL)  ? info.Uptime     : "Unkown");
    printf("        \033[1mShell\033[0m: %s\n",      (info.Shell      != NULL)  ? info.Shell      : "Unkown");
    printf("        \033[1mTerminal\033[0m: %s\n",   (info.Terminall  != NULL)  ? info.Terminall  : "Unkown");
    printf("        \033[1mFont\033[0m: %s\n",       (info.Font       != NULL)  ? info.Font       : "Unkown");
    printf("        \033[1mCPU\033[0m: %s\n",        (info.CPU        != NULL)  ? info.CPU        : "Unkwon");
    printf("        \033[1mGPU\033[0m: %s\n",        (info.GPU        != NULL)  ? info.GPU        : "Unkwon");

    printf("        \033[1mMemory:\033[0m %s\n",     (info.Memory     != NULL)  ? info.Memory     : "Unkown");
    printf("\n");
    printf("        %sStandard Colors     High Intensity Colors%s \n\n", BOLD, RESET);
        

    printf("        \033[40m   \033[0m");
    printf("\033[41m   \033[0m");
    printf("\033[42m   \033[0m");
    printf("\033[43m   \033[0m");
    printf("            ");
    printf("\033[100m   \033[0m");
    printf("\033[101m   \033[0m");
    printf("\033[102m   \033[0m");
    printf("\033[103m   \033[0m\n");
    
    printf("        \033[44m   \033[0m");
    printf("\033[45m   \033[0m");
    printf("\033[46m   \033[0m");
    printf("\033[47m   \033[0m");
    printf("            ");
    printf("\033[104m   \033[0m");
    printf("\033[105m   \033[0m");
    printf("\033[106m   \033[0m");
    printf("\033[107m   \033[0m\n\n\n");
}

int main(void) {
    Info info;
    info.User       = peek_username();
    info.OS         = peek_os();
    info.Host       = peek_hostname();
    info.ISA        = peek_isa();
    info.Kernel     = peek_kernel();
    info.Uptime     = peek_uptime();
    info.Shell      = peek_shell();
    info.Terminall  = peek_terminal();
    info.Font       = peek_font();
    info.CPU        = peek_cpu();
    info.GPU        = peek_gpu();
    info.Memory     = peek_memory();

    UI(info);


    free(info.OS);
    free(info.ISA);
    free(info.Host);
    free(info.Kernel);
    free(info.Uptime);
    free(info.Shell);
    free(info.Terminall);
    free(info.Font);
    free(info.CPU);
    free(info.Memory);
    free(info.User);
    free(info.GPU);

    return 0;
}
