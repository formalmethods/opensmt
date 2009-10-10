SUBDIRS = emptysolver bvsolver lrasolver dlsolver osolver

noinst_LTLIBRARIES = libtsolvers.la

INCLUDES=$(config_includedirs)

libtsolvers_la_SOURCES = TSolver.h THandler.C THandler.h
libtsolvers_la_LIBADD = emptysolver/libemptysolver.la \
			bvsolver/libbvsolver.la \
			lrasolver/liblrasolver.la \
			dlsolver/libdlsolver.la \
			osolver/libosolver.la
