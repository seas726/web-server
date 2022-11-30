
SERVER_DIR := tcp_server
CLIENT_DIR := tcp_client
SRC_DIR := $(SERVER_DIR)/src
OBJ_DIR := $(SERVER_DIR)/obj
BIN_DIR := .

EXE := $(BIN_DIR)/server
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CC := gcc
CPPFLAGS := -Itcp_server/include -MMD -MP -pthread
CFLAGS   := -Wall
LDFLAGS  := -Llib -pthread
LDLIBS   := -lm

.PHONY: all clean

all: $(EXE) client

client: $(CLIENT_DIR)/tcp_client.c | $(BIN_DIR)
	$(CC) -o client $(CLIENT_DIR)/tcp_client.c

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
	
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
	
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@
	
clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
