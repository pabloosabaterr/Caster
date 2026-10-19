BUILDDIR := build
TARGET := $(BUILDDIR)/sim
OBJODIR := $(BUILDDIR)/obj
RTLDIR := ../rtl

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
	-std=c++17

# Extra checks for the testbench sources only, not the Verilator runtime
DEVFLAGS :=
ifeq ($(DEVELOPER),1)
DEVFLAGS += -Wextra -Wpedantic -Werror
endif

LDFLAGS :=

INCLUDES += \
	-I. \
	-I$(RTLDIR) \
	-isystem $(RTLOBJDIR) \
	-isystem $(VINCD) \
	-isystem $(VINCD)/vltstd

CXXSRCS += \
	./main.cpp \
	./dispsim.cpp \
	./intapi.cpp \
	./spisim.cpp \
	./srcsim.cpp \
	./stb.cpp \
	./vramsim.cpp \
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
