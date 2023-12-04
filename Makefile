TARGET_ARCH ?= $(shell uname -m)

# Project folders that contain CUDA samples
PROJECTS ?= $(shell find src -name Makefile)

FILTER_OUT :=

PROJECTS := $(filter-out $(FILTER_OUT),$(PROJECTS))

%.ph_build :
	+@$(MAKE) -C $(dir $*) $(MAKECMDGOALS)

%.ph_clean : 
	+@$(MAKE) -C $(dir $*) clean $(USE_DEVICE)

all:  $(addsuffix .ph_build,$(PROJECTS))

build: $(addsuffix .ph_build,$(PROJECTS))

clean: $(addsuffix .ph_clean,$(PROJECTS))
