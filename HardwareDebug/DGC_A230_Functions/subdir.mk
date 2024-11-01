################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../DGC_A230_Functions/DGC_A230_Motor_Functions.c \
../DGC_A230_Functions/DGC_A230_Normal_Mode.c \
../DGC_A230_Functions/DGC_A230_Parameters_Functions.c \
../DGC_A230_Functions/DGC_A230_Program_Mode.c \
../DGC_A230_Functions/DGC_A230_Test_Mode.c 

LST += \
DGC_A230_Motor_Functions.lst \
DGC_A230_Normal_Mode.lst \
DGC_A230_Parameters_Functions.lst \
DGC_A230_Program_Mode.lst \
DGC_A230_Test_Mode.lst 

C_DEPS += \
./DGC_A230_Functions/DGC_A230_Motor_Functions.d \
./DGC_A230_Functions/DGC_A230_Normal_Mode.d \
./DGC_A230_Functions/DGC_A230_Parameters_Functions.d \
./DGC_A230_Functions/DGC_A230_Program_Mode.d \
./DGC_A230_Functions/DGC_A230_Test_Mode.d 

OBJS += \
./DGC_A230_Functions/DGC_A230_Motor_Functions.o \
./DGC_A230_Functions/DGC_A230_Normal_Mode.o \
./DGC_A230_Functions/DGC_A230_Parameters_Functions.o \
./DGC_A230_Functions/DGC_A230_Program_Mode.o \
./DGC_A230_Functions/DGC_A230_Test_Mode.o 

MAP += \
DGC_A230_6_Rev_1_0_Software.map 


# Each subdirectory must supply rules for building sources it contributes
DGC_A230_Functions/%.o: ../DGC_A230_Functions/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-O0 -ffunction-sections -fdata-sections -Wstack-usage=256 -g2 -mcpu=s2 -I"E:/AIoT/Code/Rogger/DGC_A230_6_Rev_1_0_Software/generate" -I"E:\\AIoT\\Code\\Rogger\\DGC_A230_6_Rev_1_0_Software\\pfdl" -I"E:\\AIoT\\Code\\Rogger\\DGC_A230_6_Rev_1_0_Software\\DGC_A230_Functions" -I"E:\\AIoT\\Code\\Rogger\\DGC_A230_6_Rev_1_0_Software\\HMI" -I"E:\\AIoT\\Code\\Rogger\\DGC_A230_6_Rev_1_0_Software\\src" -I"E:\\AIoT\\Code\\Rogger\\DGC_A230_6_Rev_1_0_Software\\Common" -Wa,-adlnh="$(basename $(notdir $<)).lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" "$<" -c -o "$@")
	@rl78-elf-gcc @"$@.in"

