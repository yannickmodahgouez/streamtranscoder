## Process this with automake to create Makefile.in
 
AUTOMAKE_OPTIONS = foreign
 
bin_PROGRAMS = streamTranscoderv3

SUBDIRS = liboddcast

EXTRA_DIST = 			\
				AUTHORS \
				cbuffer.h \
				COPYING \
				decodethread.h \
				mp3decoder.h \
				oddsock_logo.bmp \
				reconnectthread.h \
				res/streamTranscoderUIv3.ico \
				res/streamTranscoderUIv3.rc2 \
				resource.h \
				socket_client.h \
				sourcethread.h \
				StdAfx.cpp \
				StdAfx.h \
				streamTranscoderUIv3.cpp \
				streamTranscoderUIv3Dlg.cpp \
				streamTranscoderUIv3Dlg.h \
				streamTranscoderUIv3.dsp \
				streamTranscoderUIv3.dsw \
				streamTranscoderUIv3.h \
				streamTranscoderUIv3.rc \
				streamTranscoderv3.nsi \
				streamTranscoderv3Service.cpp \
				streamTranscoderV3Service.dsp \
				vorbisdecoder.h
 
streamTranscoderv3_SOURCES = 	streamTranscoderv3.cpp \
								cbuffer.c \
								sourcethread.cpp \
								decodethread.cpp \
								reconnectthread.cpp \
								mp3decoder.cpp \
								vorbisdecoder.cpp \
								socket_client.c
 
streamTranscoderv3_LDADD = liboddcast/liboddcast.a

LIBS = @LIBS@ @OGG_LIBS@ @VORBIS_LIBS@ @MAD_LIBS@ @LAME_LIBS@ @VORBISENC_LIBS@ @VORBISFILE_LIBS@ @FAAC_LIBS@ @LIBFLAC_LIBS@
CFLAGS = -g @CFLAGS@ @OGG_CFLAGS@ @VORBIS_CFLAGS@ @MAD_CFLAGS@ @LAME_CFLAGS@ @FAAC_CFLAGS@ @LIBFLAC_CFLAGS@
