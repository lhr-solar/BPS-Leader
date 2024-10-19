# project configuration
TARGET = stm32f413rht
# STM32RHT = stm32f413rht
# STM32RET = stm32f446ret

# directory for Embedded-Sharepoint submodule
SUBMODULE_DIR = Embedded-Sharepoint
SHAREPOINT_MKFILE_DIR = test
BUILD_DIR = build

# TEST and TEST_FILE
TEST = none
export TEST
ifneq ($(TEST), none)
TEST_FILE := Test_$(TEST).c
endif


default: production_code

#######################################
# Production Code
#######################################
production_code: 
	@echo "build production code here"
ifneq ($(TEST), none)
	@echo "Making STM32 build for file ${TEST_FILE}"
else
	@echo "Making STM32 build with NO test."
endif
	@echo "Build Embedded-Sharepoint and BPS-Leader files"
# build BPS-Leader specific files using Embedded-Sharepoint
	$(MAKE) -C $(SUBMODULE_DIR)/$(SHAREPOINT_MKFILE_DIR) \
		PROJECT_TARGET=$(TARGET) \
		PROJECT_C_SOURCES=Apps/Src/*.c \
		PROJECT_C_INCLUDES=Apps/Inc
# build tests if needed



#######################################
# Help
#######################################
.PHONY: help
help:
	@echo "available targets"


#######################################
# Clean
#######################################
.PHONY: clean
clean:
	@echo "cleaning Embedded-Sharepoint/build directory"
	-rm -rf $(SUBMODULE_DIR)/$(BUILD_DIR)