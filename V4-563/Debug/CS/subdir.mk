################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CS/Accessory.c \
../CS/CS.c \
../CS/Cab.c \
../CS/Clock.c \
../CS/LinkedList.c \
../CS/Loco.c \
../CS/MsgQueue.c \
../CS/Packet.c \
../CS/Service.c \
../CS/TrakList.c \
../CS/Uart2.c 

C_DEPS += \
./CS/Accessory.d \
./CS/CS.d \
./CS/Cab.d \
./CS/Clock.d \
./CS/LinkedList.d \
./CS/Loco.d \
./CS/MsgQueue.d \
./CS/Packet.d \
./CS/Service.d \
./CS/TrakList.d \
./CS/Uart2.d 

OBJS += \
./CS/Accessory.o \
./CS/CS.o \
./CS/Cab.o \
./CS/Clock.o \
./CS/LinkedList.o \
./CS/Loco.o \
./CS/MsgQueue.o \
./CS/Packet.o \
./CS/Service.o \
./CS/TrakList.o \
./CS/Uart2.o 


# Each subdirectory must supply rules for building sources it contributes
CS/%.o CS/%.su CS/%.cyclo: ../CS/%.c CS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -DNX_INCLUDE_USER_DEFINE_FILE -DSEND_VERSION=4 -DDECODER_TEST -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/konem/nmradcc/sender/V4-563/Shell" -I"C:/Users/konem/nmradcc/sender/V4-563/Minini" -I"C:/Users/konem/nmradcc/sender/V4-563/Drivers/BSP" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/http" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/telnet" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/tftp" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/web" -I"C:/Users/konem/nmradcc/sender/V4-563/Arch" -I"C:/Users/konem/nmradcc/sender/V4-563/CS/Wangrow" -I"C:/Users/konem/nmradcc/sender/V4-563/CS" -I"C:/Users/konem/nmradcc/sender/V4-563/Send/INCLUDE" -I"C:/Users/konem/nmradcc/sender/V4-563/Send/SRC/SEND" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FatFs/src" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FatFs" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/LwIP/src/include" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Core/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/USB" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/LwIP/system" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-CS

clean-CS:
	-$(RM) ./CS/Accessory.cyclo ./CS/Accessory.d ./CS/Accessory.o ./CS/Accessory.su ./CS/CS.cyclo ./CS/CS.d ./CS/CS.o ./CS/CS.su ./CS/Cab.cyclo ./CS/Cab.d ./CS/Cab.o ./CS/Cab.su ./CS/Clock.cyclo ./CS/Clock.d ./CS/Clock.o ./CS/Clock.su ./CS/LinkedList.cyclo ./CS/LinkedList.d ./CS/LinkedList.o ./CS/LinkedList.su ./CS/Loco.cyclo ./CS/Loco.d ./CS/Loco.o ./CS/Loco.su ./CS/MsgQueue.cyclo ./CS/MsgQueue.d ./CS/MsgQueue.o ./CS/MsgQueue.su ./CS/Packet.cyclo ./CS/Packet.d ./CS/Packet.o ./CS/Packet.su ./CS/Service.cyclo ./CS/Service.d ./CS/Service.o ./CS/Service.su ./CS/TrakList.cyclo ./CS/TrakList.d ./CS/TrakList.o ./CS/TrakList.su ./CS/Uart2.cyclo ./CS/Uart2.d ./CS/Uart2.o ./CS/Uart2.su

.PHONY: clean-CS

