BIN=neurosim-tsim

### Auxiliaries ###
# characters
NULL:=
COMMA:=,
SPACE:=$(NULL) #

# bash commands
MKDIR=mkdir -p
RM=rm -rf

# extensions
CPPEXT=cpp
HDREXT=h
OBJEXT=o
###################

### Toolchain ###
# compiler commands
CXX=clang++
LD=clang++
AR=ar
MAKE=make 

# compiler flags
CXXFLAGS=--std=c++11 -O3 -ferror-limit=3 -DRAMULATOR $(if $(NDEBUG),-DNDEBUG) $(if $(NINFO),-DNINFO) -g 
LDFLAGS=-O3
ARFLAGS=-rc
#################

### Paths ###
# root directories
SRCDIR=src
OBJDIR=obj
HDRDIR=include
LIBDIR=lib

ifdef EXAMPLE
SRCDIR=example/$(EXAMPLE)/src
HDRDIR=example/$(EXAMPLE)/include
endif

# library paths
TSIM=TSim
TSIM_DIR=$(LIBDIR)/tsim
TSIM_HDRDIR=$(TSIM_DIR)/include/TSim
TSIM_LIB=$(TSIM_DIR)/libtsim.a
TSIM_SRC=$(filter %.$(CPPEXT),\
		 $(wildcard $(addsuffix /*,$(shell find $(TSIM_DIR)/src -type d))))

RAM=Ramulator
RAM_DIR=$(LIBDIR)/ramulator
RAM_HDRDIR=$(RAM_DIR)/src
RAM_LIB=$(RAM_DIR)/libram.a
RAM_SRCS := $(filter-out $(RAMULATOR_SRCDIR)/Main.cpp $(RAMULATOR_SRCDIR)/Gem5Wrapper.cpp, \
	$(wildcard $(RAMULATOR_SRCDIR)/*.cpp))


# TODO: add variables for new library
#[lib]=[lib_name]
#[lib]_DIR=$(LIBDIR)/[lib_dirname]
#[lib]_HDRDIR=$([lib]_DIR)/[lib_hdrdir_name]
#[lib]_LIB=$([lib]_DIR)/[lib_bindir_name]

# libraies in use
USING_LIBS=TSIM RAM  		# TODO: add [lib] in this list.
#############

### Helper functions ###
print=printf "info: %-15s $(if $(2),[,) $(2)$(if $(3), --> $(3),) $(if $(2),],)\n" $(1)
to_comma_list=$(subst $(SPACE),$(COMMA) ,$(1))
########################


### Makefile lists ###
# source and object files
SRCSUBDIRS:=$(shell find $(SRCDIR) -type d)
OBJSUBDIRS:=$(subst $(SRCDIR),$(OBJDIR),$(SRCSUBDIRS))
CODEFILES:=$(addsuffix /*,$(SRCSUBDIRS))
CODEFILES:=$(wildcard $(CODEFILES))

temp:=$(filter %.$(OBJEXT),$(wildcard $(addsuffix /*,$(shell find $(TSIM_DIR)/obj -type d))))

SRCFILES:=$(filter %.$(CPPEXT),$(CODEFILES))
OBJFILES:=$(subst $(SRCDIR),$(OBJDIR),$(SRCFILES:%.$(CPPEXT)=%.$(OBJEXT)))
RMFILES:=$(OBJDIR) $(BIN)

LIBS:=$(foreach LIB,	\
	$(if $(EXAMPLE),$(filter $(USING_LIBS),TSIM),$(USING_LIBS)),	\
	$($(LIB)_LIB))
######################


### rules

.PHONY: $(USING_LIBS)

all: $(USING_LIBS) $(OBJSUBDIRS) $(BIN)

TSIM:
	@ $(if $(wildcard $(TSIM_HDRDIR)),,$(error non-existing $(TSIM_HDRDIR)))
	@ $(shell $(RM) $(HDRDIR)/$(TSIM))
	@ $(call print,"symlinking..",$(TSIM_HDRDIR),$(HDRDIR)/$(TSIM))
	@ $(shell ln -s $(abspath $(TSIM_HDRDIR)) $(HDRDIR)/$(TSIM))

$(OBJSUBDIRS):
	@ $(call print,"creating..",$(call to_comma_list,$@))
	@ $(MKDIR) $@

$(LIBS): $(TSIM_SRC)
	@ $(MAKE) -C $(TSIM_DIR)

$(BIN): $(OBJFILES) $(LIBS)
	@ $(call print,"linking..",$(call to_comma_list,$^),$@)
	@ $(LD) $^ -o $@

$(OBJDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(CPPEXT)
	@ $(call print,"compiling..",$<,$@)
	@ $(CXX) -I$(HDRDIR) $(CXXFLAGS) -c $^ -o $@


clean:
	@ $(call print,"removing..",$(call to_comma_list,$(RMFILES)))
	@ $(RM) $(RMFILES)
	@ $(call print,"done.")

superclean:
	@ $(MAKE) -C $(TSIM_DIR) superclean
	@ $(MAKE) clean
