

bin_PROGRAMS = interxim 
#noinst_PROGRAMS =  sampleIM
interxim_SOURCES = interxim.cpp IC.c interface.cpp gui.cpp kmap.c \
	font_selector.cpp
interxim_LDADD = ../IMdkit/.libs/libIMdkit.al @X_LIBS@ @FLTK@ -lX11
#sampleIM_SOURCES = sampleIM.c IC.c
#sampleIM_LDADD = ../IMdkit/.libs/libIMdkit.al @X_LIBS@ -lX11 
INCLUDES = -Wall -I../IMdkit -DPKGDATADIR=\"$(pkgdatadir)\"
