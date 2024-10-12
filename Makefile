# project configuration
# TEST := main
TARGET := stm32f413rht

# directory for Embedded-Sharepoint submodule
SUBMODULE_DIR := Embedded-Sharepoint
BUILD_DIR := Test

all: embedded-sharepoint

embedded-sharepoint: 
	@echo "Building Embedded-Sharepoint submodule"	
	$(MAKE) -C $(SUBMODULE_DIR)/$(BUILD_DIR) \
				PROJECT_TARGET = $(TARGET) 
				


# clean:
#     rm -rf $()