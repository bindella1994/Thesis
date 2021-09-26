################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
satellite/%.obj: ../satellite/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/bin/cl430" -vmspx --abi=eabi --data_model=large -Ooff --include_path="C:/ti/ccs1040/ccs/ccs_base/msp430/include" --include_path="C:/Users/Manuel-desktop/Downloads/210504_ABACUS_Libraries_ABCS_2021_v1/ABCS_2021_v1" --include_path="C:/Users/Manuel-desktop/Downloads/210504_ABACUS_Libraries_ABCS_2021_v1/ABACUS_Libraries" --include_path="C:/ti/ccs1040/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/include" --advice:power="all" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="satellite/$(basename $(<F)).d_raw" --obj_directory="satellite" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


