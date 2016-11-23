d		:= $(dir)
# Subdirectories, in random order
dir	:= $(d)/sc-tlm
include	$(dir)/rules.mk

dir	:= $(d)/tcp-tlm
include	$(dir)/rules.mk

dir	:= $(d)/uds-tlm
include	$(dir)/rules.mk

dir	:= $(d)/mpi-tlm
include	$(dir)/rules.mk

dir	:= $(d)/sc-tlm-wait
include	$(dir)/rules.mk

dir	:= $(d)/tcp-tlm-wait
include	$(dir)/rules.mk

dir	:= $(d)/uds-tlm-wait
include	$(dir)/rules.mk

dir	:= $(d)/mpi-tlm-wait
include	$(dir)/rules.mk
