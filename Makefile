#!/usr/bin/make -f
MKFILE := $(abspath $(lastword $(MAKEFILE_LIST)))
ROOTDIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE))))
BUILDDIR ?= build

.PHONY:		default
default:	all

PACKAGE := yapsc
DESCRIPTION := A SystemC library to manage simulation processes.
VERSION := 0.2

include gnudirs.mk

SHELL = /bin/sh
CPP ?= cpp
CPPFLAGS ?=
CC ?= mpicc
CXX ?= mpic++
AR ?= ar
CFLAGS ?= -O0 -fPIC -ggdb3
CXXFLAGS ?= -O0 -std=c++11 -fPIC -ggdb3
LDFLAGS ?=
LDLIBS ?=
ARFLAGS ?= rcs

REQUIREMENTS := expio systemc protobuf avahi-client
OPTIONALS :=

include rules.mk
