AAM_CFLAGS = -g -g3 -O2 -Wall -DVERSION=\"$(VERSION)\"

rude_psychicdir = $(PURPLE_INSTALL)

rude_psychic_LTLIBRARIES = \
	rude_psychic.la

rude_psychic_la_CFLAGS =	$(purple_CFLAGS)
rude_psychic_la_SOURCES = rude_psychic.c
rude_psychic_la_LDFLAGS =    \
	-module		    \
	-avoid-version	    \
	-L$(PURPLE_INSTALL) \
	$(purple_LIBS)

