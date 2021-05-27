# -*- Makefile -*-
#
#
HOME = $(SST_WORKDIR)
INCLUDE = -I$(HOME)/firrtl-sig/ -I$(HOME)/sst-tools/tools/ariel/femlm/api/ -I$(HOME)/sst-elements/src/sst/elements/ariel/

AM_CPPFLAGS = \
	$(MPI_CPPFLAGS) \
	$(INCLUDE)

compdir = $(pkglibdir)
comp_LTLIBRARIES = libvecshiftreg.la
libvecshiftreg_la_SOURCES = \
	vecshiftreg.h \
	vecshiftreg.cc \
	rtlevent.h \
	rtlevent.cc \
	rtlmemmgr.h \
	rtlmemmgr_cache.h \
	rtlmemmgr_simple.h \
	rtlmemmgr_simple.cc \
	rtlreadev.h \
	rtlwriteev.h 

libvecshiftreg_la_LDFLAGS = -module -avoid-version

install-exec-hook:
	$(SST_REGISTER_TOOL) SST_ELEMENT_SOURCE  sst-external-element=$(abs_srcdir)

##########################################################################
##########################################################################
##########################################################################
