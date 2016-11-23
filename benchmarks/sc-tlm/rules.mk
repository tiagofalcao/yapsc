# Standard things

sp		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)


# Local variables

HEADERS_$(d)	:= $(shell find $(d) -iname '*.h')
SRCS_$(d)	:= $(shell find $(d) -iname '*.cc')
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

BENCHMARKS := $(TGT_$(d)) $(BENCHMARKS)
BENCH := $(BENCH) BENCH_$(d)
BENCH_$(d): $(TGT_$(d))
	@echo "Running $< ... "
	@LD_LIBRARY_PATH="$(PATH_$(PACKAGE)):$${LD_LIBRARY_PATH}" EXPIO_LOG_LEVEL=1 $< &>>/dev/null

# Standard things

-include	$(DEPS_$(d))

d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
