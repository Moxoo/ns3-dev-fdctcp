################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/dsdv/helper/dsdv-helper.cc 

CC_DEPS += \
./src/dsdv/helper/dsdv-helper.d 

OBJS += \
./src/dsdv/helper/dsdv-helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/dsdv/helper/%.o: ../src/dsdv/helper/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


