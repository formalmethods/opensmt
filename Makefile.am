SUBDIRS = src

bin_PROGRAMS = opensmt

INCLUDES = $(config_includedirs)

opensmt_SOURCES = src/bin/Main.C
#opensmt_LDFLAGS = -lgmp -lgmpxx

LDADD = src/libopensmt.la /usr/local/lib/libgmpxx.a /usr/local/lib/libgmp.a
