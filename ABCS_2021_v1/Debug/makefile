################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./main.obj" \
"./SatelliteAcceleration/Acceleration.obj" \
"./Scrubbing/BackupRom/BackupRom.obj" \
"./Scrubbing/PersistentRam/PersistentRam.obj" \
"./Scrubbing/Scrubbing.obj" \
"./SelfDiag/selfDiag.obj" \
"./bootloader/bootloader.obj" \
"./communications/communications.obj" \
"./communications/radio_lithium2.obj" \
"./memory/memory.obj" \
"./payload/labonchip.obj" \
"./payload/marie.obj" \
"./satellite/abcs_status.obj" \
"./satellite/configuration.obj" \
"./satellite/debug.obj" \
"./satellite/satsystem_init.obj" \
"./satellite/telemetry.obj" \
"./satellite/timers.obj" \
"../lnk_msp430f5438a.cmd" \
$(GEN_CMDS__FLAG) \
-llibmath.a \
-l"C:/Users/Manuel-desktop/Downloads/210504_ABACUS_Libraries_ABCS_2021_v1/ABACUS_Libraries/Debug/ABACUS_Libraries.lib" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include SatelliteAcceleration/subdir_vars.mk
-include Scrubbing/BackupRom/subdir_vars.mk
-include Scrubbing/PersistentRam/subdir_vars.mk
-include Scrubbing/subdir_vars.mk
-include SelfDiag/subdir_vars.mk
-include bootloader/subdir_vars.mk
-include communications/subdir_vars.mk
-include memory/subdir_vars.mk
-include payload/subdir_vars.mk
-include satellite/subdir_vars.mk
-include subdir_rules.mk
-include SatelliteAcceleration/subdir_rules.mk
-include Scrubbing/BackupRom/subdir_rules.mk
-include Scrubbing/PersistentRam/subdir_rules.mk
-include Scrubbing/subdir_rules.mk
-include SelfDiag/subdir_rules.mk
-include bootloader/subdir_rules.mk
-include communications/subdir_rules.mk
-include memory/subdir_rules.mk
-include payload/subdir_rules.mk
-include satellite/subdir_rules.mk
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
EXE_OUTPUTS += \
ABCS_2021_v1.out \

EXE_OUTPUTS__QUOTED += \
"ABCS_2021_v1.out" \


# All Target
all: ABCS_2021_v1.out

# Tool invocations
ABCS_2021_v1.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: MSP430 Linker'
	"C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/bin/cl430" -vmspx --abi=eabi --data_model=large -Ooff --advice:power="all" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal -z -m"ABCS_2021_v1.map" --heap_size=160 --stack_size=160 --use_hw_mpy=F5 -i"C:/ti/ccs1040/ccs/ccs_base/msp430/include" -i"C:/ti/ccs1040/ccs/ccs_base/msp430/lib/5xx_6xx_FRxx" -i"C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/lib" -i"C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/include" -i"C:/ti/ccs1040/ccs/ccs_base/msp430/lib" --reread_libs --warn_sections --display_error_number --diag_wrap=off --xml_link_info="ABCS_2021_v1_linkInfo.xml" --rom_model -o "ABCS_2021_v1.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)
	-$(RM) "main.obj" "SatelliteAcceleration\Acceleration.obj" "Scrubbing\BackupRom\BackupRom.obj" "Scrubbing\PersistentRam\PersistentRam.obj" "Scrubbing\Scrubbing.obj" "SelfDiag\selfDiag.obj" "bootloader\bootloader.obj" "communications\communications.obj" "communications\radio_lithium2.obj" "memory\memory.obj" "payload\labonchip.obj" "payload\marie.obj" "satellite\abcs_status.obj" "satellite\configuration.obj" "satellite\debug.obj" "satellite\satsystem_init.obj" "satellite\telemetry.obj" "satellite\timers.obj" 
	-$(RM) "main.d" "SatelliteAcceleration\Acceleration.d" "Scrubbing\BackupRom\BackupRom.d" "Scrubbing\PersistentRam\PersistentRam.d" "Scrubbing\Scrubbing.d" "SelfDiag\selfDiag.d" "bootloader\bootloader.d" "communications\communications.d" "communications\radio_lithium2.d" "memory\memory.d" "payload\labonchip.d" "payload\marie.d" "satellite\abcs_status.d" "satellite\configuration.d" "satellite\debug.d" "satellite\satsystem_init.d" "satellite\telemetry.d" "satellite\timers.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

