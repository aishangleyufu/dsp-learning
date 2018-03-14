################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv5/tools/compiler/c2000_6.2.0

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"E:/dsplab281x/cmd/EzDSP_RAM_lnk.cmd" \
"F:/dsplab281x/main/CpuTimer0/Debug/DSP281x_SysCtrl.obj" \
"F:/dsplab281x/main/CpuTimer0/Debug/DSP281x_PieVect.obj" \
"F:/dsplab281x/main/CpuTimer0/Debug/DSP281x_PieCtrl.obj" \
"F:/dsplab281x/main/CpuTimer0/Debug/DSP281x_GpioF.obj" \
"F:/dsplab281x/main/CpuTimer0/Debug/DSP281x_GlobalVariableDefs.obj" \
"F:/dsplab281x/main/CpuTimer0/Debug/DSP281x_DefaultIsr.obj" \
"F:/dsplab281x/main/CpuTimer0/Debug/DSP281x_CpuTimers.obj" \
"F:/dsplab281x/main/CpuTimer0/Debug/DSP281x_CodeStartBranch.obj" \
"F:/dsplab281x/main/CpuTimer0/Debug/CpuTimer0.obj" \
"E:/dsplab281x/cmd/DSP281x_Headers_nonBIOS.cmd" \
"E:/dsplab281x/lib/rts2800_ml.lib" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: E:/dsplab281x/main/CpuTimer0/CpuTimer0/../../../main/CpuTimer0/Debug/CpuTimer0.out

# Tool invocations
E:/dsplab281x/main/CpuTimer0/CpuTimer0/../../../main/CpuTimer0/Debug/CpuTimer0.out: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: C2000 Linker'
	"C:/ti/ccsv5/tools/compiler/c2000_6.2.0/bin/cl2000" -v28 -ml -g --define="_DEBUG" --define="LARGE_MODEL" --quiet --display_error_number --diag_wrap=off --output_all_syms --obj_directory="F:/dsplab281x/main/CpuTimer0/Debug" -z -m"E:/dsplab281x/main/CpuTimer0/Debug/CpuTimer0.map" --warn_sections -i"C:/ti/ccsv5/tools/compiler/c2000_6.2.0/lib" -i"C:/ti/ccsv5/tools/compiler/c2000_6.2.0/include" -i"E:/dsplab281x/main/CpuTimer0/CpuTimer0" -i"E:/dsplab281x/main/CpuTimer0" --reread_libs --display_error_number --diag_wrap=off --xml_link_info="CpuTimer0_linkInfo.xml" --rom_model -o "E:/dsplab281x/main/CpuTimer0/CpuTimer0/../../../main/CpuTimer0/Debug/CpuTimer0.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C2000_EXECUTABLE_OUTPUTS__QUOTED) "E:\dsplab281x\main\CpuTimer0\Debug\CpuTimer0.out"
	-$(RM) "CpuTimer0.pp" "DSP281x_CpuTimers.pp" "DSP281x_DefaultIsr.pp" "DSP281x_GlobalVariableDefs.pp" "DSP281x_GpioF.pp" "DSP281x_PieCtrl.pp" "DSP281x_PieVect.pp" "DSP281x_SysCtrl.pp" 
	-$(RM) "F:\dsplab281x\main\CpuTimer0\Debug\CpuTimer0.obj" "F:\dsplab281x\main\CpuTimer0\Debug\DSP281x_CodeStartBranch.obj" "F:\dsplab281x\main\CpuTimer0\Debug\DSP281x_CpuTimers.obj" "F:\dsplab281x\main\CpuTimer0\Debug\DSP281x_DefaultIsr.obj" "F:\dsplab281x\main\CpuTimer0\Debug\DSP281x_GlobalVariableDefs.obj" "F:\dsplab281x\main\CpuTimer0\Debug\DSP281x_GpioF.obj" "F:\dsplab281x\main\CpuTimer0\Debug\DSP281x_PieCtrl.obj" "F:\dsplab281x\main\CpuTimer0\Debug\DSP281x_PieVect.obj" "F:\dsplab281x\main\CpuTimer0\Debug\DSP281x_SysCtrl.obj" 
	-$(RM) "DSP281x_CodeStartBranch.pp" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

