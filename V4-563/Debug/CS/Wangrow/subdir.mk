################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CS/Wangrow/CabNCE.c \
../CS/Wangrow/Editor.c \
../CS/Wangrow/M_EStop.c \
../CS/Wangrow/M_Function.c \
../CS/Wangrow/M_Idle.c \
../CS/Wangrow/M_NoFunction.c \
../CS/Wangrow/M_OpsProgram.c \
../CS/Wangrow/M_ProgMode.c \
../CS/Wangrow/M_ProgramTrack.c \
../CS/Wangrow/M_SelectAccessory.c \
../CS/Wangrow/M_SelectLoco.c \
../CS/Wangrow/M_Send.c \
../CS/Wangrow/M_Track.c \
../CS/Wangrow/Menu.c \
../CS/Wangrow/Operate.c \
../CS/Wangrow/STATES.c \
../CS/Wangrow/Text.c 

C_DEPS += \
./CS/Wangrow/CabNCE.d \
./CS/Wangrow/Editor.d \
./CS/Wangrow/M_EStop.d \
./CS/Wangrow/M_Function.d \
./CS/Wangrow/M_Idle.d \
./CS/Wangrow/M_NoFunction.d \
./CS/Wangrow/M_OpsProgram.d \
./CS/Wangrow/M_ProgMode.d \
./CS/Wangrow/M_ProgramTrack.d \
./CS/Wangrow/M_SelectAccessory.d \
./CS/Wangrow/M_SelectLoco.d \
./CS/Wangrow/M_Send.d \
./CS/Wangrow/M_Track.d \
./CS/Wangrow/Menu.d \
./CS/Wangrow/Operate.d \
./CS/Wangrow/STATES.d \
./CS/Wangrow/Text.d 

OBJS += \
./CS/Wangrow/CabNCE.o \
./CS/Wangrow/Editor.o \
./CS/Wangrow/M_EStop.o \
./CS/Wangrow/M_Function.o \
./CS/Wangrow/M_Idle.o \
./CS/Wangrow/M_NoFunction.o \
./CS/Wangrow/M_OpsProgram.o \
./CS/Wangrow/M_ProgMode.o \
./CS/Wangrow/M_ProgramTrack.o \
./CS/Wangrow/M_SelectAccessory.o \
./CS/Wangrow/M_SelectLoco.o \
./CS/Wangrow/M_Send.o \
./CS/Wangrow/M_Track.o \
./CS/Wangrow/Menu.o \
./CS/Wangrow/Operate.o \
./CS/Wangrow/STATES.o \
./CS/Wangrow/Text.o 


# Each subdirectory must supply rules for building sources it contributes
CS/Wangrow/%.o CS/Wangrow/%.su CS/Wangrow/%.cyclo: ../CS/Wangrow/%.c CS/Wangrow/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -DNX_INCLUDE_USER_DEFINE_FILE -DSEND_VERSION=4 -DDECODER_TEST -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/konem/nmradcc/sender/V4-563/Shell" -I"C:/Users/konem/nmradcc/sender/V4-563/Minini" -I"C:/Users/konem/nmradcc/sender/V4-563/Drivers/BSP" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/http" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/telnet" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/tftp" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/web" -I"C:/Users/konem/nmradcc/sender/V4-563/Arch" -I"C:/Users/konem/nmradcc/sender/V4-563/CS/Wangrow" -I"C:/Users/konem/nmradcc/sender/V4-563/CS" -I"C:/Users/konem/nmradcc/sender/V4-563/Send/INCLUDE" -I"C:/Users/konem/nmradcc/sender/V4-563/Send/SRC/SEND" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FatFs/src" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FatFs" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/LwIP/src/include" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Core/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc" -I"C:/Users/konem/nmradcc/sender/V4-563/USB" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/LwIP/system" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-CS-2f-Wangrow

clean-CS-2f-Wangrow:
	-$(RM) ./CS/Wangrow/CabNCE.cyclo ./CS/Wangrow/CabNCE.d ./CS/Wangrow/CabNCE.o ./CS/Wangrow/CabNCE.su ./CS/Wangrow/Editor.cyclo ./CS/Wangrow/Editor.d ./CS/Wangrow/Editor.o ./CS/Wangrow/Editor.su ./CS/Wangrow/M_EStop.cyclo ./CS/Wangrow/M_EStop.d ./CS/Wangrow/M_EStop.o ./CS/Wangrow/M_EStop.su ./CS/Wangrow/M_Function.cyclo ./CS/Wangrow/M_Function.d ./CS/Wangrow/M_Function.o ./CS/Wangrow/M_Function.su ./CS/Wangrow/M_Idle.cyclo ./CS/Wangrow/M_Idle.d ./CS/Wangrow/M_Idle.o ./CS/Wangrow/M_Idle.su ./CS/Wangrow/M_NoFunction.cyclo ./CS/Wangrow/M_NoFunction.d ./CS/Wangrow/M_NoFunction.o ./CS/Wangrow/M_NoFunction.su ./CS/Wangrow/M_OpsProgram.cyclo ./CS/Wangrow/M_OpsProgram.d ./CS/Wangrow/M_OpsProgram.o ./CS/Wangrow/M_OpsProgram.su ./CS/Wangrow/M_ProgMode.cyclo ./CS/Wangrow/M_ProgMode.d ./CS/Wangrow/M_ProgMode.o ./CS/Wangrow/M_ProgMode.su ./CS/Wangrow/M_ProgramTrack.cyclo ./CS/Wangrow/M_ProgramTrack.d ./CS/Wangrow/M_ProgramTrack.o ./CS/Wangrow/M_ProgramTrack.su ./CS/Wangrow/M_SelectAccessory.cyclo ./CS/Wangrow/M_SelectAccessory.d ./CS/Wangrow/M_SelectAccessory.o ./CS/Wangrow/M_SelectAccessory.su ./CS/Wangrow/M_SelectLoco.cyclo ./CS/Wangrow/M_SelectLoco.d ./CS/Wangrow/M_SelectLoco.o ./CS/Wangrow/M_SelectLoco.su ./CS/Wangrow/M_Send.cyclo ./CS/Wangrow/M_Send.d ./CS/Wangrow/M_Send.o ./CS/Wangrow/M_Send.su ./CS/Wangrow/M_Track.cyclo ./CS/Wangrow/M_Track.d ./CS/Wangrow/M_Track.o ./CS/Wangrow/M_Track.su ./CS/Wangrow/Menu.cyclo ./CS/Wangrow/Menu.d ./CS/Wangrow/Menu.o ./CS/Wangrow/Menu.su ./CS/Wangrow/Operate.cyclo ./CS/Wangrow/Operate.d ./CS/Wangrow/Operate.o ./CS/Wangrow/Operate.su ./CS/Wangrow/STATES.cyclo ./CS/Wangrow/STATES.d ./CS/Wangrow/STATES.o ./CS/Wangrow/STATES.su ./CS/Wangrow/Text.cyclo ./CS/Wangrow/Text.d ./CS/Wangrow/Text.o ./CS/Wangrow/Text.su

.PHONY: clean-CS-2f-Wangrow

