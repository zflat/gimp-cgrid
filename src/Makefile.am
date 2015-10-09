## Process this file with automake to produce Makefile.in

bin_PROGRAMS = gimp-collection-grid-maker

bindir = $(GIMP_LIBDIR)/plug-ins

gimp_collection_grid_maker_SOURCES = \
	cgrid_utils.h	\
	cgrid_utils.c	\
	plugin-intl.h	\
	ui.c	\
	ui.h	\
	main.c		\
	main.h		\
	render.c	\
	render.h

AM_CPPFLAGS = \
	-DLOCALEDIR=\""$(LOCALEDIR)"\"		\
	-DDATADIR=\""$(DATADIR)"\"

INCLUDES =\
	-I$(top_srcdir)		\
	@GIMP_CFLAGS@		\
	-I$(includedir)

LDADD = $(GIMP_LIBS)

