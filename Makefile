leader:
	$(MAKE) -C Embedded-Sharepoint -C BSP -C STM32F413 TARGET=bps-leader PROJECT_DIR=../../.. BUILD_DIR=../../../Objects

test:
ifdef TEST
	$(MAKE) -C Embedded-Sharepoint -C BSP -C STM32F413 TARGET=bps-leader PROJECT_DIR=../../.. BUILD_DIR=../../../Objects TEST=../../../Tests/Test_$(test).c PROJECT_C_SOURCES=../../../Drivers/Src/*.c PROJECT_C_INCLUDES=../../../Drivers/Inc/*.h
else
	$(error test is not set (e.g. make test test=HelloWorld))
endif

flash:
	-st-flash write Objects/bsp.bin 0x8000000

clean:
	-rm -fR Objects