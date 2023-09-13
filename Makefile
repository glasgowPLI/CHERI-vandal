LIBNAME = libvandal.so
CC = clang-morello -march=morello+c64 -mabi=purecap
CFLAGS = -fPIC -Wall -pedantic -g
LDFLAGS = -shared -ffreestanding -Wl,-soname,$(LIBNAME)

$(LIBNAME) : vandal.o vandal-backend.o
	$(CC) $(LDFLAGS) -o $@ $^

vandal.o : vandal.s
	$(CC) $(CFLAGS) -c $< -o $@

vandal-backend.o : vandal-backend.c setjmpstatic.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(LIBNAME) *.o

