

#####
#####
##### The best makefile I have so far:
##### 	- Collects files
##### 	- Builds in parallel
#####  
##### 
#####
#####

# Multithread
	MAKEFLAGS += -j

## =============== COMPILE SECTION ================

	# Compiler
		CC := gcc

	# Static Analyzer
	#	CFLAGS += -fanalyzer

	# Version
		CFLAGS += -std=gnu99 -D_GNU_SOURCE

	# Debugging / Profiling
		CFLAGS += -gdwarf

	# Optimization
		DEBUG_CFLAGS = -D DEBUG -O0 
		RELEASE_CFLAGS = -D RELEASE -O3


	ifeq ($(RELEASE), 1)
		CFLAGS += $(RELEASE_CFLAGS)
		GLSLFLAGS += $(GLSL_RELEASE_FLAGS);
	else
		CFLAGS += $(DEBUG_CFLAGS)
		GLSLFLAGS += $(GLSL_DEBUG_FLAGS);
	endif

	# Include
		CFLAGS += -I/usr/include/freetype2

	# Warnings
		CFLAGS += -Wall -Wno-unused-function -Wno-builtin-declaration-mismatch -Wno-strict-aliasing -Wno-unused-variable -Wno-nonnull -Wno-address

	# Machine Dependent Options
		#CFLAGS += -mavx512f -mavx512dq -mavx512vl -mavx512bw
		CFLAGS += -march=native

## =============== COMPILE SECTION ================

## =============== LINKER SECTION =================

# Linker 
	LFLAGS += -fuse-ld=lld

# Debugging / Profiling 
	LFLAGS += -gdwarf -p -pg 

# Floating point
	#LFLAGS += -mlong-double-64
	#LFLAGS += -mlong-double-80
	LFLAGS += -mlong-double-128

# Performance Experimental
	# Use regular fp registers (80-bit intermediate) and sse (not 80-bit intermediate)
	# LFALGS += -mfpmath=both

# Pointer Metadata
	# LFLAGS += -mlam=u48

# Target Machine
	# TARGET THIS CPU
	#LFLAGS += -march=native

	# TARGET GENERIC X86-64
	LFLAGS += -march=x86-64

# Libraries 
	# Basic
	LFLAGS += -lm -lc

	# Threads
	LFLAGS += -lpthread -latomic

	# Window (X11: XShm Xpresent)
	LFLAGS += -lX11 -lXext -lXpresent    #-lXi -lXt 

	# Graphics (Vulkan)
	LFLAGS += -lvulkan

	# Raw User Input (evdev)
	LFLAGS += -levdev

	# Sound (asla)
	LFLAGS += -lasound

	# Other (freetype)
	LFLAGS += -lfreetype

## =============== LINKER SECTION =================

## =============== FILE SECTION ===================

# Collect all .h and .c files (requires at least one file to serch for: 'empty' is a placeholder)

# SRC_DIRS must have one other directory, where 'vulkan' is.
	SRC_DIRS := . empty 
	BUILD_DIR := build
	SRCS := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
	OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
	HEADERS := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.h))
	TARGET := e

	DIRS := $(foreach dir , $(SRC_DIRS),$(BUILD_DIR)/$(dir))
	DIRS := $(filter-out build/. , $(DIRS))

## =============== FILE SECTION ===================

	EMBED_DIR = ../bin/embed
	BIN_DIR= ../bin

	EMBEDS := $(wildcard dir, $(EMBED_DIR)/*.o)

## =============== RUN SECTION ===================

# Run the Build (Do not indent)

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET)


$(BUILD_DIR)/%.o: %.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


$(BUILD_DIR)/$(TARGET): $(OBJS) $(EMBEDS)
	$(CC) $(LFLAGS) $^ -o $@

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) -r $(EMBED_DIR)
	mkdir -p $(DIRS)
	mkdir -p $(EMBED_DIR)

## =============== RUN SECTION ===================












