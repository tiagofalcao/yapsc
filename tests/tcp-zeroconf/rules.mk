# Standard things

sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)


# Local variables

HEADERS_$(d)	:= $(shell find $(d) -type f -iname '*.h')
SRCS_$(d)	:= $(shell find $(d) -type f -iname '*.cc')
OBJS_$(d)	:= $(patsubst $(d)/%.cc,$(BUILDDIR)/$(d)/%.o, $(filter %.cc,$(SRCS_$(d))))

CLEAN		:= $(CLEAN) $(OBJS_$(d))

# Local rules

$(BUILDDIR)/$(d):
	mkdir -p $@

$(BUILDDIR)/$(d)/%.o: CXXFLAGS_TGT = $(INCLUDES_$(PACKAGE)) $(CXXFLAGS_$(PACKAGE))
$(BUILDDIR)/$(d)/%.o: $(HEADERS_$(d)) | $(BUILDDIR)/$(d)
$(BUILDDIR)/$(d)/%.o: $(d)/%.cc | $(BUILDDIR)/$(d)
		$(COMP)

TGT_$(d):= $(BUILDDIR)/$(d)/test
CLEAN	:= $(CLEAN) $(TGT_$(d))

$(TGT_$(d)): LDFLAGS_TGT = $(LDFLAGS_$(PACKAGE))
$(TGT_$(d)): LIBS_TGT = $(LIBPATH_$(PACKAGE)) $(LIB_$(PACKAGE))
$(TGT_$(d)): $(OBJS_$(d))
	$(LINK)

TESTS := $(TGT_$(d)) $(TESTS)
CHECK := $(CHECK) CHECK_$(d)
CHECK_$(d): $(TGT_$(d))
	@echo -n "Running $< ... "
	@LD_LIBRARY_PATH="$(PATH_$(PACKAGE)):$${LD_LIBRARY_PATH}" $< >>/dev/null  2>>/dev/null
	@echo ok

# Standard things

-include	$(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
