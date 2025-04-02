################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../NetXDuo/App/app_netxduo.c 

C_DEPS += \
./NetXDuo/App/app_netxduo.d 

OBJS += \
./NetXDuo/App/app_netxduo.o 


# Each subdirectory must supply rules for building sources it contributes
NetXDuo/App/%.o NetXDuo/App/%.su NetXDuo/App/%.cyclo: ../NetXDuo/App/%.c NetXDuo/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -DNX_INCLUDE_USER_DEFINE_FILE -DSEND_VERSION=4 -DDECODER_TEST -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../NetXDuo/App -I../Middlewares/ST/netxduo/common/inc -I../Middlewares/ST/netxduo/ports/cortex_m33/gnu/inc -I"C:/Users/Karl/nmradcc/V4-563/Shell" -I"C:/Users/Karl/nmradcc/V4-563/Minini" -I"C:/Users/Karl/nmradcc/V4-563/Drivers/BSP" -I"C:/Users/Karl/nmradcc/V4-563/common/inc" -I"C:/Users/Karl/nmradcc/V4-563/Net Apps/http" -I"C:/Users/Karl/nmradcc/V4-563/Net Apps/telnet" -I"C:/Users/Karl/nmradcc/V4-563/Net Apps/tftp" -I"C:/Users/Karl/nmradcc/V4-563/Net Apps/web" -I"C:/Users/Karl/nmradcc/V4-563/Arch" -I"C:/Users/Karl/nmradcc/V4-563/FatFs/src" -I"C:/Users/Karl/nmradcc/V4-563/FatFs" -I../NetXDuo/Target -I../Drivers/BSP/Components/lan8742 -I../Middlewares/ST/netxduo/common/drivers/ethernet -I"C:/Users/Karl/nmradcc/V4-563/CS/Wangrow" -I"C:/Users/Karl/nmradcc/V4-563/CS" -I"C:/Users/Karl/nmradcc/V4-563/Send/INCLUDE" -I"C:/Users/Karl/nmradcc/V4-563/Send/SRC/SEND" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-NetXDuo-2f-App

clean-NetXDuo-2f-App:
	-$(RM) ./NetXDuo/App/app_netxduo.cyclo ./NetXDuo/App/app_netxduo.d ./NetXDuo/App/app_netxduo.o ./NetXDuo/App/app_netxduo.su

.PHONY: clean-NetXDuo-2f-App

