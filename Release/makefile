################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include pcf8574/subdir.mk
-include lcdpcf8574/subdir.mk
-include i2chw/subdir.mk
-include DS1302/subdir.mk
-include 1Wire/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
stacja_pogodowa.lss \

FLASH_IMAGE += \
stacja_pogodowa.hex \

EEPROM_IMAGE += \
stacja_pogodowa.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: stacja_pogodowa.elf secondary-outputs

# Tool invocations
stacja_pogodowa.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,stacja_pogodowa.map -mmcu=atmega328p -o "stacja_pogodowa.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

stacja_pogodowa.lss: stacja_pogodowa.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S stacja_pogodowa.elf  >"stacja_pogodowa.lss"
	@echo 'Finished building: $@'
	@echo ' '

stacja_pogodowa.hex: stacja_pogodowa.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex stacja_pogodowa.elf  "stacja_pogodowa.hex"
	@echo 'Finished building: $@'
	@echo ' '

stacja_pogodowa.eep: stacja_pogodowa.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex stacja_pogodowa.elf  "stacja_pogodowa.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: stacja_pogodowa.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega328p stacja_pogodowa.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) stacja_pogodowa.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents

-include ../makefile.targets
