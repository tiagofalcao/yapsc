#!/usr/bin/make -f
PREFIX ?= /usr/local
EXEC_PREFIX ?= $(PREFIX)

# user commands
BINDIR ?= $(EXEC_PREFIX)/bin

# system binaries
SBINDIR ?= $(EXEC_PREFIX)/sbin

# program-specific binaries
LIBEXECDIR ?= $(EXEC_PREFIX)/libexec

# host-specific configuration
SYSCONFDIR ?= $(PREFIX)/etc

# architecture-independent variable data
SHAREDSTATEDIR ?= $(PREFIX)/com

# variable data
LOCALSTATEDIR ?= $(PREFIX)/var

# object code libraries
LIBDIR ?= $(EXEC_PREFIX)/lib

# header files
INCLUDEDIR ?= $(PREFIX)/include

# header files for non-GCC compilers
OLDINCLUDEDIR ?= /usr/include

# architecture-independent data root
DATAROOTDIR ?= $(PREFIX)/share

# architecture-independent data
DATADIR ?= $(DATAROOTDIR)

# GNU "info" documentation
INFODIR ?= $(DATAROOTDIR)/info

# locale-dependent data
LOCALEDIR ?= $(DATAROOTDIR)/locale

# manual pages
MANDIR ?= $(DATAROOTDIR)/man

# documentation root
DOCDIR ?= $(DATAROOTDIR)/doc/$(PACKAGE)

# HTML documentation
HTMLDIR ?= $(DOCDIR)

# DVI documentation
DVIDIR ?= $(DOCDIR)

# PDF documentation
PDFDIR ?= $(DOCDIR)

# PostScript documentation
PSDIR ?= $(DOCDIR)
