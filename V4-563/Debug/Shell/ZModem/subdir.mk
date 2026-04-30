################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Shell/ZModem/ZMisc.c \
../Shell/ZModem/ZReceive.c \
../Shell/ZModem/ZSend.c \
../Shell/ZModem/ZVars.c 

OBJS += \
./Shell/ZModem/ZMisc.o \
./Shell/ZModem/ZReceive.o \
./Shell/ZModem/ZSend.o \
./Shell/ZModem/ZVars.o 

C_DEPS += \
./Shell/ZModem/ZMisc.d \
./Shell/ZModem/ZReceive.d \
./Shell/ZModem/ZSend.d \
./Shell/ZModem/ZVars.d 


# Each subdirectory must supply rules for building sources it contributes
Shell/ZModem/%.o Shell/ZModem/%.su Shell/ZModem/%.cyclo: ../Shell/ZModem/%.c Shell/ZModem/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -DFX_INCLUDE_USER_DEFINE_FILE -DNX_INCLUDE_USER_DEFINE_FILE -DUX_INCLUDE_USER_DEFINE_FILE -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../FileX/App -I../NetXDuo/App -I../USBX/App -I../USBX/Target -I../Middlewares/ST/filex/common/inc -I../Middlewares/ST/filex/ports/generic/inc -I../Middlewares/ST/netxduo/common/inc -I../Middlewares/ST/netxduo/ports/cortex_m33/gnu/inc -I../Middlewares/ST/usbx/common/core/inc -I../Middlewares/ST/usbx/ports/generic/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Shell-2f-ZModem

clean-Shell-2f-ZModem:
	-$(RM) ./Shell/ZModem/ZMisc.cyclo ./Shell/ZModem/ZMisc.d ./Shell/ZModem/ZMisc.o ./Shell/ZModem/ZMisc.su ./Shell/ZModem/ZReceive.cyclo ./Shell/ZModem/ZReceive.d ./Shell/ZModem/ZReceive.o ./Shell/ZModem/ZReceive.su ./Shell/ZModem/ZSend.cyclo ./Shell/ZModem/ZSend.d ./Shell/ZModem/ZSend.o ./Shell/ZModem/ZSend.su ./Shell/ZModem/ZVars.cyclo ./Shell/ZModem/ZVars.d ./Shell/ZModem/ZVars.o ./Shell/ZModem/ZVars.su

.PHONY: clean-Shell-2f-ZModem

