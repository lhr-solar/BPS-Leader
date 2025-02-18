# crazy colors
RED=\033[0;31m
GREEN=\033[0;32m
ORANGE=\033[0;33m
BLUE=\033[0;34m
PURPLE=\033[0;35m
CYAN=\033[0;36m
LIGHTGRAY=\033[0;37m
DARKGRAY=\033[1;30m
YELLOW=\033[0;33m
NC=\033[0m # No Color
#----------------------------


# Project Configuration
TEST ?= main
PROJECT_TARGET ?= stm32f413rht

# source and include directories
PROJECT_C_INCLUDES = $(wildcard */Inc)
PROJECT_C_SOURCES = $(wildcard */Src/*.c)

# build directories
PROJECT_BUILD_DIR = Build
BUILD_MAKEFILE_DIR = Embedded-Sharepoint

# path files
MAKEFILE_DIR = $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

PROJECT_MAIN_DIR ?= Apps/Src/main

ifneq ($(TEST), main)
PROJECT_C_SOURCES := $(filter-out $(PROJECT_MAIN_DIR).c, $(PROJECT_C_SOURCES))
PROJECT_C_SOURCES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_SOURCES) Tests/$(TEST).c)
else
PROJECT_C_SOURCES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_SOURCES))
endif

# debug
ifeq ($(PRINT_DEBUG), true)
$(info SOURCES: $(PROJECT_C_SOURCES))
$(info INCLUDES: $(PROJECT_C_INCLUDES))
endif

PROJECT_C_INCLUDES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_INCLUDES))
PROJECT_BUILD_DIR := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_BUILD_DIR))

# export variables 
export PROJECT_TARGET
export PROJECT_C_SOURCES
export PROJECT_C_INCLUDES
export PROJECT_BUILD_DIR


#-------------------------------
# Clang
BEAR_PREFIX := 
# check if bear is installed
BEAR_INSTALLED := $(shell command -v bear >/dev/null 2>&1 && echo yes || echo no)

# define path of .vscode
VS_CODE_DIR := $(realpath .vscode/)

ifeq ($(BEAR_INSTALLED),yes)
BEAR_PREFIX := bear --output $(VS_CODE_DIR)/compile_commands.json --append --
endif


#-------------------------------
# Build
ifeq ($(MAKECMDGOALS),)
default: build_code
else ifeq ($(MAKECMDGOALS), all)
all: build_code
else
%:
	$(BEAR_PREFIX) $(MAKE) -C $(BUILD_MAKEFILE_DIR) $(MAKECMDGOALS)
endif


build_code:
ifneq ($(TEST), main)
	@echo "Making STM32 build for ${BLUE}TEST=${PURPLE} ${TEST}${NC}"
else
	@echo "Making STM32 build with ${ORANGE}no test.${NC}"
endif
	$(BEAR_PREFIX) $(MAKE) -C $(BUILD_MAKEFILE_DIR) all -j
	@echo "${BLUE}Compiled for BPS-Leader! Splendid! Jolly Good!!${NC}"
#-------------------------------

#-------------------------------
# Flash
flash: 	
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) flash

# Help
.PHONY: help
help:
	@echo "Format: ${ORANGE}make ${BLUE}TEST=${PURPLE}<Test name>${NC}"
	@echo "- Running ${ORANGE}make${NC}by itself will compile the production code (same as running ${ORANGE}make${NC}all) \n"
	@echo "TEST:"
	@echo "- If you want to run a test, specify ${BLUE}TEST=${PURPLE}<Test name>${NC}, with ${PURPLE}<Test name>${NC}"
	@echo "   being the exact name of the test file ${RED} without${NC} the .c suffix.\n"
	@echo "PRINT_DEBUG:"
	@echo "- For debugs, specify ${BLUE}PRINT_DEBUG=${PURPLE}true${NC}."
	@echo "- For now, this will print the directories that will be compiled (can be useful for troubleshooting)."
