################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
camera/%.obj: ../camera/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1000/ccs/tools/compiler/msp430_4.2.7/bin/cl430" -vmspx --abi=eabi --data_model=large -Ooff --include_path="C:/ti/ccs1000/ccs/ccs_base/msp430/include" --include_path="C:/Users/Augusto Nascetti/workspace_bootloader/ABCS_2021_v1" --include_path="C:/Users/Augusto Nascetti/Documents/ABACUS/BOOTLOADER/ABACUS_Libraries" --include_path="C:/ti/ccs1000/ccs/tools/compiler/msp430_4.2.7/include" --advice:power=all -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="camera/$(basename $(<F)).d_raw" --obj_directory="camera" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


