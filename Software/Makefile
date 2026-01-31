# default project configuration
PROJECT_TARGET ?= stm32g474xx
BEAR_ENABLE ?= 1	
VERBOSE ?= 0




# --- Test Configuration Logic ---
# Move this high up so it's easy to see
ifeq ($(strip $(TEST)),)
    # Standard Build: Include production main
    SELECTED_TEST_PATH := 
    _PROJECT_C_SOURCES := $(wildcard Apps/Src/*.c) $(wildcard Drivers/Src/*.c) $(wildcard Tasks/Src/*.c)
else
    # Test Build: Filter out production main and add the specific test
    SELECTED_TEST_PATH := Tests/Src/$(strip $(TEST))
    # We find all sources EXCEPT main.c
    _CORE_SOURCES := $(wildcard Apps/Src/*.c) $(wildcard Drivers/Src/*.c) $(wildcard Tasks/Src/*.c)
    _NO_MAIN_SOURCES := $(filter-out %main.c, $(_CORE_SOURCES))
    _PROJECT_C_SOURCES := $(_NO_MAIN_SOURCES) $(SELECTED_TEST_PATH)
endif

# build and driver directories
_PROJECT_BUILD_DIR ?= build	
BUILD_MAKEFILE_DIR = Embedded-Sharepoint

# ensure all paths are absolute
MAKEFILE_DIR = $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PROJECT_C_SOURCES = $(addprefix $(MAKEFILE_DIR)/, $(_PROJECT_C_SOURCES)) 
PROJECT_C_INCLUDES = $(addprefix $(MAKEFILE_DIR)/, $(_PROJECT_C_INCLUDES))
PROJECT_BUILD_DIR = $(addprefix $(MAKEFILE_DIR)/, $(_PROJECT_BUILD_DIR))

_PROJECT_C_INCLUDES = Apps/Inc Drivers/Inc Tasks/Inc 


export PROJECT_TARGET
export PROJECT_C_SOURCES
export PROJECT_C_INCLUDES
export PROJECT_BUILD_DIR
export BEAR_ENABLE
export VERBOSE


ifeq ($(BEAR_ENABLE), 1)
	COMPILE_COMMANDS_JSON := $(PROJECT_BUILD_DIR)/compile_commands.json
endif

$(info $(PROJECT_BUILD_DIR))

default: all
.PHONY: all
all:
	@echo "DEBUG: Project Sources found: $(PROJECT_C_SOURCES)"
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) all

ifeq ($(BEAR_ENABLE), 1)
ifeq ($(VERBOSE), 1)
	$(BUILD_MAKEFILE_DIR)/bear2.py $(PROJECT_BUILD_DIR) $(COMPILE_COMMANDS_JSON)
else
	@echo "Generating compile commands..."
	@$(BUILD_MAKEFILE_DIR)/bear2.py $(PROJECT_BUILD_DIR) $(COMPILE_COMMANDS_JSON)
endif
endif

.PHONY: clean
clean:
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) clean

# generate documentation
.PHONY: docs
docs:
	cd .. && mkdocs serve

.PHONY: flash
flash:
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) flash

# help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  docs     - Generate documentation using mkdocs."
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) help

