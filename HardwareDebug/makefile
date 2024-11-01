################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

-include makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include generate/subdir.mk
-include KEELOQ/subdir.mk
-include HMI/subdir.mk
-include DGC_A230_Functions/subdir.mk
-include Common/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(JMP_DEPS)),)
-include $(JMP_DEPS)
endif
ifneq ($(strip $(FSY_DEPS)),)
-include $(FSY_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(SRC_UPPER_DEPS)),)
-include $(SRC_UPPER_DEPS)
endif
ifneq ($(strip $(P_UPPER_DEPS)),)
-include $(P_UPPER_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(PP_UPPER_DEPS)),)
-include $(PP_UPPER_DEPS)
endif
ifneq ($(strip $(P_DEPS)),)
-include $(P_DEPS)
endif
ifneq ($(strip $(FSY_UPPER_DEPS)),)
-include $(FSY_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(SRC_DEPS)),)
-include $(SRC_DEPS)
endif
ifneq ($(strip $(JMP_UPPER_DEPS)),)
-include $(JMP_UPPER_DEPS)
endif
ifneq ($(strip $(PP_DEPS)),)
-include $(PP_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(SX_DEPS)),)
-include $(SX_DEPS)
endif
ifneq ($(strip $(SX_UPPER_DEPS)),)
-include $(SX_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LIBRARY_GENERATOR_OUTPUTTYPE_OUTPUTS += \

OBJCOPY += \
DGC_A230_6_Rev_1_0_Software.mot \

PRINT_SIZE += \
DGC_A230_6_Rev_1_0_Software.siz \


# All Target
all: DGC_A230_6_Rev_1_0_Software.elf secondary-outputs

# Tool invocations
DGC_A230_6_Rev_1_0_Software.elf: $(OBJS) $(USER_OBJS) $(LIBRARY_GENERATOR_OUTPUTTYPE_OUTPUTS) $(LINKER_SCRIPT)
	$(file > DGC_A230_6_Rev_1_0_Software.elf.in,-O0 -ffunction-sections -fdata-sections -Wstack-usage=256 -g2 -mcpu=s2 -o "DGC_A230_6_Rev_1_0_Software.elf" $(OBJS) $(USER_OBJS) $(LIBS) -T "E:/AIoT/Code/Rogger/Test_new/DGC_A230_6_Rev_1_0_Software/generate/linker_script.ld" -Wl,--start-group -lm -lc -lgcc -Wl,--end-group -nostartfiles -Wl,-e_PowerON_Reset -Wl,--Map=DGC_A230_6_Rev_1_0_Software.map)
	@echo Building target: $@ && rl78-elf-gcc @"DGC_A230_6_Rev_1_0_Software.elf.in"
: 
	rl78-elf-libgen  --select-lib=newlib --compiler-options=-O0,-ffunction-sections,-fdata-sections,-mcpu=s2  -o
DGC_A230_6_Rev_1_0_Software.mot: DGC_A230_6_Rev_1_0_Software.elf
	rl78-elf-objcopy "DGC_A230_6_Rev_1_0_Software.elf" -O srec  "DGC_A230_6_Rev_1_0_Software.mot"
DGC_A230_6_Rev_1_0_Software.siz: DGC_A230_6_Rev_1_0_Software.elf
	rl78-elf-size --format=berkeley "DGC_A230_6_Rev_1_0_Software.elf"
# Other Targets
clean:
	-$(file > clean_file.tmp,  $(PRINT_SIZE)  $(JMP_DEPS)  $(FSY_DEPS)  $(C_UPPER_DEPS)  $(SRC_UPPER_DEPS)  $(P_UPPER_DEPS)  $(LST)  $(S_DEPS)  $(PP_UPPER_DEPS)  $(P_DEPS)  $(FSY_UPPER_DEPS)  $(C_DEPS)  $(SRC_DEPS)  $(OBJCOPY)  $(JMP_UPPER_DEPS)  $(PP_DEPS)  $(ASM_UPPER_DEPS)  $(SX_DEPS)  $(OBJS)  $(SX_UPPER_DEPS)  $(ASM_DEPS)  $(LIBRARY_GENERATOR_OUTPUTTYPE_OUTPUTS)  $(MAP)  $(S_UPPER_DEPS) )
	-@ xargs -P 2 -s 32000 -t -a clean_file.tmp $(RM)
	-@$(RM) clean_file.tmp
	-$(RM)  DGC_A230_6_Rev_1_0_Software.elf
	-@echo ' '

secondary-outputs: $(OBJCOPY) $(PRINT_SIZE)

.PHONY: all clean dependents 

-include ../makefile.targets
