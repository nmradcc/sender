################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.c 

C_DEPS += \
./Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.d 

OBJS += \
./Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.o 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/ST/netxduo/common/drivers/ethernet/%.o Middlewares/ST/netxduo/common/drivers/ethernet/%.su Middlewares/ST/netxduo/common/drivers/ethernet/%.cyclo: ../Middlewares/ST/netxduo/common/drivers/ethernet/%.c Middlewares/ST/netxduo/common/drivers/ethernet/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -DNX_INCLUDE_USER_DEFINE_FILE -DSEND_VERSION=4 -DDECODER_TEST -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/konem/nmradcc/sender/V4-563/Shell" -I"C:/Users/konem/nmradcc/sender/V4-563/Minini" -I"C:/Users/konem/nmradcc/sender/V4-563/Drivers/BSP" -I"C:/Users/konem/nmradcc/sender/V4-563/common/inc" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/http" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/telnet" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/tftp" -I"C:/Users/konem/nmradcc/sender/V4-563/Net Apps/web" -I"C:/Users/konem/nmradcc/sender/V4-563/Arch" -I"C:/Users/konem/nmradcc/sender/V4-563/CS/Wangrow" -I"C:/Users/konem/nmradcc/sender/V4-563/CS" -I"C:/Users/konem/nmradcc/sender/V4-563/Send/INCLUDE" -I"C:/Users/konem/nmradcc/sender/V4-563/Send/SRC/SEND" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FatFs/src" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FatFs" -I"C:/Users/konem/nmradcc/sender/V4-563/Middlewares/Third_Party/FreeRTOS/Source/include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-ST-2f-netxduo-2f-common-2f-drivers-2f-ethernet

clean-Middlewares-2f-ST-2f-netxduo-2f-common-2f-drivers-2f-ethernet:
	-$(RM) ./Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.cyclo ./Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.d ./Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.o ./Middlewares/ST/netxduo/common/drivers/ethernet/nx_stm32_eth_driver.su

.PHONY: clean-Middlewares-2f-ST-2f-netxduo-2f-common-2f-drivers-2f-ethernet

