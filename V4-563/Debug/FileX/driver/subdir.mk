################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FileX/driver/fx_stm32_sd_driver.c 

OBJS += \
./FileX/driver/fx_stm32_sd_driver.o 

C_DEPS += \
./FileX/driver/fx_stm32_sd_driver.d 


# Each subdirectory must supply rules for building sources it contributes
FileX/driver/%.o FileX/driver/%.su FileX/driver/%.cyclo: ../FileX/driver/%.c FileX/driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -DFX_INCLUDE_USER_DEFINE_FILE -DNX_INCLUDE_USER_DEFINE_FILE -DUX_INCLUDE_USER_DEFINE_FILE -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../FileX/App -I../NetXDuo/App -I../USBX/App -I../USBX/Target -I../Middlewares/ST/filex/common/inc -I../Middlewares/ST/filex/ports/generic/inc -I../Middlewares/ST/netxduo/common/inc -I../Middlewares/ST/netxduo/ports/cortex_m33/gnu/inc -I../Middlewares/ST/usbx/common/core/inc -I../Middlewares/ST/usbx/ports/generic/inc -I"C:/Users/konem/nmradcc/sender/V4563/Shell" -I"C:/Users/konem/nmradcc/sender/V4563/Minini" -I"C:/Users/konem/nmradcc/sender/V4563/FileX/driver" -I"C:/Users/konem/nmradcc/sender/V4563/Drivers/BSP" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-FileX-2f-driver

clean-FileX-2f-driver:
	-$(RM) ./FileX/driver/fx_stm32_sd_driver.cyclo ./FileX/driver/fx_stm32_sd_driver.d ./FileX/driver/fx_stm32_sd_driver.o ./FileX/driver/fx_stm32_sd_driver.su

.PHONY: clean-FileX-2f-driver

