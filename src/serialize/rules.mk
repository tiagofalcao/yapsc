# Standard things

sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)

# Subdirectories, in random order

#dir	:= $(d)/test
#include		$(dir)/rules.mk

# Local variables

MODULE_$(d)	:= $(shell basename $(d))
HEADERS_$(d)	:= $(BUILDDIR)/$(d)/yapsc_message.pb.h
SRCS_$(d)	:= $(BUILDDIR)/$(d)/yapsc_message.pb.cc
OBJS_$(d)	:= $(BUILDDIR)/$(d)/yapsc_message.pb.o

.SECONDARY: $(HEADERS_$(d)) $(SRCS_$(d))
CLEAN		:= $(CLEAN) $(OBJS_$(d))

$(BUILDDIR)/$(d):
		mkdir -p $@

$(BUILDDIR)/$(d)/%.pb.h: $(d)/%.proto | $(BUILDDIR)/$(d)
	protoc --cpp_out=$(BUILDDIR) $<

$(BUILDDIR)/$(d)/%.pb.cc: $(BUILDDIR)/$(d)/%.pb.h
	true

INCLUDES_$(PACKAGE) := $(INCLUDES_$(PACKAGE)) -I$(BUILDDIR)/$(d) -I$(d)

$(BUILDDIR)/$(d)/%.o: CXXFLAGS_TGT = $(INCLUDES_$(PACKAGE)) $(CXXFLAGS_$(PACKAGE))
$(BUILDDIR)/$(d)/%.o: $(BUILDDIR)/$(d)/%.cc $(HEADERS_$(d)) | $(BUILDDIR)/$(d)
		$(COMP)

HEADERS_$(PACKAGE) := $(HEADERS_$(d)) $(HEADERS_$(PACKAGE))
OBJS_$(PACKAGE) := $(OBJS_$(d)) $(OBJS_$(PACKAGE))

# Standard things

-include	$(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
