################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Adi/src/configuration_files/adi_wil_example_cfg_profiles.c 

COMPILED_SRCS += \
./Adi/src/configuration_files/adi_wil_example_cfg_profiles.src 

C_DEPS += \
./Adi/src/configuration_files/adi_wil_example_cfg_profiles.d 

OBJS += \
./Adi/src/configuration_files/adi_wil_example_cfg_profiles.o 


# Each subdirectory must supply rules for building sources it contributes
Adi/src/configuration_files/%.src: ../Adi/src/configuration_files/%.c Adi/src/configuration_files/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING C/C++ Compiler'
	cctc -cs --dep-file="$(basename $@).d" --misrac-version=2004 -D__CPU__=tc38x "-fC:/WORK/AURIX-Workspace_JG_TSW/WBMS_CDD/TriCore Debug (TASKING)/TASKING_C_C___Compiler-Include_paths.opt" --iso=99 --c++14 --language=+volatile --exceptions --anachronisms --fp-model=3 -O0 --tradeoff=4 --compact-max-size=200 -g -Wc-w544 -Wc-w557 -Ctc38x -Y0 -N0 -Z0 -o "$@" "$<" && \
	if [ -f "$(basename $@).d" ]; then sed.exe -r  -e 's/\b(.+\.o)\b/Adi\/src\/configuration_files\/\1/g' -e 's/\\/\//g' -e 's/\/\//\//g' -e 's/"//g' -e 's/([a-zA-Z]:\/)/\L\1/g' -e 's/\d32:/@TARGET_DELIMITER@/g; s/\\\d32/@ESCAPED_SPACE@/g; s/\d32/\\\d32/g; s/@ESCAPED_SPACE@/\\\d32/g; s/@TARGET_DELIMITER@/\d32:/g' "$(basename $@).d" > "$(basename $@).d_sed" && cp "$(basename $@).d_sed" "$(basename $@).d" && rm -f "$(basename $@).d_sed" 2>/dev/null; else echo 'No dependency file to process';fi
	@echo 'Finished building: $<'
	@echo ' '

Adi/src/configuration_files/adi_wil_example_cfg_profiles.o: ./Adi/src/configuration_files/adi_wil_example_cfg_profiles.src Adi/src/configuration_files/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: TASKING Assembler'
	astc -Og -Os --no-warnings= --error-limit=42 -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-Adi-2f-src-2f-configuration_files

clean-Adi-2f-src-2f-configuration_files:
	-$(RM) ./Adi/src/configuration_files/adi_wil_example_cfg_profiles.d ./Adi/src/configuration_files/adi_wil_example_cfg_profiles.o ./Adi/src/configuration_files/adi_wil_example_cfg_profiles.src

.PHONY: clean-Adi-2f-src-2f-configuration_files
