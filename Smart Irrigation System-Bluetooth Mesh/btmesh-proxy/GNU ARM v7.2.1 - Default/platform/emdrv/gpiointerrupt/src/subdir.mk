################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../platform/emdrv/gpiointerrupt/src/gpiointerrupt.c 

OBJS += \
./platform/emdrv/gpiointerrupt/src/gpiointerrupt.o 

C_DEPS += \
./platform/emdrv/gpiointerrupt/src/gpiointerrupt.d 


# Each subdirectory must supply rules for building sources it contributes
platform/emdrv/gpiointerrupt/src/gpiointerrupt.o: ../platform/emdrv/gpiointerrupt/src/gpiointerrupt.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m4 -mthumb -std=c99 '-DEFR32BG13P632F512GM48=1' '-DMESH_LIB_NATIVE=1' '-DHAL_CONFIG=1' '-D__STACK_SIZE=0x1000' '-D__HEAP_SIZE=0x1200' -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\radio\rail_lib\common" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\hardware\kit\common\halconfig" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\emdrv\gpiointerrupt\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\emlib\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\middleware\glib\glib" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\protocol\bluetooth\bt_mesh\inc\common" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\middleware\glib\dmd\ssd2119" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\CMSIS\Include" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\protocol\bluetooth\bt_mesh\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\protocol\bluetooth\bt_mesh\src" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\Device\SiliconLabs\EFR32BG13P\Include" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\emlib\src" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\protocol\bluetooth\bt_mesh\inc\soc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\hardware\kit\common\bsp" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\hardware\kit\common\drivers" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\hardware\kit\EFR32BG13_BRD4104A\config" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\Device\SiliconLabs\EFR32BG13P\Source\GCC" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\emdrv\uartdrv\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\middleware\glib" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\emdrv\sleep\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\halconfig\inc\hal-config" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\emdrv\gpiointerrupt\src" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\middleware\glib\dmd" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\bootloader\api" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\emdrv\common\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\middleware\glib\dmd\display" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\radio\rail_lib\chip\efr32" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\Device\SiliconLabs\EFR32BG13P\Source" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\soc-btmesh-proxy\platform\emdrv\sleep\src" -Os -fno-builtin -flto -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"platform/emdrv/gpiointerrupt/src/gpiointerrupt.d" -MT"platform/emdrv/gpiointerrupt/src/gpiointerrupt.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


