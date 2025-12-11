include src/metadata

SHELL = /bin/bash
.DEFAULT_GOAL = all

# Compiler and flags
CC = gcc
CFLAGS = -fPIC -Werror -Wall -Wextra -O2 -Isrc \
		 -Dmalloc=__forbidden_malloc \
		 -Dcalloc=__forbidden_calloc \
		 -Drealloc=__forbidden_realloc \
		 -Dfree=__forbidden_free

LDFLAGS = -shared

# Directories
SRC_DIR = src
BUILD_DIR = build

# Output targets
TARGET = $(BUILD_DIR)/libospager.so

# Source files
SRC = $(wildcard $(SRC_DIR)/*.c)
HDR = $(wildcard $(SRC_DIR)/*.h)

# Object files
OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))

# may be used f.ex. "--gtest_list_tests" "--gtest_filter=" or multiple
DEBUG ?= 

ifeq ($(shell [[ "$(AIS_ID)" != "" ]] && [[ "$(AIS_LOGIN)" != "" ]] && echo true),true)
# Default target: build app (which depends on the library)
.PHONY: all
all: test

# Create zip
.PHONY: zip
zip: $(BUILD_DIR)
	@echo "Creating: $(BUILD_DIR)/$(AIS_LOGIN)_$(AIS_ID).zip"
	@rm -f *.zip
	@zip -r "$(BUILD_DIR)/$(AIS_LOGIN)_$(AIS_ID).zip" src

else
.PHONY: all
all:
	@less README.md
	@echo "fill your metadata to src/setup file"

.PHONY: zip
zip:
	@echo "fill your metadata to src/setup file"

endif

.PHONY: help
help:
	@echo "make     - build project and run tests"
	@echo "make all - build project and run tests"
	@echo "make zip - creates build/<AIS_LOGIN>_<AIS_ID>.zip file that may be copied to ais"
	@echo "make clean - cleans build products"
	@echo ""
	@echo "make DEBUG=\"...\" - runs tests with extra parameters f.ex."
	@echo ""
	@echo "    make DEBUG=\"--gtest_list_tests\""
	@echo "    make DEBUG=\"--gtest_filter=InitRamTest*\""
	@echo "    make DEBUG=\"-d\""

.PHONY: test
test: $(TARGET)
	@LD_LIBRARY_PATH=$(BUILD_DIR) /public/zadania/os $(DEBUG)

# Build the shared library
$(TARGET): $(OBJ) | $(BUILD_DIR)
	@echo Linking $@
	@$(CC) $(LDFLAGS) -o $@ $^

# Compile library source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HDR) | $(BUILD_DIR)
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	@echo Creating $@
	@mkdir -p $(BUILD_DIR)

# Clean all build artifacts
.PHONY: clean
clean:
	@echo Deleting $(BUILD_DIR)
	@rm -rf $(BUILD_DIR)
