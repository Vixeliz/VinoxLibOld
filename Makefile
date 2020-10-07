CFLAGS = -Wall -g -fPIC

INCLUDES = -Iinclude -Iexternal

LIBS = -lGLESv2 -lEGL -lcglm

SRCS = src/shader.c src/vingl.c 

PREFIX = /usr/local

OBJS = $(SRCS:.c=.o)

MAIN = vinox

.PHONY: depend uninstall install clean

all: $(MAIN)
	@echo LibVinox has successfully compiled

$(MAIN): $(OBJS)
	gcc -fPIC -shared $(OBJS) -o libvinox.so
	ar -rcs libvinox.a $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

install:
	mkdir -p $(DESTDIR)$(PREFIX)/lib
#	cp libvinox.a $(DESTDIR)$(PREFIX)/lib
	cp libvinox.so $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/include/vinox
	cp include/* $(DESTDIR)$(PREFIX)/include/vinox

uninstall:
	rm -rf $(DESTDIR)$(PREFIX)/lib/libvinox.a
	rm -rf $(DESTDIR)$(PREFIX)/lib/libvinox.so
	rm -rf $(DESTDIR)$(PREFIX)/include/vinox

clean:
	$(RM) $(OBJS) libvinox.a libvinox.so

depend: $(SRCS)
	makedepend $(INCLUDES) $^

#END
