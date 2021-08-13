################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../examples/large-scale-sim/large-scale-sim.cc 

C_SRCS += \
../examples/large-scale-sim/cdf.c 

CC_DEPS += \
./examples/large-scale-sim/large-scale-sim.d 

OBJS += \
./examples/large-scale-sim/cdf.o \
./examples/large-scale-sim/large-scale-sim.o 

C_DEPS += \
./examples/large-scale-sim/cdf.d 


# Each subdirectory must supply rules for building sources it contributes
examples/large-scale-sim/%.o: ../examples/large-scale-sim/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

examples/large-scale-sim/%.o: ../examples/large-scale-sim/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


