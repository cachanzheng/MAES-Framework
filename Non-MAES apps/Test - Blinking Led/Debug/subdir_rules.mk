################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
MSP_EXP432P401R.obj: ../MSP_EXP432P401R.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv7/ccs_base/arm/include" --include_path="C:/ti/ccsv7/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Carmen/workspace_v7/Test - Blinking Led" --include_path="C:/Users/Carmen/workspace_v7/Test - Blinking Led" --include_path="C:/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc/CMSIS" --include_path="C:/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc" --include_path="C:/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/driverlib/MSP432P4xx" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS/include" --advice:power=all --advice:power_severity=suppress --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --define=MSP432WARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="MSP_EXP432P401R.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

empty_min.obj: ../empty_min.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccsv7/ccs_base/arm/include" --include_path="C:/ti/ccsv7/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Carmen/workspace_v7/Test - Blinking Led" --include_path="C:/Users/Carmen/workspace_v7/Test - Blinking Led" --include_path="C:/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc/CMSIS" --include_path="C:/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc" --include_path="C:/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/driverlib/MSP432P4xx" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS/include" --advice:power=all --advice:power_severity=suppress --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --define=MSP432WARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="empty_min.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

build-907870319:
	@$(MAKE) -Onone -f subdir_rules.mk build-907870319-inproc

build-907870319-inproc: ../empty_min.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"C:/ti/xdctools_3_32_00_06_core/xs" --xdcpath="C:/ti/tirtos_msp43x_2_20_00_06/packages;C:/ti/tirtos_msp43x_2_20_00_06/products/bios_6_46_00_23/packages;C:/ti/tirtos_msp43x_2_20_00_06/products/tidrivers_msp43x_2_20_00_08/packages;C:/ti/tirtos_msp43x_2_20_00_06/products/uia_2_00_06_52/packages;C:/ti/ccsv7/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M4F -p ti.platforms.msp432:MSP432P401R -r release -c "C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS" --compileOptions "-mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path=\"C:/ti/ccsv7/ccs_base/arm/include\" --include_path=\"C:/ti/ccsv7/ccs_base/arm/include/CMSIS\" --include_path=\"C:/Users/Carmen/workspace_v7/Test - Blinking Led\" --include_path=\"C:/Users/Carmen/workspace_v7/Test - Blinking Led\" --include_path=\"C:/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc/CMSIS\" --include_path=\"C:/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc\" --include_path=\"C:/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/driverlib/MSP432P4xx\" --include_path=\"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS/include\" --advice:power=all --advice:power_severity=suppress --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --define=MSP432WARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi  " "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/linker.cmd: build-907870319 ../empty_min.cfg
configPkg/compiler.opt: build-907870319
configPkg/: build-907870319


