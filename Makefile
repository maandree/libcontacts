.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)


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


all: libcontacts.a
$(OBJ): $($@:.o=.c) $(HDR)

libcontacts.a: $(OBJ)
	$(AR) rc $@ $(OBJ)
	$(AR) -s $@

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

install: libcontacts.a
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	cp -- libcontacts.a "$(DESTDIR)$(PREFIX)/lib/"
	cp -- libcontacts.h "$(DESTDIR)$(PREFIX)/include/"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcontacts.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libcontacts.h"

clean:
	-rm -f -- *.o *.a *.lo *.so *.su

.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: all install uninstall clean
