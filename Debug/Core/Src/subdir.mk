################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/button.c \
../Core/Src/crc.c \
../Core/Src/fsm_input_processing.c \
../Core/Src/fsm_main_processing.c \
../Core/Src/fsm_output_processing.c \
../Core/Src/logger.c \
../Core/Src/main.c \
../Core/Src/soft_timer.c \
../Core/Src/stm32f1xx_hal_msp.c \
../Core/Src/stm32f1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f1xx.c \
../Core/Src/uart.c 

OBJS += \
./Core/Src/button.o \
./Core/Src/crc.o \
./Core/Src/fsm_input_processing.o \
./Core/Src/fsm_main_processing.o \
./Core/Src/fsm_output_processing.o \
./Core/Src/logger.o \
./Core/Src/main.o \
./Core/Src/soft_timer.o \
./Core/Src/stm32f1xx_hal_msp.o \
./Core/Src/stm32f1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f1xx.o \
./Core/Src/uart.o 

C_DEPS += \
./Core/Src/button.d \
./Core/Src/crc.d \
./Core/Src/fsm_input_processing.d \
./Core/Src/fsm_main_processing.d \
./Core/Src/fsm_output_processing.d \
./Core/Src/logger.d \
./Core/Src/main.d \
./Core/Src/soft_timer.d \
./Core/Src/stm32f1xx_hal_msp.d \
./Core/Src/stm32f1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f1xx.d \
./Core/Src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/button.cyclo ./Core/Src/button.d ./Core/Src/button.o ./Core/Src/button.su ./Core/Src/crc.cyclo ./Core/Src/crc.d ./Core/Src/crc.o ./Core/Src/crc.su ./Core/Src/fsm_input_processing.cyclo ./Core/Src/fsm_input_processing.d ./Core/Src/fsm_input_processing.o ./Core/Src/fsm_input_processing.su ./Core/Src/fsm_main_processing.cyclo ./Core/Src/fsm_main_processing.d ./Core/Src/fsm_main_processing.o ./Core/Src/fsm_main_processing.su ./Core/Src/fsm_output_processing.cyclo ./Core/Src/fsm_output_processing.d ./Core/Src/fsm_output_processing.o ./Core/Src/fsm_output_processing.su ./Core/Src/logger.cyclo ./Core/Src/logger.d ./Core/Src/logger.o ./Core/Src/logger.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/soft_timer.cyclo ./Core/Src/soft_timer.d ./Core/Src/soft_timer.o ./Core/Src/soft_timer.su ./Core/Src/stm32f1xx_hal_msp.cyclo ./Core/Src/stm32f1xx_hal_msp.d ./Core/Src/stm32f1xx_hal_msp.o ./Core/Src/stm32f1xx_hal_msp.su ./Core/Src/stm32f1xx_it.cyclo ./Core/Src/stm32f1xx_it.d ./Core/Src/stm32f1xx_it.o ./Core/Src/stm32f1xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f1xx.cyclo ./Core/Src/system_stm32f1xx.d ./Core/Src/system_stm32f1xx.o ./Core/Src/system_stm32f1xx.su ./Core/Src/uart.cyclo ./Core/Src/uart.d ./Core/Src/uart.o ./Core/Src/uart.su

.PHONY: clean-Core-2f-Src

