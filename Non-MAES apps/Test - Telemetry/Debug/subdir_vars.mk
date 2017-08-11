################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CFG_SRCS += \
../empty_min.cfg 

CMD_SRCS += \
../MSP_EXP432P401R.cmd 

C_SRCS += \
../MSP_EXP432P401R.c \
../telemetry.c 

GEN_CMDS += \
./configPkg/linker.cmd 

GEN_FILES += \
./configPkg/linker.cmd \
./configPkg/compiler.opt 

GEN_MISC_DIRS += \
./configPkg/ 

C_DEPS += \
./MSP_EXP432P401R.d \
./telemetry.d 

GEN_OPTS += \
./configPkg/compiler.opt 

OBJS += \
./MSP_EXP432P401R.obj \
./telemetry.obj 

GEN_MISC_DIRS__QUOTED += \
"configPkg\" 

OBJS__QUOTED += \
"MSP_EXP432P401R.obj" \
"telemetry.obj" 

C_DEPS__QUOTED += \
"MSP_EXP432P401R.d" \
"telemetry.d" 

GEN_FILES__QUOTED += \
"configPkg\linker.cmd" \
"configPkg\compiler.opt" 

C_SRCS__QUOTED += \
"../MSP_EXP432P401R.c" \
"../telemetry.c" 


