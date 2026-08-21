TARGET := notepad2
CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion
CFLAGS += -std=c17 -g3 -MMD -MP #-fsanitize=address
SRC_DIR := src/
BUILD_DIR := build
SRC := $(SRC_DIR)main.c $(SRC_DIR)3rd.c
OBJ := $(patsubst $(SRC_DIR)%.c,$(BUILD_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)
INC := inc

ifeq ($(OS),Windows_NT)
	FLAGS := -lgdi32 -lm
	TARGET := $(TARGET).exe
	RUN := $(TARGET)
else
	CC := gcc
	FLAGS := -lX11 -lXrandr -lm #-fsanitize=address 
	TARGET := $(TARGET).bin
	RUN := ./$(TARGET)
endif

# Link
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(FLAGS)

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -isystem $(INC) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(DEP)

run: $(TARGET)
	$(RUN)
rundebug:$(TARGET)
	gdb -ex run  $(RUN) --tui
clean:
	rm -rf $(BUILD_DIR) $(TARGET)