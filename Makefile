CFLAGS = -Wall -g

INCLUDES = -Iinclude -Iexternal

LIBS = -lGLESv2 -lEGL -lcglm

SRCS = src/shader.c src/shapes.c src/vingl.c 

PREFIX = /usr/local

OBJS = $(SRCS:.c=.o)

MAIN = vinox

.PHONY: depend uninstall install clean

all: $(MAIN)
	@echo LibVinox has successfully compiled

$(MAIN): $(OBJS)
	ar -rcs libvinox.a $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

install:
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp libvinox.a $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/include/vinox
	cp include/* $(DESTDIR)$(PREFIX)/include/vinox

uninstall:
	rm -rf $(DESTDIR)$(PREFIX)/lib/libvinox.a
	rm -rf $(DESTDIR)$(PREFIX)/include/vinox

clean:
	$(RM) $(OBJS) libvinox.a

depend: $(SRCS)
	makedepend $(INCLUDES) $^

#END
