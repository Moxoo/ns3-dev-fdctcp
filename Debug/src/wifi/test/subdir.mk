################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/wifi/test/block-ack-test-suite.cc \
../src/wifi/test/channel-access-manager-test.cc \
../src/wifi/test/inter-bss-test-suite.cc \
../src/wifi/test/power-rate-adaptation-test.cc \
../src/wifi/test/spectrum-wifi-phy-test.cc \
../src/wifi/test/tx-duration-test.cc \
../src/wifi/test/wifi-aggregation-test.cc \
../src/wifi/test/wifi-error-rate-models-test.cc \
../src/wifi/test/wifi-phy-reception-test.cc \
../src/wifi/test/wifi-phy-thresholds-test.cc \
../src/wifi/test/wifi-test.cc \
../src/wifi/test/wifi-transmit-mask-test.cc 

C_SRCS += \
../src/wifi/test/80211b.c 

CC_DEPS += \
./src/wifi/test/block-ack-test-suite.d \
./src/wifi/test/channel-access-manager-test.d \
./src/wifi/test/inter-bss-test-suite.d \
./src/wifi/test/power-rate-adaptation-test.d \
./src/wifi/test/spectrum-wifi-phy-test.d \
./src/wifi/test/tx-duration-test.d \
./src/wifi/test/wifi-aggregation-test.d \
./src/wifi/test/wifi-error-rate-models-test.d \
./src/wifi/test/wifi-phy-reception-test.d \
./src/wifi/test/wifi-phy-thresholds-test.d \
./src/wifi/test/wifi-test.d \
./src/wifi/test/wifi-transmit-mask-test.d 

OBJS += \
./src/wifi/test/80211b.o \
./src/wifi/test/block-ack-test-suite.o \
./src/wifi/test/channel-access-manager-test.o \
./src/wifi/test/inter-bss-test-suite.o \
./src/wifi/test/power-rate-adaptation-test.o \
./src/wifi/test/spectrum-wifi-phy-test.o \
./src/wifi/test/tx-duration-test.o \
./src/wifi/test/wifi-aggregation-test.o \
./src/wifi/test/wifi-error-rate-models-test.o \
./src/wifi/test/wifi-phy-reception-test.o \
./src/wifi/test/wifi-phy-thresholds-test.o \
./src/wifi/test/wifi-test.o \
./src/wifi/test/wifi-transmit-mask-test.o 

C_DEPS += \
./src/wifi/test/80211b.d 


# Each subdirectory must supply rules for building sources it contributes
src/wifi/test/%.o: ../src/wifi/test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/wifi/test/%.o: ../src/wifi/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


