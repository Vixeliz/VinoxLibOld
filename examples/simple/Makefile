CFLAGS = -Wall -g

INCLUDES = -Iinclude -Iexternal

LIBS = -lvinox -lGLESv2 -lEGL -lcglm -lglfw

SRCS = main.c

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
