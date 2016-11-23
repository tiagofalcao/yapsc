d		:= $(dir)

dir	:= $(d)/serialize
include	$(dir)/rules.mk

dir	:= $(d)/core
include	$(dir)/rules.mk

dir	:= $(d)/tcp
include	$(dir)/rules.mk

dir	:= $(d)/uds
include	$(dir)/rules.mk

dir	:= $(d)/mpi
include	$(dir)/rules.mk


# Local rules

$(BUILDDIR)/$(d)/lib$(PACKAGE).so:	LDFLAGS_TGT = $(LINKFLAGS_systemc)
$(BUILDDIR)/$(d)/lib$(PACKAGE).so:	LIBS_TGT = $(LIBPATH_systemc) $(LIB_systemc)
$(BUILDDIR)/$(d)/lib$(PACKAGE).so: $(OBJS_$(PACKAGE))
	$(SHAREDLIB)

$(BUILDDIR)/$(d)/lib$(PACKAGE).a:	ARFLAGS_TGT :=
$(BUILDDIR)/$(d)/lib$(PACKAGE).a: $(OBJS_$(PACKAGE))
	$(STATICLIB)

TGT_LIB		:= $(TGT_LIB) $(BUILDDIR)/$(d)/lib$(PACKAGE).so $(BUILDDIR)/$(d)/lib$(PACKAGE).a
CLEAN		:= $(CLEAN) $(TGT_LIB)
TGT_INCLUDE	:= $(HEADERS_$(PACKAGE))

INCLUDES_$(PACKAGE)	:= $(INCLUDES_$(PACKAGE))
PATH_$(PACKAGE)		:= $(BUILDDIR)/$(d)/
LINKFLAGS_$(PACKAGE)	:= -L $(PATH_$(PACKAGE))
LIBPATH_$(PACKAGE)	:= -L $(PATH_$(PACKAGE))
LIB_$(PACKAGE)		:= -l $(PACKAGE)
