################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/versiones\ viejas/adc.c \
../Core/Src/versiones\ viejas/borrador.c \
../Core/Src/versiones\ viejas/borrador_19_jul.c \
../Core/Src/versiones\ viejas/borrador_funcional.c \
../Core/Src/versiones\ viejas/eeprom_dma.c \
../Core/Src/versiones\ viejas/fsm_2.c \
../Core/Src/versiones\ viejas/teclado.c 

OBJS += \
./Core/Src/versiones\ viejas/adc.o \
./Core/Src/versiones\ viejas/borrador.o \
./Core/Src/versiones\ viejas/borrador_19_jul.o \
./Core/Src/versiones\ viejas/borrador_funcional.o \
./Core/Src/versiones\ viejas/eeprom_dma.o \
./Core/Src/versiones\ viejas/fsm_2.o \
./Core/Src/versiones\ viejas/teclado.o 

C_DEPS += \
./Core/Src/versiones\ viejas/adc.d \
./Core/Src/versiones\ viejas/borrador.d \
./Core/Src/versiones\ viejas/borrador_19_jul.d \
./Core/Src/versiones\ viejas/borrador_funcional.d \
./Core/Src/versiones\ viejas/eeprom_dma.d \
./Core/Src/versiones\ viejas/fsm_2.d \
./Core/Src/versiones\ viejas/teclado.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/versiones\ viejas/adc.o: ../Core/Src/versiones\ viejas/adc.c Core/Src/versiones\ viejas/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/Users/alekseychahuillco/Documents/FIUBA/1er_Cuatri_2025/Taller_de_Sistemas_Embebidos/TPFINAL/TPF/app/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Core/Src/versiones viejas/adc.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/versiones\ viejas/borrador.o: ../Core/Src/versiones\ viejas/borrador.c Core/Src/versiones\ viejas/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/Users/alekseychahuillco/Documents/FIUBA/1er_Cuatri_2025/Taller_de_Sistemas_Embebidos/TPFINAL/TPF/app/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Core/Src/versiones viejas/borrador.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/versiones\ viejas/borrador_19_jul.o: ../Core/Src/versiones\ viejas/borrador_19_jul.c Core/Src/versiones\ viejas/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/Users/alekseychahuillco/Documents/FIUBA/1er_Cuatri_2025/Taller_de_Sistemas_Embebidos/TPFINAL/TPF/app/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Core/Src/versiones viejas/borrador_19_jul.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/versiones\ viejas/borrador_funcional.o: ../Core/Src/versiones\ viejas/borrador_funcional.c Core/Src/versiones\ viejas/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/Users/alekseychahuillco/Documents/FIUBA/1er_Cuatri_2025/Taller_de_Sistemas_Embebidos/TPFINAL/TPF/app/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Core/Src/versiones viejas/borrador_funcional.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/versiones\ viejas/eeprom_dma.o: ../Core/Src/versiones\ viejas/eeprom_dma.c Core/Src/versiones\ viejas/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/Users/alekseychahuillco/Documents/FIUBA/1er_Cuatri_2025/Taller_de_Sistemas_Embebidos/TPFINAL/TPF/app/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Core/Src/versiones viejas/eeprom_dma.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/versiones\ viejas/fsm_2.o: ../Core/Src/versiones\ viejas/fsm_2.c Core/Src/versiones\ viejas/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/Users/alekseychahuillco/Documents/FIUBA/1er_Cuatri_2025/Taller_de_Sistemas_Embebidos/TPFINAL/TPF/app/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Core/Src/versiones viejas/fsm_2.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/versiones\ viejas/teclado.o: ../Core/Src/versiones\ viejas/teclado.c Core/Src/versiones\ viejas/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"/Users/alekseychahuillco/Documents/FIUBA/1er_Cuatri_2025/Taller_de_Sistemas_Embebidos/TPFINAL/TPF/app/inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Core/Src/versiones viejas/teclado.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-versiones-20-viejas

clean-Core-2f-Src-2f-versiones-20-viejas:
	-$(RM) ./Core/Src/versiones\ viejas/adc.cyclo ./Core/Src/versiones\ viejas/adc.d ./Core/Src/versiones\ viejas/adc.o ./Core/Src/versiones\ viejas/adc.su ./Core/Src/versiones\ viejas/borrador.cyclo ./Core/Src/versiones\ viejas/borrador.d ./Core/Src/versiones\ viejas/borrador.o ./Core/Src/versiones\ viejas/borrador.su ./Core/Src/versiones\ viejas/borrador_19_jul.cyclo ./Core/Src/versiones\ viejas/borrador_19_jul.d ./Core/Src/versiones\ viejas/borrador_19_jul.o ./Core/Src/versiones\ viejas/borrador_19_jul.su ./Core/Src/versiones\ viejas/borrador_funcional.cyclo ./Core/Src/versiones\ viejas/borrador_funcional.d ./Core/Src/versiones\ viejas/borrador_funcional.o ./Core/Src/versiones\ viejas/borrador_funcional.su ./Core/Src/versiones\ viejas/eeprom_dma.cyclo ./Core/Src/versiones\ viejas/eeprom_dma.d ./Core/Src/versiones\ viejas/eeprom_dma.o ./Core/Src/versiones\ viejas/eeprom_dma.su ./Core/Src/versiones\ viejas/fsm_2.cyclo ./Core/Src/versiones\ viejas/fsm_2.d ./Core/Src/versiones\ viejas/fsm_2.o ./Core/Src/versiones\ viejas/fsm_2.su ./Core/Src/versiones\ viejas/teclado.cyclo ./Core/Src/versiones\ viejas/teclado.d ./Core/Src/versiones\ viejas/teclado.o ./Core/Src/versiones\ viejas/teclado.su

.PHONY: clean-Core-2f-Src-2f-versiones-20-viejas

