CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -g
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
EXECUTABLE = $(BIN_DIR)/twitch_irc_client

C_SRCS = $(wildcard $(SRC_DIR)/*.c)
C_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))

all: setup $(EXECUTABLE)

setup:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BIN_DIR)

$(EXECUTABLE): $(C_OBJS)
	$(CC) $(CFLAGS) $(C_OBJS) -o $(EXECUTABLE) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

debug: CFLAGS += -DDEBUG -g3 -O0
debug: all

release: CFLAGS += -O3 -DNDEBUG
release: all

.PHONY: all setup clean debug release
