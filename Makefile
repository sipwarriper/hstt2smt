#	
#	credits to klanco
#

APPNAME := hstt2smt
BINDIR	:= bin
PLATFORM    := linux
SOURCES := source smtapi/src smtapi/src/solvers/glucose #insert multiple folders here if wanted, but it already does a recursive search
INCLUDE := smtapi/src smtapi/src/controllers smtapi/src/encoders smtapi/src/MDD smtapi/src/optimizers smtapi/src/util smtapi/src/solvers/glucose/ smtapi/src/solvers/glucose/core smtapi/src/solvers/glucose/mtl smtapi/src/solvers/glucose/simp smtapi/src/solvers/glucose/utils
BUILDDIR := build
DEBUG := 0


ifeq ($(DEBUG), 1)
FLAGS	:= -g3 -O0 -fbuiltin -fstack-protector-all
else 
FLAGS    := -O3 -ffast-math -D__LINUX__ -Xlinker -Map=$(BUILDDIR)/$(PLATFORM)/$(APPNAME).map #-Werror=return-type
endif

CCFLAGS  := $(FLAGS) 
CXXFLAGS := $(FLAGS) -std=c++17
LIBS    :=  -lyices -lpugixml -L$(BUILDDIR)/$(PLATFORM)

DEFS := -DUSEYICES -DUSEGLUCOSE



#YOU SHOULDN'T NEED TO MODIFY ANYTHING PAST THIS POINT

TOPDIR ?= $(CURDIR)

CFILES		:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/**/*.c)) $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/**/*.cpp)) $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp)) $(foreach dir,$(SOURCES),$(wildcard $(dir)/**/*.cc)) $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cc))

# $(info $(CPPFILES))


CFILES	    := $(CFILES:$(SOURCES)/%=%)
CPPFILES	:= $(CPPFILES:$(SOURCES)/%=%)

OFILES 	:=	$(addsuffix .o, $(basename $(CFILES))) $(addsuffix .o, $(basename $(CPPFILES)))
OFILES := $(addprefix $(BUILDDIR)/$(PLATFORM)/,$(OFILES))

# $(info $(OFILES))

CC   := gcc
CXX  := g++


INCLUDE := $(addprefix -I,$(INCLUDE))
.DEFAULT_GOAL := all
.PHONY: pre
pre:
	@printf "Creating directories "
	@mkdir -p $(BINDIR)/$(PLATFORM)
	@mkdir -p $(addprefix $(BUILDDIR)/$(PLATFORM)/,$(dir $(CFILES))) $(addprefix $(BUILDDIR)/$(PLATFORM)/,$(dir $(CPPFILES)))
	@echo "DONE"

$(BUILDDIR)/$(PLATFORM)/%.o: %.c
	@printf "Compiling $< ... "
	@$(CC) $(CCFLAGS) $(INCLUDE) $(LIBS) $(DEFS) -c $< -o $@
	@echo "DONE"

$(BUILDDIR)/$(PLATFORM)/%.o: %.cc
	@printf "Compiling $< ... "
	@$(CXX) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(DEFS) -c $< -o $@
	@echo "DONE"

$(BUILDDIR)/$(PLATFORM)/%.o: %.cpp
	@printf "Compiling $< ... "
	@$(CXX) $(CXXFLAGS) $(INCLUDE) $(LIBS) $(DEFS) -c $< -o $@
	@echo "DONE"

.PHONY: all
all: pre $(OFILES)
	@printf "Linking $@ ... "
	@$(CXX) $(CXXFLAGS) $(INCLUDE) $(OFILES) $(LIBS) -o $(BINDIR)/$(PLATFORM)/$(APPNAME)
	@echo "DONE"

.PHONY: clean
clean:
	@rm -rf $(BUILDDIR)/$(PLATFORM)/*
	@rm -f $(APPNAME)
