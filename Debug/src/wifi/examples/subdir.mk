################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/wifi/examples/test-interference-helper.cc \
../src/wifi/examples/wifi-manager-example.cc \
../src/wifi/examples/wifi-phy-configuration.cc \
../src/wifi/examples/wifi-phy-test.cc \
../src/wifi/examples/wifi-trans-example.cc 

CC_DEPS += \
./src/wifi/examples/test-interference-helper.d \
./src/wifi/examples/wifi-manager-example.d \
./src/wifi/examples/wifi-phy-configuration.d \
./src/wifi/examples/wifi-phy-test.d \
./src/wifi/examples/wifi-trans-example.d 

OBJS += \
./src/wifi/examples/test-interference-helper.o \
./src/wifi/examples/wifi-manager-example.o \
./src/wifi/examples/wifi-phy-configuration.o \
./src/wifi/examples/wifi-phy-test.o \
./src/wifi/examples/wifi-trans-example.o 


# Each subdirectory must supply rules for building sources it contributes
src/wifi/examples/%.o: ../src/wifi/examples/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


