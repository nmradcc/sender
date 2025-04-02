################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/LwIP/src/api/api_lib.c \
../Middlewares/Third_Party/LwIP/src/api/api_msg.c \
../Middlewares/Third_Party/LwIP/src/api/err.c \
../Middlewares/Third_Party/LwIP/src/api/netbuf.c \
../Middlewares/Third_Party/LwIP/src/api/netdb.c \
../Middlewares/Third_Party/LwIP/src/api/netifapi.c \
../Middlewares/Third_Party/LwIP/src/api/sockets.c \
../Middlewares/Third_Party/LwIP/src/api/tcpip.c 

C_DEPS += \
./Middlewares/Third_Party/LwIP/src/api/api_lib.d \
./Middlewares/Third_Party/LwIP/src/api/api_msg.d \
./Middlewares/Third_Party/LwIP/src/api/err.d \
./Middlewares/Third_Party/LwIP/src/api/netbuf.d \
./Middlewares/Third_Party/LwIP/src/api/netdb.d \
./Middlewares/Third_Party/LwIP/src/api/netifapi.d \
./Middlewares/Third_Party/LwIP/src/api/sockets.d \
./Middlewares/Third_Party/LwIP/src/api/tcpip.d 

OBJS += \
./Middlewares/Third_Party/LwIP/src/api/api_lib.o \
./Middlewares/Third_Party/LwIP/src/api/api_msg.o \
./Middlewares/Third_Party/LwIP/src/api/err.o \
./Middlewares/Third_Party/LwIP/src/api/netbuf.o \
./Middlewares/Third_Party/LwIP/src/api/netdb.o \
./Middlewares/Third_Party/LwIP/src/api/netifapi.o \
./Middlewares/Third_Party/LwIP/src/api/sockets.o \
./Middlewares/Third_Party/LwIP/src/api/tcpip.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/LwIP/src/api/%.o Middlewares/Third_Party/LwIP/src/api/%.su Middlewares/Third_Party/LwIP/src/api/%.cyclo: ../Middlewares/Third_Party/LwIP/src/api/%.c Middlewares/Third_Party/LwIP/src/api/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -DNX_INCLUDE_USER_DEFINE_FILE -DSEND_VERSION=4 -DDECODER_TEST -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/konem/nmradcc/sender/V4-563/Shell" -I"C:/Users/konem/nmradcc/sender/V4-563/Minini" -I"C:/Users/konem/nmradcc/sender/V4-563/Drivers/BSP" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/http" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/telnet" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/tftp" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/web" -I"C:/Users/konem/nmradcc/sender/V4-563/Arch" -I"C:/Users/konem/nmradcc/sender/V4-563/CS/Wangrow" -I"C:/Users/konem/nmradcc/sender/V4-563/CS" -I"C:/Users/konem/nmradcc/sender/V4-563/Send/INCLUDE" -I"C:/Users/konem/nmradcc/sender/V4-563/Send/SRC/SEND" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FatFs/src" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FatFs" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/LwIP/src/include" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Core/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/USB" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/LwIP/system" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-Third_Party-2f-LwIP-2f-src-2f-api

clean-Middlewares-2f-Third_Party-2f-LwIP-2f-src-2f-api:
	-$(RM) ./Middlewares/Third_Party/LwIP/src/api/api_lib.cyclo ./Middlewares/Third_Party/LwIP/src/api/api_lib.d ./Middlewares/Third_Party/LwIP/src/api/api_lib.o ./Middlewares/Third_Party/LwIP/src/api/api_lib.su ./Middlewares/Third_Party/LwIP/src/api/api_msg.cyclo ./Middlewares/Third_Party/LwIP/src/api/api_msg.d ./Middlewares/Third_Party/LwIP/src/api/api_msg.o ./Middlewares/Third_Party/LwIP/src/api/api_msg.su ./Middlewares/Third_Party/LwIP/src/api/err.cyclo ./Middlewares/Third_Party/LwIP/src/api/err.d ./Middlewares/Third_Party/LwIP/src/api/err.o ./Middlewares/Third_Party/LwIP/src/api/err.su ./Middlewares/Third_Party/LwIP/src/api/netbuf.cyclo ./Middlewares/Third_Party/LwIP/src/api/netbuf.d ./Middlewares/Third_Party/LwIP/src/api/netbuf.o ./Middlewares/Third_Party/LwIP/src/api/netbuf.su ./Middlewares/Third_Party/LwIP/src/api/netdb.cyclo ./Middlewares/Third_Party/LwIP/src/api/netdb.d ./Middlewares/Third_Party/LwIP/src/api/netdb.o ./Middlewares/Third_Party/LwIP/src/api/netdb.su ./Middlewares/Third_Party/LwIP/src/api/netifapi.cyclo ./Middlewares/Third_Party/LwIP/src/api/netifapi.d ./Middlewares/Third_Party/LwIP/src/api/netifapi.o ./Middlewares/Third_Party/LwIP/src/api/netifapi.su ./Middlewares/Third_Party/LwIP/src/api/sockets.cyclo ./Middlewares/Third_Party/LwIP/src/api/sockets.d ./Middlewares/Third_Party/LwIP/src/api/sockets.o ./Middlewares/Third_Party/LwIP/src/api/sockets.su ./Middlewares/Third_Party/LwIP/src/api/tcpip.cyclo ./Middlewares/Third_Party/LwIP/src/api/tcpip.d ./Middlewares/Third_Party/LwIP/src/api/tcpip.o ./Middlewares/Third_Party/LwIP/src/api/tcpip.su

.PHONY: clean-Middlewares-2f-Third_Party-2f-LwIP-2f-src-2f-api

