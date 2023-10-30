################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../HAL/adi_wil_hal_spi.c \
../HAL/adi_wil_hal_task.c \
../HAL/adi_wil_hal_task_cb.c \
../HAL/adi_wil_hal_ticker.c \
../HAL/adi_wil_hal_ticker_bt.c \
../HAL/adi_wil_hal_tmr.c \
../HAL/adi_wil_osal.c 

COMPILED_SRCS += \
./HAL/adi_wil_hal_spi.src \
./HAL/adi_wil_hal_task.src \
./HAL/adi_wil_hal_task_cb.src \
./HAL/adi_wil_hal_ticker.src \
./HAL/adi_wil_hal_ticker_bt.src \
./HAL/adi_wil_hal_tmr.src \
./HAL/adi_wil_osal.src 

C_DEPS += \
./HAL/adi_wil_hal_spi.d \
./HAL/adi_wil_hal_task.d \
./HAL/adi_wil_hal_task_cb.d \
./HAL/adi_wil_hal_ticker.d \
./HAL/adi_wil_hal_ticker_bt.d \
./HAL/adi_wil_hal_tmr.d \
./HAL/adi_wil_osal.d 

OBJS += \
./HAL/adi_wil_hal_spi.o \
./HAL/adi_wil_hal_task.o \
./HAL/adi_wil_hal_task_cb.o \
./HAL/adi_wil_hal_ticker.o \
./HAL/adi_wil_hal_ticker_bt.o \
./HAL/adi_wil_hal_tmr.o \
./HAL/adi_wil_osal.o 


# Each subdirectory must supply rules for building sources it contributes
HAL/%.src: ../HAL/%.c HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -cs --dep-file="$(basename $@).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/WORK/AURIX-Workspace_JG_TSW/WBMS_CDD/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<" && \
	if [ -f "$(basename $@).d" ]; then sed.exe -r  -e 's/\b(.+\.o)\b/HAL\/\1/g' -e 's/\\/\//g' -e 's/\/\//\//g' -e 's/"//g' -e 's/([a-zA-Z]:\/)/\L\1/g' -e 's/\d32:/@TARGET_DELIMITER@/g; s/\\\d32/@ESCAPED_SPACE@/g; s/\d32/\\\d32/g; s/@ESCAPED_SPACE@/\\\d32/g; s/@TARGET_DELIMITER@/\d32:/g' "$(basename $@).d" > "$(basename $@).d_sed" && cp "$(basename $@).d_sed" "$(basename $@).d" && rm -f "$(basename $@).d_sed" 2>/dev/null; else echo 'No dependency file to process';fi
	@echo 'Finished building: $<'
	@echo ' '

HAL/adi_wil_hal_spi.o: ./HAL/adi_wil_hal_spi.src HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

HAL/adi_wil_hal_task.o: ./HAL/adi_wil_hal_task.src HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

HAL/adi_wil_hal_task_cb.o: ./HAL/adi_wil_hal_task_cb.src HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

HAL/adi_wil_hal_ticker.o: ./HAL/adi_wil_hal_ticker.src HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

HAL/adi_wil_hal_ticker_bt.o: ./HAL/adi_wil_hal_ticker_bt.src HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

HAL/adi_wil_hal_tmr.o: ./HAL/adi_wil_hal_tmr.src HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

HAL/adi_wil_osal.o: ./HAL/adi_wil_osal.src HAL/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-HAL

clean-HAL:
	-$(RM) ./HAL/adi_wil_hal_spi.d ./HAL/adi_wil_hal_spi.o ./HAL/adi_wil_hal_spi.src ./HAL/adi_wil_hal_task.d ./HAL/adi_wil_hal_task.o ./HAL/adi_wil_hal_task.src ./HAL/adi_wil_hal_task_cb.d ./HAL/adi_wil_hal_task_cb.o ./HAL/adi_wil_hal_task_cb.src ./HAL/adi_wil_hal_ticker.d ./HAL/adi_wil_hal_ticker.o ./HAL/adi_wil_hal_ticker.src ./HAL/adi_wil_hal_ticker_bt.d ./HAL/adi_wil_hal_ticker_bt.o ./HAL/adi_wil_hal_ticker_bt.src ./HAL/adi_wil_hal_tmr.d ./HAL/adi_wil_hal_tmr.o ./HAL/adi_wil_hal_tmr.src ./HAL/adi_wil_osal.d ./HAL/adi_wil_osal.o ./HAL/adi_wil_osal.src

.PHONY: clean-HAL

