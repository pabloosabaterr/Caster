BUILDDIR := build
TARGET := $(BUILDDIR)/sim
OBJODIR := $(BUILDDIR)/obj
SRCDIR := src
RTLDIR := caster/rtl

ifeq ($(VERILATOR_ROOT),)
VERILATOR_ROOT ?= $(shell verilator -getenv VERILATOR_ROOT)
endif
VROOT := $(VERILATOR_ROOT)
VINCD := $(VROOT)/include
RTLOBJDIR := $(BUILDDIR)/obj_dir
RTLOBJ := $(RTLOBJDIR)/Vcaster__ALL.a

CXX ?= g++
SDL_CONFIG = sdl2-config
DEVELOPER ?= 0

LIBS := -lm $(shell $(SDL_CONFIG) --libs)

CXXFLAGS := \
	$(shell $(SDL_CONFIG) --cflags) \
	-Wall -Og -g \
	-faligned-new \
	-std=c++20

# Extra checks for the testbench sources only, not the Verilator runtime
DEVFLAGS :=
ifeq ($(DEVELOPER),1)
DEVFLAGS += -Wextra -Wpedantic -Werror
endif

LDFLAGS :=

INCLUDES += \
	-I$(SRCDIR) \
	-I$(RTLDIR) \
	-isystem third-party \
	-isystem $(RTLOBJDIR) \
	-isystem $(VINCD) \
	-isystem $(VINCD)/vltstd

CXXSRCS += \
	$(SRCDIR)/main.cpp \
	$(SRCDIR)/panel.cpp \
	$(SRCDIR)/int-api.cpp \
	$(SRCDIR)/spi.cpp \
	$(SRCDIR)/video-source.cpp \
	$(SRCDIR)/stb.cpp \
	$(SRCDIR)/vram.cpp \
	$(SRCDIR)/sim.cpp \
	$(SRCDIR)/display.cpp \
	$(SRCDIR)/firmware.cpp \
	$(SRCDIR)/parse-options.cpp \
	$(SRCDIR)/log.cpp \
	verilated.cpp \
	verilated_vcd_c.cpp \
	verilated_threads.cpp

OBJS := $(CXXSRCS:%.cpp=$(OBJODIR)/%.o)
OBJS += $(RTLOBJ)

$(RTLOBJ):
	@echo Building RTL
	make -f rtl.mk VOBJ=$(RTLOBJDIR)

$(OBJODIR)/%.o: %.cpp $(RTLOBJ)
	@echo [CXX] $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(DEVFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJODIR)/%.o: $(VINCD)/%.cpp
	@echo [CXX] $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

PHONY += all
all: $(OBJS)
	@$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)
	@echo 'all finish'

PHONY += clean
clean:
	rm -rf $(BUILDDIR)
	@echo 'clean finish'

.PHONY: $(PHONY)

.DEFAULT_GOAL := all
