################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Libraries/iLLD/TC38A/Tricore/_PinMap/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/_Lib/DataHandling/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/_Impl/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Stm/Std/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Src/Std/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Scu/Std/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Qspi/Std/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Qspi/SpiMaster/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Port/Std/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Pms/Std/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Mtu/Std/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Gtm/Std/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Dma/Std/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Dma/Dma/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Cpu/Trap/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Cpu/Std/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Cpu/Irq/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Asclin/Std/subdir.mk
-include Libraries/iLLD/TC38A/Tricore/Asclin/Asc/subdir.mk
-include Libraries/Service/CpuGeneric/SysSe/Bsp/subdir.mk
-include Libraries/Service/CpuGeneric/StdIf/subdir.mk
-include Libraries/Service/CpuGeneric/If/subdir.mk
-include Libraries/Infra/Ssw/TC38A/Tricore/subdir.mk
-include Libraries/Infra/Platform/Tricore/Compilers/subdir.mk
-include Configurations/Debug/subdir.mk
-include Configurations/subdir.mk
-include Cmic/subdir.mk
-include Adi/src/otap_files/subdir.mk
-include Adi/src/container_files/bms_scripts/subdir.mk
-include Adi/src/configuration_files/subdir.mk
-include Adi/src/application/subdir.mk
-include Adi/src/HAL/subdir.mk
-include Adi/src/subdir.mk
-include Adi/WBMS_Interface_Lib-Rel2.2.0/Source/subdir.mk
-include subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := WBMS_CDD
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_SIZE += \
WBMS_CDD.siz \


# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: WBMS_CDD.elf secondary-outputs

# Tool invocations
WBMS_CDD.elf: $(OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: TASKING Linker'
	cctc --lsl-file="../Lcf_Tasking_Tricore_Tc.lsl" -Wl-Oc -Wl-OL -Wl-Ot -Wl-Ox -Wl-Oy -Wl--map-file="WBMS_CDD.map" -Wl-mc -Wl-mf -Wl-mi -Wl-mk -Wl-ml -Wl-mm -Wl-md -Wl-mr -Wl-mu --no-warnings= -Wl--error-limit=42 --fp-model=3 -lrt --lsl-core=vtc --exceptions --strict --anachronisms --force-c++ -Ctc38x -o"WBMS_CDD.elf" -Wl-o"WBMS_CDD.hex:IHEX" $(OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

WBMS_CDD.siz: WBMS_CDD.elf makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: Print Size'
	elfsize  "WBMS_CDD.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	find . ! -name "*.mk" ! -name "makefile" ! -name "." -type f -delete 
	-@echo ' '

post-build:
	-C:\WORK\AURIX-Workspace_JG_TSW\WBMS_CDD\Tool\BuildProcedure\PostBuild.bat
	-@echo ' '

secondary-outputs: $(SECONDARY_SIZE)

.PHONY: all clean dependents main-build post-build

-include ../makefile.targets
