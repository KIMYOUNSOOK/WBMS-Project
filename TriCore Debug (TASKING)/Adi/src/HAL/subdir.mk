################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Adi/src/HAL/adi_wil_hal_spi.c \
../Adi/src/HAL/adi_wil_hal_task.c \
../Adi/src/HAL/adi_wil_hal_task_cb.c \
../Adi/src/HAL/adi_wil_hal_ticker.c \
../Adi/src/HAL/adi_wil_hal_ticker_bt.c \
../Adi/src/HAL/adi_wil_hal_tmr.c \
../Adi/src/HAL/adi_wil_osal.c 

COMPILED_SRCS += \
./Adi/src/HAL/adi_wil_hal_spi.src \
./Adi/src/HAL/adi_wil_hal_task.src \
./Adi/src/HAL/adi_wil_hal_task_cb.src \
./Adi/src/HAL/adi_wil_hal_ticker.src \
./Adi/src/HAL/adi_wil_hal_ticker_bt.src \
./Adi/src/HAL/adi_wil_hal_tmr.src \
./Adi/src/HAL/adi_wil_osal.src 

C_DEPS += \
./Adi/src/HAL/adi_wil_hal_spi.d \
./Adi/src/HAL/adi_wil_hal_task.d \
./Adi/src/HAL/adi_wil_hal_task_cb.d \
./Adi/src/HAL/adi_wil_hal_ticker.d \
./Adi/src/HAL/adi_wil_hal_ticker_bt.d \
./Adi/src/HAL/adi_wil_hal_tmr.d \
./Adi/src/HAL/adi_wil_osal.d 

OBJS += \
./Adi/src/HAL/adi_wil_hal_spi.o \
./Adi/src/HAL/adi_wil_hal_task.o \
./Adi/src/HAL/adi_wil_hal_task_cb.o \
./Adi/src/HAL/adi_wil_hal_ticker.o \
./Adi/src/HAL/adi_wil_hal_ticker_bt.o \
./Adi/src/HAL/adi_wil_hal_tmr.o \
./Adi/src/HAL/adi_wil_osal.o 


# Each subdirectory must supply rules for building sources it contributes
Adi/src/HAL/%.src: ../Adi/src/HAL/%.c Adi/src/HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -cs --dep-file="$(basename $@).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/WORK/AURIX-Workspace_JG_TSW/WBMS_CDD/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<" && \
	if [ -f "$(basename $@).d" ]; then sed.exe -r  -e 's/\b(.+\.o)\b/Adi\/src\/HAL\/\1/g' -e 's/\\/\//g' -e 's/\/\//\//g' -e 's/"//g' -e 's/([a-zA-Z]:\/)/\L\1/g' -e 's/\d32:/@TARGET_DELIMITER@/g; s/\\\d32/@ESCAPED_SPACE@/g; s/\d32/\\\d32/g; s/@ESCAPED_SPACE@/\\\d32/g; s/@TARGET_DELIMITER@/\d32:/g' "$(basename $@).d" > "$(basename $@).d_sed" && cp "$(basename $@).d_sed" "$(basename $@).d" && rm -f "$(basename $@).d_sed" 2>/dev/null; else echo 'No dependency file to process';fi
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/HAL/adi_wil_hal_spi.o: ./Adi/src/HAL/adi_wil_hal_spi.src Adi/src/HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/HAL/adi_wil_hal_task.o: ./Adi/src/HAL/adi_wil_hal_task.src Adi/src/HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/HAL/adi_wil_hal_task_cb.o: ./Adi/src/HAL/adi_wil_hal_task_cb.src Adi/src/HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/HAL/adi_wil_hal_ticker.o: ./Adi/src/HAL/adi_wil_hal_ticker.src Adi/src/HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/HAL/adi_wil_hal_ticker_bt.o: ./Adi/src/HAL/adi_wil_hal_ticker_bt.src Adi/src/HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/HAL/adi_wil_hal_tmr.o: ./Adi/src/HAL/adi_wil_hal_tmr.src Adi/src/HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/HAL/adi_wil_osal.o: ./Adi/src/HAL/adi_wil_osal.src Adi/src/HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-Adi-2f-src-2f-HAL

clean-Adi-2f-src-2f-HAL:
	-$(RM) ./Adi/src/HAL/adi_wil_hal_spi.d ./Adi/src/HAL/adi_wil_hal_spi.o ./Adi/src/HAL/adi_wil_hal_spi.src ./Adi/src/HAL/adi_wil_hal_task.d ./Adi/src/HAL/adi_wil_hal_task.o ./Adi/src/HAL/adi_wil_hal_task.src ./Adi/src/HAL/adi_wil_hal_task_cb.d ./Adi/src/HAL/adi_wil_hal_task_cb.o ./Adi/src/HAL/adi_wil_hal_task_cb.src ./Adi/src/HAL/adi_wil_hal_ticker.d ./Adi/src/HAL/adi_wil_hal_ticker.o ./Adi/src/HAL/adi_wil_hal_ticker.src ./Adi/src/HAL/adi_wil_hal_ticker_bt.d ./Adi/src/HAL/adi_wil_hal_ticker_bt.o ./Adi/src/HAL/adi_wil_hal_ticker_bt.src ./Adi/src/HAL/adi_wil_hal_tmr.d ./Adi/src/HAL/adi_wil_hal_tmr.o ./Adi/src/HAL/adi_wil_hal_tmr.src ./Adi/src/HAL/adi_wil_osal.d ./Adi/src/HAL/adi_wil_osal.o ./Adi/src/HAL/adi_wil_osal.src

.PHONY: clean-Adi-2f-src-2f-HAL

