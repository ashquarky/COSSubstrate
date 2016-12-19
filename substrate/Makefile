
.SUFFIXES:

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPRO")
endif

export BUILD	= build

export PATH		:=$(DEVKITPPC)/bin:$(PORTLIBS)/bin:$(PATH)

export PREFIX	= powerpc-eabi-
export CC		= $(PREFIX)gcc
export LD		= $(PREFIX)ld

export SOURCES	= src
export TARGET	= substrate.cosm

export CFLAGS	= -c -fPIC
export LDFLAGS	= -fPIC -shared

export CFILES	= $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
export OFILES	= $(foreach cfl,$(CFILES),$(BUILD)/$(dir $(cfl))$(basename $(notdir $(cfl))).o)

all: setup $(TARGET)

setup:
	@echo Compiling...
	@mkdir -p $(BUILD)/$(SOURCES)

$(TARGET): $(OFILES)
	@echo Linking $(TARGET)...
	@$(LD) $(LDFLAGS) -o $@ $(OFILES)

$(OFILES): $(CFILES)
	@echo Compiling $<...
	@$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rdf $(BUILD)
	rm -f $(TARGET)
