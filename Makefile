# project configuration
TARGET := stm32f413rht

# directory for Embedded-Sharepoint submodule
SUBMODULE_DIR := Embedded-Sharepoint
BUILD_DIR := build
SUB_MAKEFILE_DIR := Test

all: embedded-sharepoint

embedded-sharepoint: 
	@echo "Building Embedded-Sharepoint submodule"	
	$(MAKE) -C $(SUBMODULE_DIR)/$(SUB_MAKEFILE_DIR) PROJECT_TARGET=$(TARGET)

clean:
	@echo "cleaning Embedded-Sharepoint/build directory"
	-rm -rf $(SUBMODULE_DIR)/$(BUILD_DIR)
# $(MAKE) -C $(SUBMODULE_DIR)/$(BUILD_DIR) clean