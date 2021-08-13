################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../scratch/FIFOvsRED.cc \
../scratch/convergence.cc \
../scratch/dctcpVScubic.cc \
../scratch/incast.cc \
../scratch/intrarack_fct_test.cc \
../scratch/long_short.cc \
../scratch/scratch-simulator.cc \
../scratch/tcp-variants-comparison.cc 

CC_DEPS += \
./scratch/FIFOvsRED.d \
./scratch/convergence.d \
./scratch/dctcpVScubic.d \
./scratch/incast.d \
./scratch/intrarack_fct_test.d \
./scratch/long_short.d \
./scratch/scratch-simulator.d \
./scratch/tcp-variants-comparison.d 

OBJS += \
./scratch/FIFOvsRED.o \
./scratch/convergence.o \
./scratch/dctcpVScubic.o \
./scratch/incast.o \
./scratch/intrarack_fct_test.o \
./scratch/long_short.o \
./scratch/scratch-simulator.o \
./scratch/tcp-variants-comparison.o 


# Each subdirectory must supply rules for building sources it contributes
scratch/%.o: ../scratch/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


