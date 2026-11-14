CC = gcc
AR = ar
CFLAGS = -Wall -Wextra
LDFLAGS =

# задание 2
CLIENT = client
SERVER = server
CLIENT_SRC = client.c
SERVER_SRC = server.c

# задание 3
NEW_CLIENT = new_client
NEW_SERVER = new_server
NEW_CLIENT_SRC = new_client.c
NEW_SERVER_SRC = new_server.c
NEW_CLIENT_OBJ = $(NEW_CLIENT_SRC:.c=.o)
NEW_SERVER_OBJ = $(NEW_SERVER_SRC:.c=.o)

LIB = libcommon.a
LIB_SRC = common.c
LIB_OBJ = $(LIB_SRC:.c=.o)
LIB_HDR = common.h

.PHONY: all all-lab2 all-lab3 clean

all: all-lab2 all-lab3

all-lab2: $(CLIENT) $(SERVER)

all-lab3: $(NEW_CLIENT) $(NEW_SERVER)

# задание 2
$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -pthread

$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -pthread

# задание 3
$(LIB): $(LIB_OBJ)
	$(AR) rcs $@ $^

$(LIB_OBJ): $(LIB_SRC) $(LIB_HDR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(NEW_CLIENT_OBJ): $(NEW_CLIENT_SRC) $(LIB_HDR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(NEW_SERVER_OBJ): $(NEW_SERVER_SRC) $(LIB_HDR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(NEW_CLIENT): $(NEW_CLIENT_OBJ) $(LIB)
	$(CC) $(CFLAGS) -o $@ $(NEW_CLIENT_OBJ) $(LIB) $(LDFLAGS) -pthread

$(NEW_SERVER): $(NEW_SERVER_OBJ) $(LIB)
	$(CC) $(CFLAGS) -o $@ $(NEW_SERVER_OBJ) $(LIB) $(LDFLAGS) -pthread

clean:
	rm -f $(CLIENT) $(SERVER) \
	      $(NEW_CLIENT) $(NEW_SERVER) \
	      $(NEW_CLIENT_OBJ) $(NEW_SERVER_OBJ) \
	      $(LIB_OBJ) $(LIB)