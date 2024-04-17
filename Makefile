leader:
	$(MAKE) -C Embedded-Sharepoint/BSP/STM32F413 \
			TARGET=bps-leader \
			PROJECT_DIR=../../.. \
			BUILD_DIR=../../../Objects \
			PROJECT_C_SOURCES="../../../Apps/Src/*.c \
								../../../Drivers/Src/*.c" \
			PROJECT_C_INCLUDES="../../../Apps/Inc \
								../../../Drivers/Inc"
test:
ifdef TEST
	$(MAKE) -C Embedded-Sharepoint/BSP/STM32F413 \
				TARGET=bps-leader \
				PROJECT_DIR=../../.. \
				BUILD_DIR=../../../Objects \
				PROJECT_C_SOURCES="../../../Apps/Src/*.c \
									../../../Drivers/Src/*.c"
				PROJECT_C_INCLUDES="../../../Apps/Inc \
									../../../Drivers/Inc" \
				TEST=../../../Tests/Test_$(test).c
else
	$(error test is not set (e.g. make test test=HelloWorld))
endif

flash:
	-st-flash write Objects/bsp.bin 0x8000000

clean:
	-rm -fR Objects