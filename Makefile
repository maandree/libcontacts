.POSIX:

LIB_MAJOR = 1
LIB_MINOR = 0
LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR)


CONFIGFILE = config.mk
include $(CONFIGFILE)

OS = linux
# Linux:   linux
# Mac OS:  macos
# Windows: windows
include mk/$(OS).mk



OBJ =\
	libcontacts_address_destroy.o\
	libcontacts_birthday_destroy.o\
	libcontacts_block_destroy.o\
	libcontacts_chat_destroy.o\
	libcontacts_contact_destroy.o\
	libcontacts_email_destroy.o\
	libcontacts_format_contact.o\
	libcontacts_get_path.o\
	libcontacts_list_contacts.o\
	libcontacts_load_contact.o\
	libcontacts_load_contacts.o\
	libcontacts_number_destroy.o\
	libcontacts_organisation_destroy.o\
	libcontacts_parse_contact.o\
	libcontacts_pgpkey_destroy.o\
	libcontacts_same_number.o\
	libcontacts_save_contact.o\
	libcontacts_site_destroy.o

HDR =\
	common.h\
	libcontacts.h

LOBJ = $(OBJ:.o=.lo)


all: libcontacts.a libcontacts.$(LIBEXT)
$(OBJ): $($@:.o=.c) $(HDR)

libcontacts.a: $(OBJ)
	-rm -f -- $@
	$(AR) rc $@ $(OBJ)
	$(AR) -s $@

libcontacts.$(LIBEXT): $(LOBJ)
	$(CC) $(LIBFLAGS) $(LDFLAGS_METHODS) -o $@ $(LOBJ) $(LDFLAGS)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.c.lo:
	$(CC) -fPIC -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

install: libcontacts.a libcontacts.$(LIBEXT)
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	cp -- libcontacts.a "$(DESTDIR)$(PREFIX)/lib/"
	cp -- libcontacts.h "$(DESTDIR)$(PREFIX)/include/"
	cp -- libcontacts.$(LIBEXT) "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBMINOREXT)"
	ln -sf -- libcontacts.$(LIBMINOREXT) "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBMAJOREXT)"
	ln -sf -- libcontacts.$(LIBMAJOREXT) "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBEXT)"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBMAJOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBMINOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBEXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcontacts.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libcontacts.h"

clean:
	-rm -f -- *.o *.a *.lo *.so *.dylib *.dll *.su

.SUFFIXES:
.SUFFIXES: .c .o .lo

.PHONY: all install uninstall clean
