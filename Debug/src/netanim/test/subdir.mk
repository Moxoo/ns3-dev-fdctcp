################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/netanim/test/netanim-test.cc 

CC_DEPS += \
./src/netanim/test/netanim-test.d 

OBJS += \
./src/netanim/test/netanim-test.o 


# Each subdirectory must supply rules for building sources it contributes
src/netanim/test/%.o: ../src/netanim/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


