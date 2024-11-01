################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Common/Common.c 

LST += \
Common.lst 

C_DEPS += \
./Common/Common.d 

OBJS += \
./Common/Common.o 

MAP += \
DGC_A230_6_Rev_1_0_Software.map 


# Each subdirectory must supply rules for building sources it contributes
Common/%.o: ../Common/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-O0 -ffunction-sections -fdata-sections -Wstack-usage=256 -g2 -mcpu=s2 -I"E:/AIoT/Code/Rogger/Test_new/DGC_A230_6_Rev_1_0_Software/generate" -I"E:\\AIoT\\Code\\Rogger\\Test_new\\DGC_A230_6_Rev_1_0_Software\\pfdl" -I"E:\\AIoT\\Code\\Rogger\\Test_new\\DGC_A230_6_Rev_1_0_Software\\DGC_A230_Functions" -I"E:\\AIoT\\Code\\Rogger\\Test_new\\DGC_A230_6_Rev_1_0_Software\\HMI" -I"E:\\AIoT\\Code\\Rogger\\Test_new\\DGC_A230_6_Rev_1_0_Software\\src" -I"E:\\AIoT\\Code\\Rogger\\Test_new\\DGC_A230_6_Rev_1_0_Software\\Common" -Wa,-adlnh="$(basename $(notdir $<)).lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" "$<" -c -o "$@")
	@rl78-elf-gcc @"$@.in"

