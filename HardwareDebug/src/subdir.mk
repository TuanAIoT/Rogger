################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
ASM_SRCS += \
../src/r_reset_program.asm 

C_SRCS += \
../src/r_cg_cgc.c \
../src/r_cg_cgc_user.c \
../src/r_cg_intc.c \
../src/r_cg_intc_user.c \
../src/r_cg_pfdl.c \
../src/r_cg_port.c \
../src/r_cg_port_user.c \
../src/r_cg_serial.c \
../src/r_cg_serial_user.c \
../src/r_cg_timer.c \
../src/r_cg_timer_user.c \
../src/r_cg_vector_table.c \
../src/r_hardware_setup.c \
../src/r_main.c 

LST += \
r_cg_cgc.lst \
r_cg_cgc_user.lst \
r_cg_intc.lst \
r_cg_intc_user.lst \
r_cg_pfdl.lst \
r_cg_port.lst \
r_cg_port_user.lst \
r_cg_serial.lst \
r_cg_serial_user.lst \
r_cg_timer.lst \
r_cg_timer_user.lst \
r_cg_vector_table.lst \
r_hardware_setup.lst \
r_main.lst \
r_reset_program.lst 

C_DEPS += \
./src/r_cg_cgc.d \
./src/r_cg_cgc_user.d \
./src/r_cg_intc.d \
./src/r_cg_intc_user.d \
./src/r_cg_pfdl.d \
./src/r_cg_port.d \
./src/r_cg_port_user.d \
./src/r_cg_serial.d \
./src/r_cg_serial_user.d \
./src/r_cg_timer.d \
./src/r_cg_timer_user.d \
./src/r_cg_vector_table.d \
./src/r_hardware_setup.d \
./src/r_main.d 

OBJS += \
./src/r_cg_cgc.o \
./src/r_cg_cgc_user.o \
./src/r_cg_intc.o \
./src/r_cg_intc_user.o \
./src/r_cg_pfdl.o \
./src/r_cg_port.o \
./src/r_cg_port_user.o \
./src/r_cg_serial.o \
./src/r_cg_serial_user.o \
./src/r_cg_timer.o \
./src/r_cg_timer_user.o \
./src/r_cg_vector_table.o \
./src/r_hardware_setup.o \
./src/r_main.o \
./src/r_reset_program.o 

ASM_DEPS += \
./src/r_reset_program.d 

MAP += \
DGC_A230_6_Rev_1_0_Software.map 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-O0 -ffunction-sections -fdata-sections -Wstack-usage=256 -g2 -mcpu=s2 -I"E:/AIoT/Code/Rogger/DGC_A230_6_Rev_1_0_Software/generate" -I"E:\\AIoT\\Code\\Rogger\\DGC_A230_6_Rev_1_0_Software\\pfdl" -I"E:\\AIoT\\Code\\Rogger\\DGC_A230_6_Rev_1_0_Software\\DGC_A230_Functions" -I"E:\\AIoT\\Code\\Rogger\\DGC_A230_6_Rev_1_0_Software\\HMI" -I"E:\\AIoT\\Code\\Rogger\\DGC_A230_6_Rev_1_0_Software\\src" -I"E:\\AIoT\\Code\\Rogger\\DGC_A230_6_Rev_1_0_Software\\Common" -Wa,-adlnh="$(basename $(notdir $<)).lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" "$<" -c -o "$@")
	@rl78-elf-gcc @"$@.in"
src/%.o: ../src/%.asm
	@echo 'Building file: $<'
	$(file > $@.in,-O0 -ffunction-sections -fdata-sections -Wstack-usage=256 -g2 -mcpu=s2 -x assembler-with-cpp -Wa,--gdwarf2 -I"E:/AIoT/Code/Rogger/DGC_A230_6_Rev_1_0_Software/src" -Wa,-adlhn="$(basename $(notdir $<)).lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c "$<" -o "$@")
	@rl78-elf-gcc @"$@.in"

