RTLDIR   := caster/rtl
BUILDDIR := build
SRCDIR   := src

RTLOBJDIR := $(BUILDDIR)/obj_dir
OBJODIR   := $(BUILDDIR)/obj
TARGET    := $(BUILDDIR)/sim

TOP  := caster
VTOP := V$(TOP)

RTLOBJ := $(RTLOBJDIR)/$(VTOP)__ALL.a

# Touched each time Verilator runs, so the RTL is only rebuilt when a .v
# changes.
VSTAMP := $(RTLOBJDIR)/.verilated

VERILATOR ?= verilator
VINCD     := $(shell $(VERILATOR) -getenv VERILATOR_ROOT)/include

CPUS       ?= $(shell getconf _NPROCESSORS_ONLN)
SDL_CONFIG ?= sdl2-config
DEVELOPER  ?= 0
CXX        ?= g++

VFLAGS := -Wall -Wno-fatal -MMD --trace -cc \
          --Mdir $(RTLOBJDIR) \
          -I$(RTLDIR) \
          -I$(RTLDIR)/mulib/rtl \
          -I$(RTLDIR)/mulib/rtl/baseip \
          -I$(RTLDIR)/mulib/rtl/baseip/generic \
          -Wno-PINCONNECTEMPTY \
          +define+SIMULATION=1

LIBS := -lm $(shell $(SDL_CONFIG) --libs)

CXXFLAGS := $(shell $(SDL_CONFIG) --cflags) \
            -Wall -std=c++20 -MMD -MP

ifeq ($(DEVELOPER),1)
DEVFLAGS := -Wextra -Wpedantic -Werror
endif

INCLUDES := -I$(SRCDIR) \
            -I$(RTLDIR) \
            -isystem third-party \
            -isystem $(RTLOBJDIR) \
            -isystem $(VINCD) \
            -isystem $(VINCD)/vltstd

SIMSRCS := $(wildcard $(SRCDIR)/*.cpp)
VLTSRCS := verilated.cpp \
           verilated_vcd_c.cpp \
           verilated_threads.cpp

SIMOBJS := $(SIMSRCS:%.cpp=$(OBJODIR)/%.o)
VLTOBJS := $(VLTSRCS:%.cpp=$(OBJODIR)/vlt/%.o)
OBJS    := $(SIMOBJS) $(VLTOBJS)

.PHONY: all clean

all: $(TARGET)

# Link only when an object or the RTL library changed.
$(TARGET): $(OBJS) $(RTLOBJ)
	@echo LD $@
	@$(CXX) $(LDFLAGS) $(OBJS) $(RTLOBJ) $(LIBS) -o $@

# Verilate. The generated .d lists every .v the top module pulls in.
# Rewrite it so those files become prerequisites of the stamp.
$(VSTAMP):
	@echo VERILATOR $(TOP).v
	@mkdir -p $(RTLOBJDIR)
	@$(VERILATOR) $(VFLAGS) $(TOP).v > /dev/null
	@sed -e '1s|^[^:]*:|$(VSTAMP):|' \
		$(RTLOBJDIR)/$(VTOP)__ver.d \
		> $(RTLOBJDIR)/stamp.d
	@touch $@

# Built in parallel by default. With make -jN, the jobserver is shared.
$(RTLOBJ): $(VSTAMP)
	@echo "RTL $(notdir $@)"
	@$(MAKE) --no-print-directory -s \
		$(if $(findstring jobserver,$(MAKEFLAGS)),,-j$(CPUS)) \
		-C $(RTLOBJDIR) -f $(VTOP).mk

# Testbench sources need Vcaster.h on the first build. Afterwards, the
# generated -MMD dependency files handle the dependencies.
$(OBJODIR)/%.o: %.cpp | $(VSTAMP)
	@echo "CXX $(notdir $<)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(DEVFLAGS) $(INCLUDES) \
		-c -o $@ $<

$(OBJODIR)/vlt/%.o: $(VINCD)/%.cpp
	@echo "CXX $(notdir $<)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -rf $(BUILDDIR)

-include $(OBJS:.o=.d)
-include $(RTLOBJDIR)/stamp.d
