## automake.am
##
## Makefile for the lwcored(1) daemon
##
## (C) Copyright 2015 Fred Gleason <fredg@paravelsystems.com>
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License version 2 as
##   published by the Free Software Foundation.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public
##   License along with this program; if not, write to the Free Software
##   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##
##
## Use automake to process this into a Makefile.in

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -Wno-strict-aliasing @QT4_CFLAGS@ @ALSA_CFLAGS@ @SAMPLERATE_CFLAGS@ @SOUNDTOUCH_CFLAGS@
MOC = @QT4_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	@MOC@ $< -o $@


sbin_PROGRAMS = lwcored

dist_lwcored_SOURCES = audioqueue.cpp qaudioqueue.h\
                       capture.cpp\
                       lwcored.cpp lwcored.h\
                       playout.cpp\
                       ringbuffer.cpp ringbuffer.h

nodist_lwcored_SOURCES = cmdswitch.cpp cmdswitch.h\
                         moc_audioqueue.cpp\
                         moc_lwcored.cpp

lwcored_LDADD = @ALSA_LIBS@ @SOUNDTOUCH_LIBS@ @SAMPLERATE_LIBS@ -lQtCore -lQtNetwork
lwcored_LDFLAGS = 

CLEANFILES = *~\
             moc_*

DISTCLEANFILES = cmdswitch.cpp cmdswitch.h

MAINTAINERCLEANFILES = *~\
                       Makefile.in
