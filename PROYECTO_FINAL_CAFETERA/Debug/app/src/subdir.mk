################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app/src/app.c \
../app/src/logger.c \
../app/src/task_actuator.c \
../app/src/task_adc.c \
../app/src/task_botones.c \
../app/src/task_cafe_fsm.c \
../app/src/task_lcd.c \
../app/src/task_sensor.c \
../app/src/task_stock.c \
../app/src/task_system.c 

OBJS += \
./app/src/app.o \
./app/src/logger.o \
./app/src/task_actuator.o \
./app/src/task_adc.o \
./app/src/task_botones.o \
./app/src/task_cafe_fsm.o \
./app/src/task_lcd.o \
./app/src/task_sensor.o \
./app/src/task_stock.o \
./app/src/task_system.o 

C_DEPS += \
./app/src/app.d \
./app/src/logger.d \
./app/src/task_actuator.d \
./app/src/task_adc.d \
./app/src/task_botones.d \
./app/src/task_cafe_fsm.d \
./app/src/task_lcd.d \
./app/src/task_sensor.d \
./app/src/task_stock.d \
./app/src/task_system.d 


# Each subdirectory must supply rules for building sources it contributes
app/src/%.o app/src/%.su app/src/%.cyclo: ../app/src/%.c app/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/Users/alekseychahuillco/Documents/FIUBA/1er_Cuatri_2025/Taller_de_Sistemas_Embebidos/PROYECTO FINAL/PROYECTO_FINAL_CAFETERA/app/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-app-2f-src

clean-app-2f-src:
	-$(RM) ./app/src/app.cyclo ./app/src/app.d ./app/src/app.o ./app/src/app.su ./app/src/logger.cyclo ./app/src/logger.d ./app/src/logger.o ./app/src/logger.su ./app/src/task_actuator.cyclo ./app/src/task_actuator.d ./app/src/task_actuator.o ./app/src/task_actuator.su ./app/src/task_adc.cyclo ./app/src/task_adc.d ./app/src/task_adc.o ./app/src/task_adc.su ./app/src/task_botones.cyclo ./app/src/task_botones.d ./app/src/task_botones.o ./app/src/task_botones.su ./app/src/task_cafe_fsm.cyclo ./app/src/task_cafe_fsm.d ./app/src/task_cafe_fsm.o ./app/src/task_cafe_fsm.su ./app/src/task_lcd.cyclo ./app/src/task_lcd.d ./app/src/task_lcd.o ./app/src/task_lcd.su ./app/src/task_sensor.cyclo ./app/src/task_sensor.d ./app/src/task_sensor.o ./app/src/task_sensor.su ./app/src/task_stock.cyclo ./app/src/task_stock.d ./app/src/task_stock.o ./app/src/task_stock.su ./app/src/task_system.cyclo ./app/src/task_system.d ./app/src/task_system.o ./app/src/task_system.su

.PHONY: clean-app-2f-src

