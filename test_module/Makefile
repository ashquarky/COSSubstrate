#	Cafe OS Substrate

#	Makefile - Build rules for Substrate modules.
#	No partner file

#	https://github.com/QuarkTheAwesome/COSSubstrate

#	Copyright (c) 2016 Ash (QuarkTheAwesome)
#	Permission is hereby granted, free of charge, to any person obtaining a copy
#	of this software and associated documentation files (the "Software"), to
#	deal in the Software without restriction, including without limitation the
#	rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
#	sell copies of the Software, and to permit persons to whom the Software is
#	furnished to do so, subject to the following conditions:
#	The above copyright notice and this permission notice shall be included in
#	all copies or substantial portions of the Software.
#	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#	THE SOFTWARE.

.SUFFIXES:

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPRO")
endif

export VER ?= 550

export SRC	= src
export SOURCES	= src 
export TARGET	= test.cosm
export INCLUDES	= src
export LIBDIRS	= $(DEVKITPRO)/libogc

export BUILD	= build/build

export PATH		:= $(DEVKITPPC)/bin:$(PATH)
export INCLUDES	:= $(INCLUDES)

export PREFIX	= powerpc-eabi-
export CC		= $(PREFIX)gcc
export AS		= $(PREFIX)gcc
export LD		= $(PREFIX)ld

export CFLAGS	= -c -O1 -fno-strict-aliasing -fPIE -fno-builtin -Wall -std=c99 -DVER=$(VER)
export SFLAGS	= -c -fPIE
export LDFLAGS	= -fPIE -shared --unresolved-symbols=report-all #BYO memcpy

export CFILES	= $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
export SFILES	= $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.S))
export OFILES	= $(foreach cfl,$(CFILES) $(SFILES),$(BUILD)/$(dir $(cfl))$(basename $(notdir $(cfl))).o)
export INCLUDE	= $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
				  $(foreach dir,$(LIBDIRS),-I$(dir)/include)

all: setup $(TARGET)

setup:
	@echo Compiling...
	@mkdir -p $(foreach dir,$(SOURCES),$(BUILD)/$(dir))

$(TARGET): $(OFILES)
	@echo Linking $(TARGET)...
	@$(LD) $(LDFLAGS) -o $@ $(OFILES)

$(BUILD)/$(SRC)/%.o: $(SRC)/%.c
	@echo Compiling $<...
	@$(CC) $(CFLAGS) $(INCLUDE) -o $@ $<

$(BUILD)/$(SRC)/%.o: $(SRC)/%.S
	@echo Assembling $<...
	@$(AS) $(SFLAGS) $(INCLUDE) -o $@ $<

clean:
	rm -rdf $(BUILD)
	rm -f $(TARGET)
