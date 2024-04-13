leader:
	$(MAKE) -C Embedded-Sharepoint -C BSP -C STM32F413 TARGET=bsp PROJECT_DIR=../.. BUILD_DIR=../../../Objects
flash:
	-st-flash write Objects/bsp.bin 0x8000000

clean:
	-rm -fR Objects