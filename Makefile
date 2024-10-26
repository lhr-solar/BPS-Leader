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

# Project Configuration
TEST ?= main
PROJECT_TARGET ?= stm32f413rht
# PROJECT_TARGET ?= stm32f446ret

# source and include directories
PROJECT_C_SOURCES = $(wildcard Src/*.c)
# PROJECT_C_INCLUDES = $(wildcard */Inc)
PROJECT_C_INCLUDES = Inc

# build and driver directories
PROJECT_BUILD_DIR = Embedded-Sharepoint/build
BUILD_MAKEFILE_DIR = Embedded-Sharepoint

# path files
MAKEFILE_DIR = $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PROJECT_C_SOURCES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_SOURCES)/)

ifneq ($(TEST), main)
PROJECT_C_SOURCES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_SOURCES) Tests/$(TEST).c)
endif

PROJECT_C_INCLUDES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_INCLUDES))
PROJECT_BUILD_DIR := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_BUILD_DIR))

# export variables 
export PROJECT_TARGET
export PROJECT_C_SOURCES
export PROJECT_C_INCLUDES
export PROJECT_BUILD_DIR

# Build
ifeq ($(MAKECMDGOALS),)
default: build_code
else ifeq ($(MAKECMDGOALS), all)
all: build_code
else
%:
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) $(MAKECMDGOALS)
endif

build_code:
ifneq ($(TEST), main)
	@echo "Making STM32 build for ${BLUE}TEST=${PURPLE} ${TEST}${NC}"
else
	@echo "Making STM32 build with ${ORANGE}no test.${NC}"
endif
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) all

# Help
.PHONY: help
help:
	@echo "Make Format: ${ORANGE}make ${BLUE}TEST=${PURPLE}<Test name>${NC}"
	@echo "- Running ${ORANGE}make${NC}will compile the production code."
	@echo "- If you want to run a test, specify ${BLUE}TEST=${PURPLE}<Test name>${NC}"
	@echo "  with ${PURPLE}<Test name>${NC} being the exact name of the test file."


#--------------
# Documentation
# .PHONY: docs
# docs:
# 	cd $(BUILD_MAKEFILE_DIR) && mkdocs serve
