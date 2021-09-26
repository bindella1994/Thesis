################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./abacus.obj" \
"./abacus_P2Interrupts.obj" \
"./abacus_utils.obj" \
"./FPGA/abacus_fpga.obj" \
"./GPIO/abacus_gpio.obj" \
"./GPIO/abacus_gpio_analog.obj" \
"./GPIO/abacus_gpio_exp.obj" \
"./RTC/abacus_rtc.obj" \
"./RTC/abacus_rtc_internal.obj" \
"./communications/abacus_i2c.obj" \
"./communications/abacus_spi.obj" \
"./communications/abacus_uart.obj" \
"./memory/abacus_flash.obj" \
"./memory/abacus_flash_mcu.obj" \
"./sensors/abacus_accelerometer.obj" \
"./sensors/abacus_gyroscope.obj" \
"./sensors/abacus_magnetometer.obj" \
"./sensors/abacus_temperature.obj" \
"./timer/abacus_timer.obj" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include FPGA/subdir_vars.mk
-include GPIO/subdir_vars.mk
-include RTC/subdir_vars.mk
-include communications/subdir_vars.mk
-include memory/subdir_vars.mk
-include sensors/subdir_vars.mk
-include timer/subdir_vars.mk
-include subdir_rules.mk
-include FPGA/subdir_rules.mk
-include GPIO/subdir_rules.mk
-include RTC/subdir_rules.mk
-include communications/subdir_rules.mk
-include memory/subdir_rules.mk
-include sensors/subdir_rules.mk
-include timer/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LIB_OUTPUTS += \
ABACUS_Libraries.lib \

LIB_OUTPUTS__QUOTED += \
"ABACUS_Libraries.lib" \


# All Target
all: ABACUS_Libraries.lib

# Tool invocations
ABACUS_Libraries.lib: $(OBJS)
ifneq ($(strip $(ORDERED_OBJS)),)
	@echo 'Building target: "$@"'
	@echo 'Invoking: MSP430 Archiver'
	"C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/bin/ar430" r "ABACUS_Libraries.lib" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '
endif

# Other Targets
clean:
	-$(RM) $(LIB_OUTPUTS__QUOTED)
	-$(RM) "abacus.obj" "abacus_P2Interrupts.obj" "abacus_utils.obj" "FPGA\abacus_fpga.obj" "GPIO\abacus_gpio.obj" "GPIO\abacus_gpio_analog.obj" "GPIO\abacus_gpio_exp.obj" "RTC\abacus_rtc.obj" "RTC\abacus_rtc_internal.obj" "communications\abacus_i2c.obj" "communications\abacus_spi.obj" "communications\abacus_uart.obj" "memory\abacus_flash.obj" "memory\abacus_flash_mcu.obj" "sensors\abacus_accelerometer.obj" "sensors\abacus_gyroscope.obj" "sensors\abacus_magnetometer.obj" "sensors\abacus_temperature.obj" "timer\abacus_timer.obj" 
	-$(RM) "abacus.d" "abacus_P2Interrupts.d" "abacus_utils.d" "FPGA\abacus_fpga.d" "GPIO\abacus_gpio.d" "GPIO\abacus_gpio_analog.d" "GPIO\abacus_gpio_exp.d" "RTC\abacus_rtc.d" "RTC\abacus_rtc_internal.d" "communications\abacus_i2c.d" "communications\abacus_spi.d" "communications\abacus_uart.d" "memory\abacus_flash.d" "memory\abacus_flash_mcu.d" "sensors\abacus_accelerometer.d" "sensors\abacus_gyroscope.d" "sensors\abacus_magnetometer.d" "sensors\abacus_temperature.d" "timer\abacus_timer.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

