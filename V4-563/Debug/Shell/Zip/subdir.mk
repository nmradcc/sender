################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Shell/Zip/bits.c \
../Shell/Zip/crypt.c \
../Shell/Zip/deflate.c \
../Shell/Zip/getopt.c \
../Shell/Zip/gzip.c \
../Shell/Zip/inflate.c \
../Shell/Zip/lzw.c \
../Shell/Zip/trees.c \
../Shell/Zip/unlzh.c \
../Shell/Zip/unlzw.c \
../Shell/Zip/unpack.c \
../Shell/Zip/unzip.c \
../Shell/Zip/util.c \
../Shell/Zip/zip.c 

OBJS += \
./Shell/Zip/bits.o \
./Shell/Zip/crypt.o \
./Shell/Zip/deflate.o \
./Shell/Zip/getopt.o \
./Shell/Zip/gzip.o \
./Shell/Zip/inflate.o \
./Shell/Zip/lzw.o \
./Shell/Zip/trees.o \
./Shell/Zip/unlzh.o \
./Shell/Zip/unlzw.o \
./Shell/Zip/unpack.o \
./Shell/Zip/unzip.o \
./Shell/Zip/util.o \
./Shell/Zip/zip.o 

C_DEPS += \
./Shell/Zip/bits.d \
./Shell/Zip/crypt.d \
./Shell/Zip/deflate.d \
./Shell/Zip/getopt.d \
./Shell/Zip/gzip.d \
./Shell/Zip/inflate.d \
./Shell/Zip/lzw.d \
./Shell/Zip/trees.d \
./Shell/Zip/unlzh.d \
./Shell/Zip/unlzw.d \
./Shell/Zip/unpack.d \
./Shell/Zip/unzip.d \
./Shell/Zip/util.d \
./Shell/Zip/zip.d 


# Each subdirectory must supply rules for building sources it contributes
Shell/Zip/%.o Shell/Zip/%.su Shell/Zip/%.cyclo: ../Shell/Zip/%.c Shell/Zip/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H563xx -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -DFX_INCLUDE_USER_DEFINE_FILE -DNX_INCLUDE_USER_DEFINE_FILE -DUX_INCLUDE_USER_DEFINE_FILE -c -I../Core/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc -I../Drivers/STM32H5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H5xx/Include -I../Drivers/CMSIS/Include -I../AZURE_RTOS/App -I../Middlewares/ST/threadx/common/inc -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../FileX/App -I../NetXDuo/App -I../USBX/App -I../USBX/Target -I../Middlewares/ST/filex/common/inc -I../Middlewares/ST/filex/ports/generic/inc -I../Middlewares/ST/netxduo/common/inc -I../Middlewares/ST/netxduo/ports/cortex_m33/gnu/inc -I../Middlewares/ST/usbx/common/core/inc -I../Middlewares/ST/usbx/ports/generic/inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Shell-2f-Zip

clean-Shell-2f-Zip:
	-$(RM) ./Shell/Zip/bits.cyclo ./Shell/Zip/bits.d ./Shell/Zip/bits.o ./Shell/Zip/bits.su ./Shell/Zip/crypt.cyclo ./Shell/Zip/crypt.d ./Shell/Zip/crypt.o ./Shell/Zip/crypt.su ./Shell/Zip/deflate.cyclo ./Shell/Zip/deflate.d ./Shell/Zip/deflate.o ./Shell/Zip/deflate.su ./Shell/Zip/getopt.cyclo ./Shell/Zip/getopt.d ./Shell/Zip/getopt.o ./Shell/Zip/getopt.su ./Shell/Zip/gzip.cyclo ./Shell/Zip/gzip.d ./Shell/Zip/gzip.o ./Shell/Zip/gzip.su ./Shell/Zip/inflate.cyclo ./Shell/Zip/inflate.d ./Shell/Zip/inflate.o ./Shell/Zip/inflate.su ./Shell/Zip/lzw.cyclo ./Shell/Zip/lzw.d ./Shell/Zip/lzw.o ./Shell/Zip/lzw.su ./Shell/Zip/trees.cyclo ./Shell/Zip/trees.d ./Shell/Zip/trees.o ./Shell/Zip/trees.su ./Shell/Zip/unlzh.cyclo ./Shell/Zip/unlzh.d ./Shell/Zip/unlzh.o ./Shell/Zip/unlzh.su ./Shell/Zip/unlzw.cyclo ./Shell/Zip/unlzw.d ./Shell/Zip/unlzw.o ./Shell/Zip/unlzw.su ./Shell/Zip/unpack.cyclo ./Shell/Zip/unpack.d ./Shell/Zip/unpack.o ./Shell/Zip/unpack.su ./Shell/Zip/unzip.cyclo ./Shell/Zip/unzip.d ./Shell/Zip/unzip.o ./Shell/Zip/unzip.su ./Shell/Zip/util.cyclo ./Shell/Zip/util.d ./Shell/Zip/util.o ./Shell/Zip/util.su ./Shell/Zip/zip.cyclo ./Shell/Zip/zip.d ./Shell/Zip/zip.o ./Shell/Zip/zip.su

.PHONY: clean-Shell-2f-Zip

