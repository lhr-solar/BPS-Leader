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
# PROJECT_TARGET ?= stm32f413rht; 	f446ret
TARGET ?= f413rht
PROJECT_TARGET ?= $(addprefix stm32, $(TARGET))
$(info PROJECT TARGET: $(PROJECT_TARGET))

# source and include directories
PROJECT_C_SOURCES = $(wildcard */Src/*.c)
PROJECT_C_INCLUDES = $(wildcard */Inc)

# debug: print project target & current sources/inc directories
PRINT_DEBUGS ?= false
ifeq ($(PRINT_DEBUGS), true)

$(info SOURCES: $(PROJECT_C_SOURCES))
$(info INCLUDES: $(PROJECT_C_INCLUDES))
endif

# build directories
PROJECT_BUILD_DIR = Embedded-Sharepoint/build
BUILD_MAKEFILE_DIR = Embedded-Sharepoint

# path files
MAKEFILE_DIR = $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
ifneq ($(TEST), main)
PROJECT_C_SOURCES := $(filter-out Apps/Src/main.c, $(PROJECT_C_SOURCES))
PROJECT_C_SOURCES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_SOURCES) Tests/$(TEST).c)
else
PROJECT_C_SOURCES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_SOURCES))
endif

# debug: print modified sources / include directories
ifeq ($(PRINT_DEBUGS), true)
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
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) $(MAKECMDGOALS)
	@echo "${BLUE}Compiled for BPS-Leader! Splendid! Jolly Good!!${NC}"
#-------------------------------

# Help
.PHONY: help
help:
	@echo "Format: ${ORANGE}make  ${GREEN}TARGET=${PURPLE}<stm-target>${NC} ${BLUE}TEST=${PURPLE}<Test name>${NC}"
	@echo "- Running ${ORANGE}make${NC}by itself will compile the production code"
	@echo "  (same as running ${ORANGE}make${NC}all). \n"

	@echo "${GREEN}TARGET:${NC}"
	@echo "- Specify stm32 target, including identifier after 'stm32'."
	@echo "- e.g. building for stm32${PURPLE}f413rht${NC} would mean specifying ${GREEN}TARGET=${PURPLE}f413rht.${NC} \n"

	@echo "${BLUE}TEST:${NC}"
	@echo "- If you want to run a test, specify ${BLUE}TEST=${PURPLE}<Test name>${NC}, with ${PURPLE}<Test name>${NC}"
	@echo "   being the exact name of the test file ${RED} without${NC} the .c suffix.\n"
	
	@echo "PRINT_DEBUGS:"
	@echo "- For debugs, specify ${BLUE}PRINT_DEBUGS=${PURPLE}true${NC}."
	@echo "- For now, this will print the directories that will be compiled (can be useful for troubleshooting)."

#-------------- 
# Documentation
# .PHONY: docs
# docs:
# 	cd $(BUILD_MAKEFILE_DIR) && mkdocs serve
