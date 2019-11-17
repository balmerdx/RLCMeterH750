RM := rm -rf
TC=${ARM_NONE_EABI}/arm-none-eabi
CC=$(TC)-gcc
LD=$(TC)-ld -v
OBJCOPY=$(TC)-objcopy
AR=$(TC)-ar
GDB=$(TC)-gdb

STM_LIB_DIR = "Drivers"
C_INCLIDES= \
-I$(STM_LIB_DIR)/STM32H7xx_HAL_Driver/Inc \
-I$(STM_LIB_DIR)/STM32H7xx_HAL_Driver/Inc/Legacy \
-I$(STM_LIB_DIR)/CMSIS/Include \
-I$(STM_LIB_DIR)/CMSIS/Device/ST/STM32H7xx/Include \
-ISrc \
-IInc \
-IMiddlewares/ST/STM32_USB_Device_Library/Core/Inc \
-IMiddlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc

C_INCLIDES+= -includeDrivers/STM32H7xx_HAL_Driver/Inc/stm32h7xx_hal.h

M_CPU = -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard
LDFLAGS = $(M_CPU) -specs=nano.specs

ifeq ($(MAKECMDGOALS),debug)
C_DEFINES= -DDEBUG
C_FLAGS = -O0 -g -gdwarf-2
endif

ifeq ($(MAKECMDGOALS),release)
C_DEFINES= -DNDEBUG
C_FLAGS = -O3 -g -gdwarf-2
endif

C_FLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -Wl,--gc-sections 

#C_FLAGS += -std=gnu11
#C_FLAGS += -funsigned-char
C_DEFINES+= \
-DUSE_HAL_DRIVER \
-DSTM32H750xx \
-DHSE_VALUE=25000000

C_FLAGS+= -Wall -c -MMD $(C_INCLIDES) $(C_DEFINES) $(M_CPU)
#C_FLAGS+= -g -Wa,-adhls #asm listing
#C_FLAGS+= -S

LIBS = -lc -lm -lnosys
#LDFLAGS += -TSTM32H743ZITx_FLASH.ld  $(LIBS)  -Wl,-Map=output/linker.map,--cref
LDFLAGS += -TSTM32H750VBTx_FLASH.ld  $(LIBS)  -Wl,-Map=output/linker.map,--cref
ASM_FLAGS = -x assembler-with-cpp -c $(M_CPU)

TARGET=output/program

# All of the sources participating in the build are defined here
include sources.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

# All Target
#all: executable

release: executable

debug: executable

# Tool invocations
executable: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC Linker'
	@$(CC) $(OBJS) $(USER_OBJS) $(LDFLAGS) -o "$(TARGET)_$(MAKECMDGOALS).elf"
	@echo 'Finished building target: $@'
	@echo 'Create Binary'
	@$(OBJCOPY) -S  -O binary  "$(TARGET)_$(MAKECMDGOALS).elf" "$(TARGET)_$(MAKECMDGOALS).bin"
	@echo ' '

# Other Targets
clean:
	-@echo 'Clean files'	
	-@$(RM) $(OBJS)$(DEPS) $(TARGET)
	-@echo ' '
	-@$(RM) linker.map

flashd:
	python ~/radio/stm32/tools/stm32_flash_0.py "${CURDIR}/$(TARGET)_debug.bin"

flashr:
	python ~/radio/stm32/tools/stm32_flash_0.py "${CURDIR}/$(TARGET)_release.bin"

.PHONY: all clean dependents
