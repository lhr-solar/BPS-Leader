
# crazy colors ------------------
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
#---------------------------------


# Project Configuration ----------
TEST ?= main
JOBS ?= 8

$(info Build is configured to use $(JOBS) jobs)


# LSOM
####
PROJECT_TARGET ?= stm32g473xx
####


####
# source and include directories
PROJECT_C_INCLUDES = $(wildcard */Inc)
PROJECT_C_SOURCES = $(wildcard */Src/*.c)
####

# build directories
PROJECT_BUILD_DIR = build


####
BUILD_MAKEFILE_DIR ?= Embedded-Sharepoint
####

# generate paths
MAKEFILE_DIR = $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))


####
PROJECT_MAIN_DIR ?= Core/Src/main
####

ifneq ($(TEST), main)
PROJECT_C_SOURCES := $(filter-out $(PROJECT_MAIN_DIR).c, $(PROJECT_C_SOURCES))


####
PROJECT_C_SOURCES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_SOURCES) Tests/$(TEST).c)
####
else
PROJECT_C_SOURCES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_SOURCES))
endif

PRINT_DEBUG ?= false

# debug
ifeq ($(PRINT_DEBUG), true)
$(info SOURCES: $(PROJECT_C_SOURCES))
$(info INCLUDES: $(PROJECT_C_INCLUDES))
endif

# generate paths (cont.d)
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

# define path of .vscode (create if missing)
VS_CODE_DIR := $(MAKEFILE_DIR)/.vscode
$(shell mkdir -p $(VS_CODE_DIR))

ifeq ($(BEAR_INSTALLED),yes)
BEAR_PREFIX := bear --output $(VS_CODE_DIR)/compile_commands.json --append --
endif


# Build --------------------------
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
	@echo -e "Making ${PURPLE}$(PROJECT_TARGET)${NC}build for ${BLUE}TEST=${PURPLE} ${TEST}${NC}"
else
	@echo -e "Making ${PURPLE}$(PROJECT_TARGET)${NC}build with ${ORANGE}no test.${NC}"
endif
	$(BEAR_PREFIX) $(MAKE) -C $(BUILD_MAKEFILE_DIR) all -j$(JOBS)
	@echo -e "${GREEN}Compiled! Splendid! Jolly Good!!${NC}"


#-------------------------------
# Flash
flash: 	
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) flash

#-------------------------------
# Flash via UART
flash-uart: 	
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) flash-uart

#-------------------------------
# OpenOCD debug
debug: 	
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) debug

#-------------- 
# Documentation
# .PHONY: docs
# docs:
# 	cd $(BUILD_MAKEFILE_DIR) && mkdocs serve