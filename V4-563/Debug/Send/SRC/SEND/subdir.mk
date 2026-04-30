################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Send/SRC/SEND/ARGS.cpp \
../Send/SRC/SEND/DEC_TST.cpp \
../Send/SRC/SEND/SELF_TST.cpp \
../Send/SRC/SEND/SEND.cpp \
../Send/SRC/SEND/SEND_REG.cpp \
../Send/SRC/SEND/SR_CORE.cpp \
../Send/SRC/SEND/ST_VECT.cpp \
../Send/SRC/SEND/TEST_REG.cpp 

OBJS += \
./Send/SRC/SEND/ARGS.o \
./Send/SRC/SEND/DEC_TST.o \
./Send/SRC/SEND/SELF_TST.o \
./Send/SRC/SEND/SEND.o \
./Send/SRC/SEND/SEND_REG.o \
./Send/SRC/SEND/SR_CORE.o \
./Send/SRC/SEND/ST_VECT.o \
./Send/SRC/SEND/TEST_REG.o 

CPP_DEPS += \
./Send/SRC/SEND/ARGS.d \
./Send/SRC/SEND/DEC_TST.d \
./Send/SRC/SEND/SELF_TST.d \
./Send/SRC/SEND/SEND.d \
./Send/SRC/SEND/SEND_REG.d \
./Send/SRC/SEND/SR_CORE.d \
./Send/SRC/SEND/ST_VECT.d \
./Send/SRC/SEND/TEST_REG.d 


# Each subdirectory must supply rules for building sources it contributes
Send/SRC/SEND/%.o Send/SRC/SEND/%.su Send/SRC/SEND/%.cyclo: ../Send/SRC/SEND/%.cpp Send/SRC/SEND/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m33 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -DNX_INCLUDE_USER_DEFINE_FILE -DSEND_VERSION=4 -DDECODER_TEST -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../NetXDuo/App -I../Middlewares/ST/netxduo/common/inc -I../Middlewares/ST/netxduo/ports/cortex_m33/gnu/inc -I"C:/Users/konem/nmradcc/sender/V4-563/Shell" -I"C:/Users/konem/nmradcc/sender/V4-563/Minini" -I"C:/Users/konem/nmradcc/sender/V4-563/Drivers/BSP" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/http" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/telnet" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/tftp" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/web" -I"C:/Users/konem/nmradcc/sender/V4-563/Arch" -I../NetXDuo/Target -I../Drivers/BSP/Components/lan8742 -I../Middlewares/ST/netxduo/common/drivers/ethernet -I"C:/Users/konem/nmradcc/sender/V4-563/CS/Wangrow" -I"C:/Users/konem/nmradcc/sender/V4-563/CS" -I"C:/Users/konem/nmradcc/sender/V4-563/Send/INCLUDE" -I"C:/Users/konem/nmradcc/sender/V4-563/Send/SRC/SEND" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/LwIP/src/include" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/LwIP/system/arch" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Core/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/USB" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Send-2f-SRC-2f-SEND

clean-Send-2f-SRC-2f-SEND:
	-$(RM) ./Send/SRC/SEND/ARGS.cyclo ./Send/SRC/SEND/ARGS.d ./Send/SRC/SEND/ARGS.o ./Send/SRC/SEND/ARGS.su ./Send/SRC/SEND/DEC_TST.cyclo ./Send/SRC/SEND/DEC_TST.d ./Send/SRC/SEND/DEC_TST.o ./Send/SRC/SEND/DEC_TST.su ./Send/SRC/SEND/SELF_TST.cyclo ./Send/SRC/SEND/SELF_TST.d ./Send/SRC/SEND/SELF_TST.o ./Send/SRC/SEND/SELF_TST.su ./Send/SRC/SEND/SEND.cyclo ./Send/SRC/SEND/SEND.d ./Send/SRC/SEND/SEND.o ./Send/SRC/SEND/SEND.su ./Send/SRC/SEND/SEND_REG.cyclo ./Send/SRC/SEND/SEND_REG.d ./Send/SRC/SEND/SEND_REG.o ./Send/SRC/SEND/SEND_REG.su ./Send/SRC/SEND/SR_CORE.cyclo ./Send/SRC/SEND/SR_CORE.d ./Send/SRC/SEND/SR_CORE.o ./Send/SRC/SEND/SR_CORE.su ./Send/SRC/SEND/ST_VECT.cyclo ./Send/SRC/SEND/ST_VECT.d ./Send/SRC/SEND/ST_VECT.o ./Send/SRC/SEND/ST_VECT.su ./Send/SRC/SEND/TEST_REG.cyclo ./Send/SRC/SEND/TEST_REG.d ./Send/SRC/SEND/TEST_REG.o ./Send/SRC/SEND/TEST_REG.su

.PHONY: clean-Send-2f-SRC-2f-SEND

