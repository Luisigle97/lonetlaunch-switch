#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

TARGET      := lonetlaunch
BUILD       := build
SOURCES     := source source/core source/gfx source/sys
DATA        := data
INCLUDES    := source source/core source/gfx source/sys
ROMFS       := romfs

APP_TITLE   := LonetLaunch
APP_AUTHOR  := Custom Launcher
APP_VERSION := 1.0.0

# Arch and optimization flags (Cortex-A57 64-bit ARMv8)
ARCH        := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS      := -Wall -O3 -ffunction-sections -fdata-sections \
               -I$(PORTLIBS)/include/SDL2 -I$(PORTLIBS)/include/freetype2 -I$(PORTLIBS)/include \
               $(ARCH) $(DEFINES)

CFLAGS      += $(INCLUDE) -D__SWITCH__

CXXFLAGS    := $(CFLAGS) -std=gnu++20 -fno-rtti -fexceptions

ASFLAGS     := -g $(ARCH)
LDFLAGS     = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map) -Wl,--gc-sections

LIBS        := -L$(PORTLIBS)/lib -lSDL2_ttf -lSDL2_image -lSDL2 \
               -lharfbuzz -lfreetype -lpng16 -ljpeg -lz -lwebp -lbz2 \
               -lEGL -lGLESv2 -lglapi -ldrm_nouveau \
               -lnx

LIBDIRS     := $(PORTLIBS) $(LIBNX)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)

export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                   $(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES    := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

export LD   := $(CXX)

export OFILES_BIN := $(addsuffix .o,$(BINFILES))
export OFILES_SRC := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES     := $(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN := $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(CONFIG_JSON)),)
    jsons := $(wildcard *.json)
    ifneq (,$(findstring $(TARGET).json,$(jsons)))
        export CONFIG_JSON := $(TARGET).json
    else
        ifneq (,$(findstring config.json,$(jsons)))
            export CONFIG_JSON := config.json
        endif
    endif
endif

ifeq ($(strip $(ICON)),)
    icons := $(wildcard *.jpg)
    ifneq (,$(findstring $(TARGET).jpg,$(icons)))
        export ICON := $(TARGET).jpg
    else
        ifneq (,$(findstring icon.jpg,$(icons)))
            export ICON := icon.jpg
        endif
    endif
endif

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).pfs0 $(TARGET).nso $(TARGET).nro $(TARGET).nacp $(TARGET).elf

else

$(OUTPUT).nro   : $(OUTPUT).elf

$(OUTPUT).elf   : $(OFILES)

$(OFILES_SRC)   : $(HFILES_BIN)

%.o: %.cpp
	@echo $(notdir $<)
	@$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@ $(ERROR_FILTER)

%.o: %.c
	@echo $(notdir $<)
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@ $(ERROR_FILTER)

%.o: %.s
	@echo $(notdir $<)
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d -x assembler-with-cpp $(ASFLAGS) -c $< -o $@ $(ERROR_FILTER)

-include $(DEPSDIR)/*.d

endif
