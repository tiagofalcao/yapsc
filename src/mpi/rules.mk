# Standard things

sp 		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

# Subdirectories, in random order

#dir	:= $(d)/test
#include		$(dir)/rules.mk

# Local variables

MODULE_$(d)	:= $(shell basename $(d))
HEADERS_$(d)	:= $(shell find $(d) -type f -iname '*.h')
SRCS_$(d)	:= $(shell find $(d) -type f -iname '*.cc')
OBJS_$(d)	:= $(patsubst $(d)/%.cc,$(BUILDDIR)/$(d)/%.o, $(filter %.cc,$(SRCS_$(d))))

CLEAN		:= $(CLEAN) $(OBJS_$(d))

$(BUILDDIR)/$(d):
		mkdir -p $@

INCLUDES_$(PACKAGE) := $(INCLUDES_$(PACKAGE)) -I$(BUILDDIR)/$(d) -I$(d)

$(BUILDDIR)/$(d)/%.o: CXXFLAGS_TGT = $(INCLUDES_$(PACKAGE)) $(CXXFLAGS_$(PACKAGE))
$(BUILDDIR)/$(d)/%.o: $(d)/%.cc $(HEADERS_$(d)) $(HEADERS_src/serialize) | $(BUILDDIR)/$(d)
		$(COMP)

HEADERS_$(PACKAGE) := $(HEADERS_$(d)) $(HEADERS_$(PACKAGE))
OBJS_$(PACKAGE) := $(OBJS_$(d)) $(OBJS_$(PACKAGE))

# Standard things

-include	$(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
