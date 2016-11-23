d		:= $(dir)
# Subdirectories, in random order
dir	:= $(d)/tcp-zeroconf
include	$(dir)/rules.mk

dir	:= $(d)/tcp-wrappers
include	$(dir)/rules.mk

dir	:= $(d)/uds-wrappers
include	$(dir)/rules.mk

dir	:= $(d)/mpi-wrappers
include	$(dir)/rules.mk

dir	:= $(d)/sc-disabled-yapsc
include	$(dir)/rules.mk

dir	:= $(d)/sc-yapsc
include	$(dir)/rules.mk

dir	:= $(d)/tcp-tlm
include	$(dir)/rules.mk

dir	:= $(d)/tcp-tlm-zc
include	$(dir)/rules.mk

dir	:= $(d)/uds-tlm
include	$(dir)/rules.mk

dir	:= $(d)/mpi-tlm
include	$(dir)/rules.mk

dir	:= $(d)/tcp-tlm-fork
include	$(dir)/rules.mk

dir	:= $(d)/uds-tlm-fork
include	$(dir)/rules.mk

dir	:= $(d)/tcp-yapsc
include	$(dir)/rules.mk

dir	:= $(d)/uds-yapsc
include	$(dir)/rules.mk

dir	:= $(d)/mpi-yapsc
include	$(dir)/rules.mk
