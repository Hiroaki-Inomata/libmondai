# libmondai -- MONTSUQI data access library
# Copyright (C) 2005  ogochan
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA

## Process this file with automake to produce Makefile.in

bin_PROGRAMS = rec2copy monconv

AM_CFLAGS	=	-Wall -Wunused -Wredundant-decls -funsigned-char  \
	-Wno-pointer-sign

LDADD 		=	\
	$(GLIB_LIBS)					\
	$(XML_LIBS)						\
	$(top_builddir)/src/libmondai.la

AM_CPPFLAGS 			= 					\
	-I$(top_srcdir)/src					\
	$(JSON_CFLAGS)						\
	$(GLIB_CFLAGS)

DEPENDENCIES	=						\
	$(top_srcdir)/src/libmondai.la

rec2copy_SOURCES	=					\
	option.c	option.h				\
	rec2copy.c
monconv_SOURCES	=						\
	option.c	option.h				\
	monconv.c

EXTRA_DIST = option.doc
