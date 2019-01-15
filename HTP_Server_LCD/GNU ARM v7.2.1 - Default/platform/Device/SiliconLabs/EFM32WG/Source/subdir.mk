################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../platform/Device/SiliconLabs/EFM32WG/Source/system_efm32wg.c 

OBJS += \
./platform/Device/SiliconLabs/EFM32WG/Source/system_efm32wg.o 

C_DEPS += \
./platform/Device/SiliconLabs/EFM32WG/Source/system_efm32wg.d 


# Each subdirectory must supply rules for building sources it contributes
platform/Device/SiliconLabs/EFM32WG/Source/system_efm32wg.o: ../platform/Device/SiliconLabs/EFM32WG/Source/system_efm32wg.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m4 -mthumb -std=c99 '-DEFR32BG13P632F512GM48=1' '-D__StackLimit=0x20000000' '-DHAL_CONFIG=1' '-D__STACK_SIZE=0x800' '-D__HEAP_SIZE=0xD00' -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\Device\SiliconLabs\EFR32BG13P\Source" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\CMSIS\Include" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\emlib\src" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\hardware\kit\common\drivers" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\emlib\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\Device\SiliconLabs\EFR32BG13P\Include" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\hardware\kit\common\halconfig" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\protocol\bluetooth\ble_stack\inc\common" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\hardware\kit\common\bsp" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\Device\SiliconLabs\EFR32BG13P\Source\GCC" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\protocol\bluetooth\ble_stack\inc\soc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\radio\rail_lib\common" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\radio\rail_lib\chip\efr32" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\emdrv\uartdrv\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\emdrv\gpiointerrupt\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\hardware\kit\EFR32BG13_BRD4104A\config" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\emdrv\sleep\src" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\emdrv\common\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\emdrv\sleep\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\halconfig\inc\hal-config" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\app\bluetooth\common\stack_bridge" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\lcdGraphics" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.3//platform/middleware/glib" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.3//platform/middleware/glib/glib" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.3//platform/middleware/glib/dmd" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.3//util/silicon_labs/silabs_core/graphics" -I"C:/SiliconLabs/SimplicityStudio/v4/developer/sdks/gecko_sdk_suite/v2.3//hardware/kit/common/drivers" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\emdrv\dmadrv" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\emdrv\dmadrv\inc" -I"C:\Users\hardi\SimplicityStudio\v4_workspace\HTP_Server_LCD\platform\emdrv\dmadrv\src" -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"platform/Device/SiliconLabs/EFM32WG/Source/system_efm32wg.d" -MT"platform/Device/SiliconLabs/EFM32WG/Source/system_efm32wg.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


