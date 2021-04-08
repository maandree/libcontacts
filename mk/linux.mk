LIBEXT      = so
LIBFLAGS    = -shared -Wl,-soname,libcontacts.$(LIBEXT).$(LIB_MAJOR)
LIBMAJOREXT = $(LIBEXT).$(LIB_MAJOR)
LIBMINOREXT = $(LIBEXT).$(LIB_VERSION)
