// standard 
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#define STRBUFLEN 255
static int sender = 0;
static int frag = 1;

struct pstat{
    long unsigned int utime_ticks;
    long int cutime_ticks;
    long unsigned int stime_ticks;
    long int cstime_ticks;
    long unsigned int cpu_total_time;
};
struct option long_options[] = {
    {"help",        no_argument,        0,  'h'},
    {"application", required_argument,  0,  'a'},
    {"middleware",  required_argument,  0,  'm'},
    {"time",        required_argument,  0,  't'},
    {0, 0, 0, 0}
};

void usage (const char* program)
{
        printf ("USAGE %s [-hamt]\n\n", program);
}

void help (const char * program)
{
    usage(program);
    printf("OPTION\n");
    printf("  -h, --help\n  \tPrint this help screen.\n\n");
    printf("  -a, --application <NAME>\n  \tName of application process.\n\n");
    printf("  -m, --middleware <NAME>\n  \tName of middleware process.\n\n");
    printf("  -t, --time <TIME>\n  \tStop application after TIME.\n\n");
    printf("AUTHOR\n  Sebastian Meiling <sebastian.meiling (at) haw-hamburg.de>\n\n");
}

int pidof(const char* process_name){
    char cmd[50] ="pidof ";

    // add process name
    strncat(cmd,process_name, sizeof(cmd) - strlen(cmd) -1);
    FILE* fpidof = popen(cmd, "r");
    char spid[10];
    if(fpidof == NULL){
        perror("popen error");
        return -1;
    }
    if(fscanf(fpidof, "%s", spid) == EOF)
        return -1;
    pclose(fpidof);

    return atoi(spid);
}

//return 0 on success, -1 on error
int get_usage(const pid_t pid, struct pstat* result){
    //convert  pid to string
    char pid_str[20];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    char stat_filepath[30] = "/proc/";
    strncat(stat_filepath, pid_str, sizeof(stat_filepath) - strlen(stat_filepath) -1);
    strncat(stat_filepath, "/stat", sizeof(stat_filepath) - strlen(stat_filepath) -1);

    //open /proc/pid/stat
    FILE *fpstat = fopen(stat_filepath, "r");
    if(fpstat == NULL){
        printf("FOPEN ERROR pid stat %s:\n", stat_filepath);
        return -1;
    }

    //open /proc/stat
    FILE *fstat = fopen("/proc/stat", "r");
    if(fstat == NULL){
        printf("FOPEN ERROR");
        fclose(fstat);
        return -1;
    }
    bzero(result, sizeof(struct pstat));

    //read values from /proc/pid/stat
    if(fscanf(fpstat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %ld %ld", &result->utime_ticks, &result->stime_ticks, &result->cutime_ticks, &result->cstime_ticks) == EOF){
        fclose(fpstat);
        fclose(fstat);
        return -1;
    }
    fclose(fpstat);

    //read+calc cpu total time from /proc/stat, on linux 2.6.35-23 x86_64 the cpu row has 10values could differ on different architectures :/
    long unsigned int cpu_time[10];
    bzero(cpu_time, sizeof(cpu_time));
    if(fscanf(fstat, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", &cpu_time[0], &cpu_time[1], &cpu_time[2], &cpu_time[3], &cpu_time[4], &cpu_time[5], &cpu_time[6], &cpu_time[7], &cpu_time[8], &cpu_time[9]) == EOF){
        fclose(fstat);
        return -1;
    }
    fclose(fstat);

    for(int i=0; i < 10;i++){
        result->cpu_total_time += cpu_time[i];
    }

    return 0;
}

/*
* calculates the actual CPU usage(cur_usage - last_usage) in percent
* cur_usage, last_usage: both last measured get_usage() results
* ucpu_usage, scpu_usage: result parameters: user and sys cpu usage in %
*/
void calc_cpu_usage(struct pstat* cur_usage, struct pstat* last_usage, float* ucpu_usage, float* scpu_usage){
    *ucpu_usage = 100 * ((((cur_usage->utime_ticks + cur_usage->cutime_ticks) - (last_usage->utime_ticks + last_usage->cutime_ticks))) / (float)((cur_usage->cpu_total_time - last_usage->cpu_total_time)));
    *scpu_usage = 100 * ((((cur_usage->stime_ticks + cur_usage->cstime_ticks) - (last_usage->stime_ticks + last_usage->cstime_ticks))) / (float)((cur_usage->cpu_total_time - last_usage->cpu_total_time)));
}

int main (int argc, char **argv)
{
    int pid_middleware = 0;
    int pid_application = 0;
    int runtime = 0;
    /* parse options */
    while (1) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "ha:m:t:", 
                             long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 0: {
                if (long_options[option_index].flag != 0)
                    break;
                break;
            }
            case 'a': {
                pid_application = pidof(optarg);
                if (pid_application < 1) {
                    fprintf (stderr, "Application process not found!");
                    return EXIT_FAILURE;
                }
                break;
            }
            case 'm': {
                pid_middleware = pidof(optarg);
                if (pid_application < 1) {
                    fprintf (stderr, "Middleware process not found!");
                    return EXIT_FAILURE;
                }
                break;
            }
            case 't': {
                runtime = atoi(optarg);
                if (runtime < 1) {
                    fprintf (stderr, "Invalid time argument, cannot convert!");
                    return EXIT_FAILURE;
                }
                break;
            }
            case 'h': {
                help (argv[0]);
                return EXIT_SUCCESS;
                break;
            }
            default: {
                usage (argv[0]);
                return EXIT_FAILURE;
                break;
            }
        } // switch
    } // while

    if ((pid_middleware == 0) && (pid_application == 0)) {
        fprintf (stderr, "No process name given!\n");
        return EXIT_FAILURE;
    }
    // trace CPU usage

    printf ("[ DONE ]\n");

    return EXIT_SUCCESS;
}
