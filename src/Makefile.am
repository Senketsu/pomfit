bin_PROGRAMS = pomfit
pomfit_SOURCES = \
				Main_Linux.c		\
				pomfit.c

NOINST_H_FILES = \
		 		header.h  		\
		 		pomfit.h    	

EXTRA_DIST = $(NOINST_H_FILES)

CC=gcc
AM_CFLAGS  = `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0 --libs libnotify --libs libcurl --libs keybinder-3.0`


