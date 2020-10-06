CFLAGS = -Wall -g

INCLUDES = -Iinclude -Iexternal

LIBS = -lGLESv2 -lEGL -lglfw -lcglm

SRCS = src/shader.c src/shapes.c src/vingl.c src/main.c

OBJS = $(SRCS:.c=.o)

MAIN = vinox

.PHONY: depend clean

all: $(MAIN)
	@echo Vinox has successfully compiled

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJS) *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

#END
