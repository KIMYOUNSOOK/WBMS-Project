################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Adi/src/application/adi_wil_example.c \
../Adi/src/application/adi_wil_example_PSFromLatency.c \
../Adi/src/application/adi_wil_example_acl.c \
../Adi/src/application/adi_wil_example_cell_balance.c \
../Adi/src/application/adi_wil_example_debug_functions.c \
../Adi/src/application/adi_wil_example_functions.c \
../Adi/src/application/adi_wil_example_owd.c \
../Adi/src/application/adi_wil_example_printf.c \
../Adi/src/application/adi_wil_example_scheduler.c \
../Adi/src/application/adi_wil_example_utilities.c 

COMPILED_SRCS += \
./Adi/src/application/adi_wil_example.src \
./Adi/src/application/adi_wil_example_PSFromLatency.src \
./Adi/src/application/adi_wil_example_acl.src \
./Adi/src/application/adi_wil_example_cell_balance.src \
./Adi/src/application/adi_wil_example_debug_functions.src \
./Adi/src/application/adi_wil_example_functions.src \
./Adi/src/application/adi_wil_example_owd.src \
./Adi/src/application/adi_wil_example_printf.src \
./Adi/src/application/adi_wil_example_scheduler.src \
./Adi/src/application/adi_wil_example_utilities.src 

C_DEPS += \
./Adi/src/application/adi_wil_example.d \
./Adi/src/application/adi_wil_example_PSFromLatency.d \
./Adi/src/application/adi_wil_example_acl.d \
./Adi/src/application/adi_wil_example_cell_balance.d \
./Adi/src/application/adi_wil_example_debug_functions.d \
./Adi/src/application/adi_wil_example_functions.d \
./Adi/src/application/adi_wil_example_owd.d \
./Adi/src/application/adi_wil_example_printf.d \
./Adi/src/application/adi_wil_example_scheduler.d \
./Adi/src/application/adi_wil_example_utilities.d 

OBJS += \
./Adi/src/application/adi_wil_example.o \
./Adi/src/application/adi_wil_example_PSFromLatency.o \
./Adi/src/application/adi_wil_example_acl.o \
./Adi/src/application/adi_wil_example_cell_balance.o \
./Adi/src/application/adi_wil_example_debug_functions.o \
./Adi/src/application/adi_wil_example_functions.o \
./Adi/src/application/adi_wil_example_owd.o \
./Adi/src/application/adi_wil_example_printf.o \
./Adi/src/application/adi_wil_example_scheduler.o \
./Adi/src/application/adi_wil_example_utilities.o 


# Each subdirectory must supply rules for building sources it contributes
Adi/src/application/%.src: ../Adi/src/application/%.c Adi/src/application/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -cs --dep-file="$(basename $@).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/WORK/AURIX-Workspace_JG_TSW/WBMS_CDD/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<" && \
	if [ -f "$(basename $@).d" ]; then sed.exe -r  -e 's/\b(.+\.o)\b/Adi\/src\/application\/\1/g' -e 's/\\/\//g' -e 's/\/\//\//g' -e 's/"//g' -e 's/([a-zA-Z]:\/)/\L\1/g' -e 's/\d32:/@TARGET_DELIMITER@/g; s/\\\d32/@ESCAPED_SPACE@/g; s/\d32/\\\d32/g; s/@ESCAPED_SPACE@/\\\d32/g; s/@TARGET_DELIMITER@/\d32:/g' "$(basename $@).d" > "$(basename $@).d_sed" && cp "$(basename $@).d_sed" "$(basename $@).d" && rm -f "$(basename $@).d_sed" 2>/dev/null; else echo 'No dependency file to process';fi
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/application/adi_wil_example.o: ./Adi/src/application/adi_wil_example.src Adi/src/application/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/application/adi_wil_example_PSFromLatency.o: ./Adi/src/application/adi_wil_example_PSFromLatency.src Adi/src/application/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/application/adi_wil_example_acl.o: ./Adi/src/application/adi_wil_example_acl.src Adi/src/application/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/application/adi_wil_example_cell_balance.o: ./Adi/src/application/adi_wil_example_cell_balance.src Adi/src/application/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/application/adi_wil_example_debug_functions.o: ./Adi/src/application/adi_wil_example_debug_functions.src Adi/src/application/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/application/adi_wil_example_functions.o: ./Adi/src/application/adi_wil_example_functions.src Adi/src/application/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/application/adi_wil_example_owd.o: ./Adi/src/application/adi_wil_example_owd.src Adi/src/application/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/application/adi_wil_example_printf.o: ./Adi/src/application/adi_wil_example_printf.src Adi/src/application/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/application/adi_wil_example_scheduler.o: ./Adi/src/application/adi_wil_example_scheduler.src Adi/src/application/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/application/adi_wil_example_utilities.o: ./Adi/src/application/adi_wil_example_utilities.src Adi/src/application/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-Adi-2f-src-2f-application

clean-Adi-2f-src-2f-application:
	-$(RM) ./Adi/src/application/adi_wil_example.d ./Adi/src/application/adi_wil_example.o ./Adi/src/application/adi_wil_example.src ./Adi/src/application/adi_wil_example_PSFromLatency.d ./Adi/src/application/adi_wil_example_PSFromLatency.o ./Adi/src/application/adi_wil_example_PSFromLatency.src ./Adi/src/application/adi_wil_example_acl.d ./Adi/src/application/adi_wil_example_acl.o ./Adi/src/application/adi_wil_example_acl.src ./Adi/src/application/adi_wil_example_cell_balance.d ./Adi/src/application/adi_wil_example_cell_balance.o ./Adi/src/application/adi_wil_example_cell_balance.src ./Adi/src/application/adi_wil_example_debug_functions.d ./Adi/src/application/adi_wil_example_debug_functions.o ./Adi/src/application/adi_wil_example_debug_functions.src ./Adi/src/application/adi_wil_example_functions.d ./Adi/src/application/adi_wil_example_functions.o ./Adi/src/application/adi_wil_example_functions.src ./Adi/src/application/adi_wil_example_owd.d ./Adi/src/application/adi_wil_example_owd.o ./Adi/src/application/adi_wil_example_owd.src ./Adi/src/application/adi_wil_example_printf.d ./Adi/src/application/adi_wil_example_printf.o ./Adi/src/application/adi_wil_example_printf.src ./Adi/src/application/adi_wil_example_scheduler.d ./Adi/src/application/adi_wil_example_scheduler.o ./Adi/src/application/adi_wil_example_scheduler.src ./Adi/src/application/adi_wil_example_utilities.d ./Adi/src/application/adi_wil_example_utilities.o ./Adi/src/application/adi_wil_example_utilities.src

.PHONY: clean-Adi-2f-src-2f-application

