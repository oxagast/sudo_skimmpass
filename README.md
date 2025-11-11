# sudo_skimmpass
## Hijacking sudo's call to read terminal's file descriptor to log user input

### Author
[oxagast](mailto:oxagast@oxasploits.com)

### Cautions
I take no, and by that I mean zero fucking responsiblity for what you might do with this code.  Also, this should *never*
be used in a production environment under any circumstances, it **will** make your server insecure, and quite possibly break
other things system-wide, because `/etc/ld.so.preload` acts on all binaries run on the system, not only sudo.  Run this
code in a VM or throwaway install, for research purposes only.

### Compiling
Compile using: `gcc -fPIC -shared -ldl -Wall -o /tmp/cap_pass.so sudo_lib_hook.c`

### Use
Then add the resulting shared library `/tmp/cap_pass.so` to the file `/etc/ld.so.preload`.
Note: *You will need to perform this action as root.*

You can now open a new terminal, and run `sudo su` and enter your passphrase.  You may
now log back out, and finally, check `/tmp/stolen.txt`.  It should now contain your
passphrase.

### Code Isolation
The abused function from sudo comes from the file `tgetpass.c` and the relevant call to
`read()` line ~393 in [commit 2a33699f8a0520161a8b507a9cb256802d6f45cb from sudo-project/sudo](https://github.com/sudo-project/sudo/blob/3d467a705ea6ee53081cb11cc21ecf08eb47700d/src/tgetpass.c#L393)
is:

```
  while (cp < ep) {
    nr = read(fd, &c, 1);
    if (nr != 1 || c == '\n' || c == '\r')
      break;
```

Where `nr = read(fd, &c, 1);` can be isolated and swapped out for our jacking library to read
and log entry from the terminal's file descriptor to a file.

### Greetz and Thankz
Vesteria<br>
blissful boy<br>
atdma<br>
