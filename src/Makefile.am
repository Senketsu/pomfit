bin_PROGRAMS = pomfit
pomfit_SOURCES = \
				Main.c		\
				database.c		\
				keybinds.c		\
				log.c		\
				manager.c		\
				other.c		\
				profiles.c		\
				queue.c		\
				screenshot.cpp		\
				settings.c		\
				uploader.c

NOINST_H_FILES = \
		 		database.h  		\
		 		keybinds.h  		\
		 		log.h  		\
		 		manager.h  		\
		 		other.h  		\
		 		profiles.h  		\
		 		queue.h  		\
		 		screenshot.h  		\
		 		settings.h  		\
		 		structs.h  		\
		 		uploader.h   	

EXTRA_DIST = $(NOINST_H_FILES)

CC=gcc
AM_CFLAGS  = `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0 --libs libnotify --libs libcurl --libs keybinder-3.0 --libs sqlite3`


