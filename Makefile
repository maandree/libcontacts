.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

OS = linux
# Linux:   linux
# Mac OS:  macos
# Windows: windows
include mk/$(OS).mk


LIB_MAJOR = 1
LIB_MINOR = 0
LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR)


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

MAN0 = libcontacts.h.0
MAN3 = $(OBJ:.o=.3)
MAN5 = contacts.5
MAN7 = libcontacts.7


all: libcontacts.a libcontacts.$(LIBEXT) test
$(OBJ): $($@:.o=.c) $(HDR)
test.o: test.c $(HDR)

libcontacts.a: $(OBJ)
	-rm -f -- $@
	$(AR) rc $@ $(OBJ)
	$(AR) -s $@

libcontacts.$(LIBEXT): $(LOBJ)
	$(CC) $(LIBFLAGS) -o $@ $(LOBJ) $(LDFLAGS)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.c.lo:
	$(CC) -fPIC -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

test: test.o libcontacts.a
	$(CC) -o $@ test.o libcontacts.a $(LDFLAGS)

check: test
	@rm -rf -- .testdir
	$(CHECK_PREFIX) ./test
	@rm -rf -- .testdir

install: libcontacts.a libcontacts.$(LIBEXT)
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man0"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man3"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man5"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man7"
	cp -- libcontacts.a "$(DESTDIR)$(PREFIX)/lib/"
	cp -- libcontacts.h "$(DESTDIR)$(PREFIX)/include/"
	cp -- libcontacts.$(LIBEXT) "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBMINOREXT)"
	$(FIX_INSTALL_NAME) "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBMINOREXT)"
	ln -sf -- libcontacts.$(LIBMINOREXT) "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBMAJOREXT)"
	ln -sf -- libcontacts.$(LIBMAJOREXT) "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBEXT)"
	cp -- $(MAN0) "$(DESTDIR)$(MANPREFIX)/man0/"
	cp -- $(MAN3) "$(DESTDIR)$(MANPREFIX)/man3/"
	cp -- $(MAN5) "$(DESTDIR)$(MANPREFIX)/man5/"
	cp -- $(MAN7) "$(DESTDIR)$(MANPREFIX)/man7/"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBMAJOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBMINOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcontacts.$(LIBEXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcontacts.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libcontacts.h"
	-cd -- "$(DESTDIR)$(MANPREFIX)/man0/" && rm -rf -- $(MAN0)
	-cd -- "$(DESTDIR)$(MANPREFIX)/man3/" && rm -rf -- $(MAN3)
	-cd -- "$(DESTDIR)$(MANPREFIX)/man5/" && rm -rf -- $(MAN5)
	-cd -- "$(DESTDIR)$(MANPREFIX)/man7/" && rm -rf -- $(MAN7)

clean:
	-rm -rf -- *.o *.a *.lo *.so *.dylib *.dll *.su test .testdir

.SUFFIXES:
.SUFFIXES: .c .o .lo

.PHONY: all check install uninstall clean
