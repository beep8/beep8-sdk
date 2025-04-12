# Define the name of the tool
TOOL_NAME = relb8rom

# Define the source file
SRC = main.cpp

# Define the output directories for each platform
WIN_DIR = Windows_NT/x86_64
LINUX_DIR = linux/x86_64
OSX_DIR_X86 = osx/x86_64
OSX_DIR_ARM = osx/arm64

# Detect the platform and set the compiler and flags
ifeq ($(OS), Windows_NT)
	PLATFORM = windows
	OUTPUT_DIR = $(WIN_DIR)
	OUTPUT = $(OUTPUT_DIR)/$(TOOL_NAME).exe
	CC = x86_64-w64-mingw32-g++
	CFLAGS = -Wall -static -std=c++17
	LDFLAGS = -static
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		PLATFORM = linux
		OUTPUT_DIR = $(LINUX_DIR)
		OUTPUT = $(OUTPUT_DIR)/$(TOOL_NAME)
		CC = g++
		CFLAGS = -Wall -static -std=c++17
		LDFLAGS = -static
	endif
	ifeq ($(UNAME_S), Darwin)
		ARCH := $(shell uname -m)
		ifeq ($(ARCH), x86_64)
			PLATFORM = osx_x86_64
			OUTPUT_DIR = $(OSX_DIR_X86)
			OUTPUT = $(OUTPUT_DIR)/$(TOOL_NAME)
			CC = g++
			CFLAGS = -Wall -std=c++17
			LDFLAGS =
		endif
		ifeq ($(ARCH), arm64)
			PLATFORM = osx_arm64
			OUTPUT_DIR = $(OSX_DIR_ARM)
			OUTPUT = $(OUTPUT_DIR)/$(TOOL_NAME)
			CC = g++
			CFLAGS = -Wall -std=c++17
			LDFLAGS =
		endif
	endif
endif

# Create the output directories if they don't exist
$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

.DEFAULT_GOAL := $(OUTPUT)

# The target to build the tool
$(OUTPUT): $(SRC) | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Clean up
clean:
	rm -f *.o
	rm -f *.tmp
	touch $(SRC)

distclean: clean
	rm -f $(WIN_DIR)/$(TOOL_NAME).exe
	rm -f $(LINUX_DIR)/$(TOOL_NAME)
	rm -f $(OSX_DIR_X86)/$(TOOL_NAME)
	rm -f $(OSX_DIR_ARM)/$(TOOL_NAME)

.PHONY: all clean