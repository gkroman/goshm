CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2
BUILD_DIR = build
TARGETS = server client-c client-go
BINARIES = $(addprefix $(BUILD_DIR)/, $(TARGETS))

all: $(BUILD_DIR) $(BINARIES)

$(BUILD_DIR):
		mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/server: server.c
		$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/client-c: client.c
		$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/client-go: client.go
		go build -o $@ $<

clean:
		rm -rf $(BUILD_DIR)

.PHONY: all clean

