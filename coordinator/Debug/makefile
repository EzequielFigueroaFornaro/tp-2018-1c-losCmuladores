################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: coordinator

dependents:
	-cd /home/luciana/workspace/tp-2018-1c-losCmuladores/commons-library/Debug && $(MAKE) clean all

# Tool invocations
coordinator: $(OBJS) $(USER_OBJS) /home/luciana/workspace/tp-2018-1c-losCmuladores/commons-library/Debug/libcommons-library.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/luciana/workspace/tp-2018-1c-losCmuladores/commons-library/Debug" -o "coordinator" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) coordinator
	-@echo ' '

.PHONY: all clean dependents
/home/luciana/workspace/tp-2018-1c-losCmuladores/commons-library/Debug/libcommons-library.so:

-include ../makefile.targets
