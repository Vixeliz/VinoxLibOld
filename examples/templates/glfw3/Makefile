CFLAGS = -Wall -Os

INCLUDES = -Iinclude -Iexternal

LIBS = -lvinox -lGLESv2 -lEGL -lglfw

SRCS = main.c

OBJS = $(SRCS:.c=.o)

MAIN = glfw3

.PHONY: depend clean

all: $(MAIN)
	@echo Glfw3 has successfully compiled

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(OBJS) *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

#END
