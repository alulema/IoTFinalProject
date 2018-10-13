CC=gcc
CCFLAGS=
INCLUDES=
LFLAGS=-L/usr/lib/x86_64-linux-gnu
LIBS=-lcurl -lpthread

SRC=main.c tc_state.c tc_error.c tc_config.c tc_httpclient.c tc_json.c
OBJ=$(SRC:.c=.o)
MAIN=main_controller_x86

RM=rm -rf

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(MAIN): $(OBJ)
	$(CC) $(CCFLAGS) $(INCLUDES) -o $(MAIN) $(OBJ) $(LFLAGS) $(LIBS)

all: $(MAIN)

clean:
	$(RM) $(MAIN) *.o *~