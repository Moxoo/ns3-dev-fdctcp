################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/dsr/test/dsr-test-suite.cc 

CC_DEPS += \
./src/dsr/test/dsr-test-suite.d 

OBJS += \
./src/dsr/test/dsr-test-suite.o 


# Each subdirectory must supply rules for building sources it contributes
src/dsr/test/%.o: ../src/dsr/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


