
IPERF_SRC = middleware/MTK/sd_cli

C_FILES  += 	$(IPERF_SRC)/src/sd_cli.c
			 

#################################################################################
#include path
CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/sd_cli/inc
CFLAGS	+= -I$(SOURCE_DIR)/kernel/rtos/FreeRTOS/Source/include 
CFLAGS  += -I$(SOURCE_DIR)/kernel/rtos/FreeRTOS/Source/portable/GCC/ARM_CM4F
CFLAGS  += -I$(SOURCE_DIR)/driver/chip/mt$(PRODUCT_VERSION)/inc
CFLAGS  += -I$(SOURCE_DIR)/middleware/MTK/connsys/inc
