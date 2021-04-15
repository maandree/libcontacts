/* See LICENSE file for copyright and license details. */
#include "common.h"
#include <stdarg.h>


#define TEST(EXPR)\
	do {\
		errno = 0;\
		if (EXPR)\
			break;\
		fprintf(stderr, "Failure at line %i, with errno = %i (%s): %s\n", __LINE__, errno, strerror(errno), #EXPR);\
		exit(1);\
	} while (0)


static void
touch(const char *path)
{
	int fd;
	TEST((fd = open(path, O_WRONLY | O_CREAT, 0600)) >= 0);
	TEST(!close(fd));
}

static int
strpcmp(const void *av, const void *bv)
{
	const char *const *a = av;
	const char *const *b = bv;
	return strcmp(*a, *b);
}

static int
contactidcmp(const void *av, const void *bv)
{
	const struct libcontacts_contact *const *a = av;
	const struct libcontacts_contact *const *b = bv;
	return strcmp((*a)->id, (*b)->id);
}

static size_t
sort_contact_ids(char **ids)
{
	size_t n;
	for (n = 0; ids[n]; n++);
	qsort(ids, n, sizeof(*ids), strpcmp);
	return n;
}

static size_t
sort_contacts_by_id(struct libcontacts_contact **contacts)
{
	size_t n;
	for (n = 0; contacts[n]; n++);
	qsort(contacts, n, sizeof(*contacts), contactidcmp);
	return n;
}

static void
free_contact_ids(char **ids)
{
	char **idp;
	for (idp = ids; *idp; idp++)
		free(*idp);
	free(ids);
}

static char *
estrdup(const char *s)
{
	char *ret;
	TEST((ret = strdup(s)));
	return ret;
}

static char **
dupstrlist(const char *s, ...)
{
	va_list ap;
	size_t n = 0, i = 0;
	char **ret;
	if (s) {
		va_start(ap, s);
		n += 1;
		while (va_arg(ap, const char *))
			n += 1;
		va_end(ap);
	}
	TEST((ret = calloc(n + 1, sizeof(*ret))));
	if (s) {
		ret[i++] = estrdup(s);
		va_start(ap, s);
		while ((s = va_arg(ap, const char *)))
			ret[i++] = estrdup(s);
		va_end(ap);
	}
	ret[i] = NULL;
	return ret;
}

static void *
ecalloc(size_t n, size_t m)
{
	char *ret;
	TEST((ret = calloc(n, m)));
	return ret;
}


int
main(void)
{
	struct libcontacts_contact contact, **contacts;
	struct passwd user;
	char *s, **ids;

	TEST(libcontacts_get_path("", NULL) == NULL && errno == EINVAL);
	memset(&user, 0, sizeof(user));
	TEST(libcontacts_get_path("", &user) == NULL && errno == EINVAL);
	user.pw_dir = "";
	TEST(libcontacts_get_path("", &user) == NULL && errno == EINVAL);
	user.pw_dir = "/var/empty";
	TEST(libcontacts_get_path(NULL, &user) == NULL && errno == EINVAL);
	user.pw_dir = "/var/empty";
	TEST((s = libcontacts_get_path("", &user)) && !strcmp(s, "/var/empty/.config/contacts/"));
	free(s);
	TEST((s = libcontacts_get_path("someone", &user)) && !strcmp(s, "/var/empty/.config/contacts/someone"));
	free(s);
	/* TODO test ENOMEM case */

	user.pw_dir = ".testdir";
	TEST(!mkdir(".testdir", 0700));
	TEST(!mkdir(".testdir/.config", 0700));
	TEST(!mkdir(".testdir/.config/contacts", 0700));

	touch(".testdir/.config/contacts/alpha");
	touch(".testdir/.config/contacts/.me");
	touch(".testdir/.config/contacts/.nobody");
	touch(".testdir/.config/contacts/beta");
	touch(".testdir/.config/contacts/.exclude");
	touch(".testdir/.config/contacts/gamma");

	TEST(!libcontacts_list_contacts(&ids, &user, 0));
	TEST(sort_contact_ids(ids) == 3);
	TEST(!strcmp(ids[0], "alpha"));
	TEST(!strcmp(ids[1], "beta"));
	TEST(!strcmp(ids[2], "gamma"));
	free_contact_ids(ids);
	TEST(!libcontacts_list_contacts(&ids, &user, 1));
	TEST(sort_contact_ids(ids) == 4);
	TEST(!strcmp(ids[0], ".me"));
	TEST(!strcmp(ids[1], "alpha"));
	TEST(!strcmp(ids[2], "beta"));
	TEST(!strcmp(ids[3], "gamma"));
	free_contact_ids(ids);
	/* TODO test error cases */

	contact.id = estrdup("alpha");
	contact.name = estrdup("name");
	contact.first_name = estrdup("first name");
	contact.last_name = estrdup("last name");
	contact.full_name = estrdup("full name");
	contact.nickname = estrdup("nickname");
	contact.photos = dupstrlist("photo 1", "photo 2", "photo 3", NULL);
	contact.groups = dupstrlist("group 1", "group 2", NULL);
	contact.notes = estrdup("line1\nline2\nline3\n");
	contact.blocks = NULL;
	contact.organisations = NULL;
	contact.emails = NULL;
	contact.pgpkeys = NULL;
	contact.numbers = NULL;
	contact.addresses = NULL;
	contact.sites = NULL;
	contact.chats = NULL;
	contact.birthday = NULL;
	contact.in_case_of_emergency = 1;
	contact.gender = LIBCONTACTS_UNSPECIFIED_GENDER;
	contact.unrecognised_data = dupstrlist("X-1 x", "X-2", NULL);
	TEST(!libcontacts_save_contact(&contact, &user));
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	TEST(!libcontacts_load_contact("beta", &contact, &user));
	TEST(contact.id && !strcmp(contact.id, "beta"));
	TEST(!contact.name);
	TEST(!contact.first_name);
	TEST(!contact.last_name);
	TEST(!contact.full_name);
	TEST(!contact.nickname);
	TEST(!contact.photos);
	TEST(!contact.groups);
	TEST(!contact.notes);
	TEST(!contact.blocks);
	TEST(!contact.organisations);
	TEST(!contact.emails);
	TEST(!contact.pgpkeys);
	TEST(!contact.numbers);
	TEST(!contact.addresses);
	TEST(!contact.sites);
	TEST(!contact.chats);
	TEST(!contact.birthday);
	TEST(!contact.in_case_of_emergency);
	TEST(!contact.gender);
	TEST(contact.gender == LIBCONTACTS_UNSPECIFIED_GENDER);
	TEST(!contact.unrecognised_data);
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	TEST(!libcontacts_load_contact("alpha", &contact, &user));
	TEST(contact.id && !strcmp(contact.id, "alpha"));
	TEST(contact.name && !strcmp(contact.name, "name"));
	TEST(contact.first_name && !strcmp(contact.first_name, "first name"));
	TEST(contact.last_name && !strcmp(contact.last_name, "last name"));
	TEST(contact.full_name && !strcmp(contact.full_name, "full name"));
	TEST(contact.nickname && !strcmp(contact.nickname, "nickname"));
	TEST(contact.photos);
	TEST(contact.photos[0] && !strcmp(contact.photos[0], "photo 1"));
	TEST(contact.photos[1] && !strcmp(contact.photos[1], "photo 2"));
	TEST(contact.photos[2] && !strcmp(contact.photos[2], "photo 3"));
	TEST(!contact.photos[3]);
	TEST(contact.groups);
	TEST(contact.groups[0] && !strcmp(contact.groups[0], "group 1"));
	TEST(contact.groups[1] && !strcmp(contact.groups[1], "group 2"));
	TEST(!contact.groups[2]);
	TEST(contact.notes && !strcmp(contact.notes, "line1\nline2\nline3\n"));
	TEST(!contact.blocks);
	TEST(!contact.organisations);
	TEST(!contact.emails);
	TEST(!contact.pgpkeys);
	TEST(!contact.numbers);
	TEST(!contact.addresses);
	TEST(!contact.sites);
	TEST(!contact.chats);
	TEST(!contact.birthday);
	TEST(contact.in_case_of_emergency == 1);
	TEST(contact.gender == LIBCONTACTS_UNSPECIFIED_GENDER);
	TEST(contact.unrecognised_data);
	TEST(contact.unrecognised_data[0] && !strcmp(contact.unrecognised_data[0], "X-1 x"));
	TEST(contact.unrecognised_data[1] && !strcmp(contact.unrecognised_data[1], "X-2"));
	TEST(!contact.unrecognised_data[2]);
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	contact.id = estrdup("alpha");
	contact.photos = ecalloc(1, sizeof(*contact.photos));
	contact.groups = ecalloc(1, sizeof(*contact.groups));
	contact.notes = estrdup("line1\nline2\nline3");
	contact.blocks = ecalloc(1, sizeof(*contact.blocks));
	contact.organisations = ecalloc(1, sizeof(*contact.organisations));
	contact.emails = ecalloc(1, sizeof(*contact.emails));
	contact.pgpkeys = ecalloc(1, sizeof(*contact.pgpkeys));
	contact.numbers = ecalloc(1, sizeof(*contact.numbers));
	contact.addresses = ecalloc(1, sizeof(*contact.addresses));
	contact.sites = ecalloc(1, sizeof(*contact.sites));
	contact.chats = ecalloc(1, sizeof(*contact.chats));
	contact.birthday = ecalloc(1, sizeof(*contact.birthday));
	contact.birthday->year = 1990;
	contact.gender = LIBCONTACTS_NOT_A_PERSON;
	contact.unrecognised_data = ecalloc(1, sizeof(*contact.unrecognised_data));
	TEST(!libcontacts_save_contact(&contact, &user));
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	TEST(!libcontacts_load_contact("alpha", &contact, &user));
	TEST(contact.id && !strcmp(contact.id, "alpha"));
	TEST(!contact.first_name);
	TEST(!contact.last_name);
	TEST(!contact.full_name);
	TEST(!contact.nickname);
	TEST(!contact.photos);
	TEST(!contact.groups);
	TEST(contact.notes && !strcmp(contact.notes,"line1\nline2\nline3\n"));
	TEST(!contact.blocks);
	TEST(!contact.organisations);
	TEST(!contact.emails);
	TEST(!contact.pgpkeys);
	TEST(!contact.numbers);
	TEST(!contact.addresses);
	TEST(!contact.sites);
	TEST(!contact.chats);
	TEST(contact.birthday);
	TEST(contact.birthday->year == 1990);
	TEST(!contact.birthday->month);
	TEST(!contact.birthday->day);
	TEST(!contact.birthday->before_on_common);
	TEST(!contact.birthday->unrecognised_data);
	TEST(contact.gender == LIBCONTACTS_NOT_A_PERSON);
	TEST(!contact.unrecognised_data);
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	contact.id = estrdup("alpha");
	contact.organisations = ecalloc(3, sizeof(*contact.organisations));
	contact.organisations[0] = ecalloc(1, sizeof(**contact.organisations));
	contact.organisations[0]->organisation = estrdup("org");
	contact.organisations[1] = ecalloc(1, sizeof(**contact.organisations));
	contact.organisations[1]->title = estrdup("title");
	contact.organisations[1]->unrecognised_data = dupstrlist("X-1", "X-2", NULL);
	contact.pgpkeys = ecalloc(3, sizeof(*contact.pgpkeys));
	contact.pgpkeys[0] = ecalloc(1, sizeof(**contact.pgpkeys));
	contact.pgpkeys[0]->id = estrdup("fingerprint");
	contact.pgpkeys[1] = ecalloc(1, sizeof(**contact.pgpkeys));
	contact.pgpkeys[1]->context = estrdup("ctx");
	contact.pgpkeys[1]->unrecognised_data = dupstrlist("X1", "X2", NULL);
	contact.birthday = ecalloc(1, sizeof(*contact.birthday));
	contact.birthday->month = 2;
	contact.birthday->day = 23;
	contact.birthday->before_on_common = 1;
	contact.gender = LIBCONTACTS_FEMALE;
	TEST(!libcontacts_save_contact(&contact, &user));
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	TEST(!libcontacts_load_contact("alpha", &contact, &user));
	TEST(contact.id && !strcmp(contact.id, "alpha"));
	TEST(!contact.first_name);
	TEST(!contact.last_name);
	TEST(!contact.full_name);
	TEST(!contact.nickname);
	TEST(!contact.photos);
	TEST(!contact.groups);
	TEST(!contact.notes);
	TEST(!contact.blocks);
	TEST(contact.organisations);
	TEST(contact.organisations[0]);
	TEST(contact.organisations[0]->organisation && !strcmp(contact.organisations[0]->organisation, "org"));
	TEST(!contact.organisations[0]->title);
	TEST(!contact.organisations[0]->unrecognised_data);
	TEST(contact.organisations[1]);
	TEST(!contact.organisations[1]->organisation);
	TEST(contact.organisations[1]->title && !strcmp(contact.organisations[1]->title, "title"));
	TEST(contact.organisations[1]->unrecognised_data);
	TEST(contact.organisations[1]->unrecognised_data[0] && !strcmp(contact.organisations[1]->unrecognised_data[0], "X-1"));
	TEST(contact.organisations[1]->unrecognised_data[1] && !strcmp(contact.organisations[1]->unrecognised_data[1], "X-2"));
	TEST(!contact.organisations[1]->unrecognised_data[2]);
	TEST(!contact.organisations[2]);
	TEST(!contact.emails);
	TEST(contact.pgpkeys);
	TEST(contact.pgpkeys[0]);
	TEST(!contact.pgpkeys[0]->context);
	TEST(contact.pgpkeys[0]->id && !strcmp(contact.pgpkeys[0]->id, "fingerprint"));
	TEST(!contact.pgpkeys[0]->unrecognised_data);
	TEST(contact.pgpkeys[1]);
	TEST(contact.pgpkeys[1]->context && !strcmp(contact.pgpkeys[1]->context, "ctx"));
	TEST(!contact.pgpkeys[1]->id);
	TEST(contact.pgpkeys[1]->unrecognised_data);
	TEST(contact.pgpkeys[1]->unrecognised_data[0] && !strcmp(contact.pgpkeys[1]->unrecognised_data[0], "X1"));
	TEST(contact.pgpkeys[1]->unrecognised_data[1] && !strcmp(contact.pgpkeys[1]->unrecognised_data[1], "X2"));
	TEST(!contact.pgpkeys[1]->unrecognised_data[2]);
	TEST(!contact.pgpkeys[2]);
	TEST(!contact.numbers);
	TEST(!contact.addresses);
	TEST(!contact.sites);
	TEST(!contact.chats);
	TEST(contact.birthday);
	TEST(!contact.birthday->year);
	TEST(contact.birthday->month == 2);
	TEST(contact.birthday->day == 23);
	TEST(contact.birthday->before_on_common == 1);
	TEST(!contact.birthday->unrecognised_data);
	TEST(contact.gender == LIBCONTACTS_FEMALE);
	TEST(!contact.unrecognised_data);
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	contact.id = estrdup("alpha");
	contact.emails = ecalloc(3, sizeof(*contact.emails));
	contact.emails[0] = ecalloc(1, sizeof(**contact.emails));
	contact.emails[0]->context = estrdup("con");
	contact.emails[0]->unrecognised_data = dupstrlist("X-1", "X-2", NULL);
	contact.emails[1] = ecalloc(1, sizeof(**contact.emails));
	contact.emails[1]->address = estrdup("addr");
	contact.chats = ecalloc(3, sizeof(*contact.chats));
	contact.chats[0] = ecalloc(1, sizeof(**contact.chats));
	contact.chats[0]->address = estrdup("a");
	contact.chats[0]->service = estrdup("s");
	contact.chats[1] = ecalloc(1, sizeof(**contact.chats));
	contact.chats[1]->context = estrdup("c");
	contact.chats[1]->unrecognised_data = dupstrlist("X1", "X2", NULL);
	TEST(!libcontacts_save_contact(&contact, &user));
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	TEST(!libcontacts_load_contact("alpha", &contact, &user));
	TEST(!contact.blocks);
	TEST(!contact.organisations);
	TEST(contact.emails);
	TEST(contact.emails[0]);
	TEST(contact.emails[0]->context && !strcmp(contact.emails[0]->context, "con"));
	TEST(!contact.emails[0]->address);
	TEST(contact.emails[0]->unrecognised_data);
	TEST(contact.emails[0]->unrecognised_data[0] && !strcmp(contact.emails[0]->unrecognised_data[0], "X-1"));
	TEST(contact.emails[0]->unrecognised_data[1] && !strcmp(contact.emails[0]->unrecognised_data[1], "X-2"));
	TEST(!contact.emails[0]->unrecognised_data[2]);
	TEST(contact.emails[1]);
	TEST(!contact.emails[1]->context);
	TEST(contact.emails[1]->address && !strcmp(contact.emails[1]->address, "addr"));
	TEST(!contact.emails[1]->unrecognised_data);
	TEST(!contact.emails[2]);
	TEST(!contact.pgpkeys);
	TEST(!contact.numbers);
	TEST(!contact.addresses);
	TEST(!contact.sites);
	TEST(contact.chats);
	TEST(contact.chats[0]);
	TEST(!contact.chats[0]->context);
	TEST(contact.chats[0]->service && !strcmp(contact.chats[0]->service, "s"));
	TEST(contact.chats[0]->address && !strcmp(contact.chats[0]->address, "a"));
	TEST(!contact.chats[0]->unrecognised_data);
	TEST(contact.chats[1]);
	TEST(contact.chats[1]->context && !strcmp(contact.chats[1]->context, "c"));
	TEST(!contact.chats[1]->service);
	TEST(!contact.chats[1]->address);
	TEST(contact.chats[1]->unrecognised_data);
	TEST(contact.chats[1]->unrecognised_data[0] && !strcmp(contact.chats[1]->unrecognised_data[0], "X1"));
	TEST(contact.chats[1]->unrecognised_data[1] && !strcmp(contact.chats[1]->unrecognised_data[1], "X2"));
	TEST(!contact.chats[1]->unrecognised_data[2]);
	TEST(!contact.chats[2]);
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	contact.numbers = ecalloc(5, sizeof(*contact.numbers));
	contact.numbers[0] = ecalloc(1, sizeof(**contact.numbers));
	contact.numbers[0]->context = estrdup("context");
	contact.numbers[0]->number = estrdup("number");
	contact.numbers[0]->unrecognised_data = dupstrlist(NULL);
	contact.numbers[1] = ecalloc(1, sizeof(**contact.numbers));
	contact.numbers[1]->is_mobile = 1;
	contact.numbers[1]->unrecognised_data = dupstrlist("X-1", "X-2", NULL);
	contact.numbers[2] = ecalloc(1, sizeof(**contact.numbers));
	contact.numbers[2]->context = estrdup("ctx");
	contact.numbers[2]->is_facsimile = 1;
	contact.numbers[3] = ecalloc(1, sizeof(**contact.numbers));
	contact.numbers[3]->number = estrdup("num");
	contact.numbers[3]->is_mobile = 2;
	contact.numbers[3]->is_facsimile = 2;
	contact.sites = ecalloc(3, sizeof(*contact.sites));
	contact.sites[0] = ecalloc(1, sizeof(**contact.sites));
	contact.sites[0]->context = estrdup("context");
	contact.sites[0]->address = estrdup("address");
	contact.sites[1] = ecalloc(1, sizeof(**contact.sites));
	contact.sites[1]->unrecognised_data = dupstrlist("X1", "X2", NULL);
	TEST(!libcontacts_save_contact(&contact, &user));
	TEST(contact.id && !strcmp(contact.id, "unnamed"));
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	TEST(!libcontacts_load_contact("unnamed", &contact, &user));
	TEST(!contact.blocks);
	TEST(!contact.organisations);
	TEST(!contact.emails);
	TEST(!contact.pgpkeys);
	TEST(contact.numbers);
	TEST(contact.numbers[0]);
	TEST(contact.numbers[0]->context && !strcmp(contact.numbers[0]->context, "context"));
	TEST(contact.numbers[0]->number && !strcmp(contact.numbers[0]->number, "number"));
	TEST(!contact.numbers[0]->is_mobile);
	TEST(!contact.numbers[0]->is_facsimile);
	TEST(!contact.numbers[0]->unrecognised_data);
	TEST(contact.numbers[1]);
	TEST(!contact.numbers[1]->context);
	TEST(!contact.numbers[1]->number);
	TEST(contact.numbers[1]->is_mobile);
	TEST(!contact.numbers[1]->is_facsimile);
	TEST(contact.numbers[1]->unrecognised_data);
	TEST(contact.numbers[1]->unrecognised_data[0] && !strcmp(contact.numbers[1]->unrecognised_data[0], "X-1"));
	TEST(contact.numbers[1]->unrecognised_data[1] && !strcmp(contact.numbers[1]->unrecognised_data[1], "X-2"));
	TEST(!contact.numbers[1]->unrecognised_data[2]);
	TEST(contact.numbers[2]);
	TEST(contact.numbers[2]->context && !strcmp(contact.numbers[2]->context, "ctx"));
	TEST(!contact.numbers[2]->number);
	TEST(!contact.numbers[2]->is_mobile);
	TEST(contact.numbers[2]->is_facsimile);
	TEST(!contact.numbers[2]->unrecognised_data);
	TEST(contact.numbers[3]);
	TEST(!contact.numbers[3]->context);
	TEST(contact.numbers[3]->number && !strcmp(contact.numbers[3]->number, "num"));
	TEST(!contact.numbers[3]->unrecognised_data);
	TEST(contact.numbers[3]->is_mobile);
	TEST(contact.numbers[3]->is_facsimile);
	TEST(!contact.numbers[4]);
	TEST(!contact.addresses);
	TEST(contact.sites);
	TEST(contact.sites[0]);
	TEST(contact.sites[0]->context && !strcmp(contact.sites[0]->context, "context"));
	TEST(contact.sites[0]->address && !strcmp(contact.sites[0]->address, "address"));
	TEST(!contact.sites[0]->unrecognised_data);
	TEST(contact.sites[1]);
	TEST(!contact.sites[1]->context);
	TEST(!contact.sites[1]->address);
	TEST(contact.sites[1]->unrecognised_data);
	TEST(contact.sites[1]->unrecognised_data[0] && !strcmp(contact.sites[1]->unrecognised_data[0], "X1"));
	TEST(contact.sites[1]->unrecognised_data[1] && !strcmp(contact.sites[1]->unrecognised_data[1], "X2"));
	TEST(!contact.sites[1]->unrecognised_data[2]);
	TEST(!contact.sites[2]);
	TEST(!contact.chats);
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	contact.addresses = ecalloc(5, sizeof(*contact.addresses));
	contact.addresses[0] = ecalloc(1, sizeof(**contact.addresses));
	contact.addresses[0]->context = estrdup("context");
	contact.addresses[0]->city = estrdup("city");
	contact.addresses[1] = ecalloc(1, sizeof(**contact.addresses));
	contact.addresses[1]->address = estrdup("address");
	contact.addresses[1]->country = estrdup("Sweden");
	contact.addresses[2] = ecalloc(1, sizeof(**contact.addresses));
	contact.addresses[2]->postcode = estrdup("11111");
	contact.addresses[3] = ecalloc(1, sizeof(**contact.addresses));
	contact.addresses[3]->have_coordinates = 1;
	contact.addresses[3]->latitude = 20.11;
	contact.addresses[3]->longitude = -13.32;
	contact.addresses[3]->unrecognised_data = dupstrlist("X-1", "X-2", NULL);
	TEST(!libcontacts_save_contact(&contact, &user));
	TEST(contact.id && !strcmp(contact.id, "unnamed-1"));
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	TEST(!libcontacts_load_contact("unnamed-1", &contact, &user));
	TEST(!contact.blocks);
	TEST(!contact.organisations);
	TEST(!contact.emails);
	TEST(!contact.pgpkeys);
	TEST(!contact.numbers);
	TEST(contact.addresses);
	TEST(contact.addresses[0]);
	TEST(contact.addresses[0]->context && !strcmp(contact.addresses[0]->context, "context"));
	TEST(!contact.addresses[0]->address);
	TEST(!contact.addresses[0]->postcode);
	TEST(contact.addresses[0]->city && !strcmp(contact.addresses[0]->city, "city"));
	TEST(!contact.addresses[0]->country);
	TEST(!contact.addresses[0]->have_coordinates);
	TEST(!contact.addresses[0]->unrecognised_data);
	TEST(contact.addresses[1]);
	TEST(!contact.addresses[1]->context);
	TEST(contact.addresses[1]->address && !strcmp(contact.addresses[1]->address, "address"));
	TEST(!contact.addresses[1]->postcode);
	TEST(!contact.addresses[1]->city);
	TEST(contact.addresses[1]->country && !strcmp(contact.addresses[1]->country, "Sweden"));
	TEST(!contact.addresses[1]->have_coordinates);
	TEST(!contact.addresses[1]->unrecognised_data);
	TEST(contact.addresses[2]);
	TEST(!contact.addresses[2]->context);
	TEST(!contact.addresses[2]->address);
	TEST(contact.addresses[2]->postcode && !strcmp(contact.addresses[2]->postcode, "11111"));
	TEST(!contact.addresses[2]->city);
	TEST(!contact.addresses[2]->country);
	TEST(!contact.addresses[2]->have_coordinates);
	TEST(!contact.addresses[2]->unrecognised_data);
	TEST(contact.addresses[3]);
	TEST(!contact.addresses[3]->context);
	TEST(!contact.addresses[3]->address);
	TEST(!contact.addresses[3]->postcode);
	TEST(!contact.addresses[3]->city);
	TEST(!contact.addresses[3]->country);
	TEST(contact.addresses[3]->have_coordinates);
	TEST(20.11 - 0.000001 <= contact.addresses[3]->latitude && contact.addresses[3]->latitude <= 20.11 + 0.000001);
	TEST(-13.32 - 0.000001 <= contact.addresses[3]->longitude && contact.addresses[3]->longitude <= -13.32 + 0.000001);
	TEST(contact.addresses[3]->unrecognised_data);
	TEST(contact.addresses[3]->unrecognised_data[0] && !strcmp(contact.addresses[3]->unrecognised_data[0], "X-1"));
	TEST(contact.addresses[3]->unrecognised_data[1] && !strcmp(contact.addresses[3]->unrecognised_data[1], "X-2"));
	TEST(!contact.addresses[3]->unrecognised_data[2]);
	TEST(!contact.addresses[4]);
	TEST(!contact.sites);
	TEST(!contact.chats);
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	contact.blocks = ecalloc(6, sizeof(*contact.blocks));
	contact.blocks[0] = ecalloc(1, sizeof(**contact.blocks));
	contact.blocks[0]->service = estrdup("service");
	contact.blocks[1] = ecalloc(1, sizeof(**contact.blocks));
	contact.blocks[1]->service = estrdup("srv");
	contact.blocks[1]->explicit = 1;
	contact.blocks[1]->shadow_block = LIBCONTACTS_BLOCK_OFF;
	contact.blocks[1]->soft_unblock = 8;
	contact.blocks[2] = ecalloc(1, sizeof(**contact.blocks));
	contact.blocks[2]->explicit = 2;
	contact.blocks[2]->shadow_block = LIBCONTACTS_BLOCK_BUSY;
	contact.blocks[2]->hard_unblock = 9;
	contact.blocks[3] = ecalloc(1, sizeof(**contact.blocks));
	contact.blocks[3]->shadow_block = LIBCONTACTS_BLOCK_IGNORE;
	contact.blocks[3]->soft_unblock = 10;
	contact.blocks[3]->hard_unblock = 11;
	contact.blocks[4] = ecalloc(1, sizeof(**contact.blocks));
	contact.blocks[4]->unrecognised_data = dupstrlist("X-1", "X-2", NULL);
	TEST(!libcontacts_save_contact(&contact, &user));
	TEST(contact.id && !strcmp(contact.id, "unnamed-2"));
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	TEST(!libcontacts_load_contact("unnamed-2", &contact, &user));
	TEST(contact.blocks);
	TEST(contact.blocks[0]);
	TEST(contact.blocks[0]->service && !strcmp(contact.blocks[0]->service, "service"));
	TEST(!contact.blocks[0]->explicit);
	TEST(contact.blocks[0]->shadow_block == LIBCONTACTS_SILENT);
	TEST(!contact.blocks[0]->soft_unblock);
	TEST(!contact.blocks[0]->hard_unblock);
	TEST(!contact.blocks[0]->unrecognised_data);
	TEST(contact.blocks[1]);
	TEST(contact.blocks[1]->service && !strcmp(contact.blocks[1]->service, "srv"));
	TEST(contact.blocks[1]->explicit == 1);
	TEST(contact.blocks[1]->shadow_block == LIBCONTACTS_BLOCK_OFF);
	TEST(contact.blocks[1]->soft_unblock == 8);
	TEST(!contact.blocks[1]->hard_unblock);
	TEST(!contact.blocks[1]->unrecognised_data);
	TEST(contact.blocks[2]);
	TEST(!contact.blocks[2]->service);
	TEST(contact.blocks[2]->explicit == 1);
	TEST(contact.blocks[2]->shadow_block == LIBCONTACTS_BLOCK_BUSY);
	TEST(!contact.blocks[2]->soft_unblock);
	TEST(contact.blocks[2]->hard_unblock == 9);
	TEST(!contact.blocks[2]->unrecognised_data);
	TEST(contact.blocks[3]);
	TEST(!contact.blocks[3]->service);
	TEST(!contact.blocks[3]->explicit);
	TEST(contact.blocks[3]->shadow_block == LIBCONTACTS_BLOCK_IGNORE);
	TEST(contact.blocks[3]->soft_unblock == 10);
	TEST(contact.blocks[3]->hard_unblock == 11);
	TEST(!contact.blocks[3]->unrecognised_data);
	TEST(contact.blocks[4]);
	TEST(!contact.blocks[4]->service);
	TEST(!contact.blocks[4]->explicit);
	TEST(!contact.blocks[4]->shadow_block);
	TEST(!contact.blocks[4]->soft_unblock);
	TEST(!contact.blocks[4]->hard_unblock);
	TEST(contact.blocks[4]->unrecognised_data);
	TEST(contact.blocks[4]->unrecognised_data[0] && !strcmp(contact.blocks[4]->unrecognised_data[0], "X-1"));
	TEST(contact.blocks[4]->unrecognised_data[1] && !strcmp(contact.blocks[4]->unrecognised_data[1], "X-2"));
	TEST(!contact.blocks[4]->unrecognised_data[2]);
	TEST(!contact.blocks[5]);
	TEST(!contact.organisations);
	TEST(!contact.emails);
	TEST(!contact.pgpkeys);
	TEST(!contact.numbers);
	TEST(!contact.addresses);
	TEST(!contact.sites);
	TEST(!contact.chats);
	libcontacts_contact_destroy(&contact);
	memset(&contact, 0, sizeof(contact));

	TEST(!libcontacts_load_contacts(&contacts, &user, 0));
	TEST(sort_contacts_by_id(contacts) == 6);
	TEST(contacts[0] && contacts[0]->id && !strcmp(contacts[0]->id, "alpha"));
	TEST(contacts[1] && contacts[1]->id && !strcmp(contacts[1]->id, "beta"));
	TEST(contacts[2] && contacts[2]->id && !strcmp(contacts[2]->id, "gamma"));
	TEST(contacts[3] && contacts[3]->id && !strcmp(contacts[3]->id, "unnamed"));
	TEST(contacts[4] && contacts[4]->id && !strcmp(contacts[4]->id, "unnamed-1"));
	TEST(contacts[5] && contacts[5]->id && !strcmp(contacts[5]->id, "unnamed-2"));
	TEST(!contacts[6]);
	libcontacts_contact_destroy(contacts[0]);
	libcontacts_contact_destroy(contacts[1]);
	libcontacts_contact_destroy(contacts[2]);
	libcontacts_contact_destroy(contacts[3]);
	libcontacts_contact_destroy(contacts[4]);
	libcontacts_contact_destroy(contacts[5]);
	free(contacts[0]);
	free(contacts[1]);
	free(contacts[2]);
	free(contacts[3]);
	free(contacts[4]);
	free(contacts[5]);
	free(contacts);

	TEST(!libcontacts_load_contacts(&contacts, &user, 1));
	TEST(sort_contacts_by_id(contacts) == 7);
	TEST(contacts[0] && contacts[0]->id && !strcmp(contacts[0]->id, ".me"));
	TEST(contacts[1] && contacts[1]->id && !strcmp(contacts[1]->id, "alpha"));
	TEST(contacts[2] && contacts[2]->id && !strcmp(contacts[2]->id, "beta"));
	TEST(contacts[3] && contacts[3]->id && !strcmp(contacts[3]->id, "gamma"));
	TEST(contacts[4] && contacts[4]->id && !strcmp(contacts[4]->id, "unnamed"));
	TEST(contacts[5] && contacts[5]->id && !strcmp(contacts[5]->id, "unnamed-1"));
	TEST(contacts[6] && contacts[6]->id && !strcmp(contacts[6]->id, "unnamed-2"));
	TEST(!contacts[7]);
	libcontacts_contact_destroy(contacts[0]);
	libcontacts_contact_destroy(contacts[1]);
	libcontacts_contact_destroy(contacts[2]);
	libcontacts_contact_destroy(contacts[3]);
	libcontacts_contact_destroy(contacts[4]);
	libcontacts_contact_destroy(contacts[5]);
	libcontacts_contact_destroy(contacts[6]);
	free(contacts[0]);
	free(contacts[1]);
	free(contacts[2]);
	free(contacts[3]);
	free(contacts[4]);
	free(contacts[5]);
	free(contacts[6]);
	free(contacts);

	/* TODO test all ways to parse ADDR:COORD */
	/* TODO test error cases in libcontacts_parse_contact */
	/* TODO test error cases in libcontacts_load_contact */
	/* TODO test error cases in libcontacts_load_contacts */
	/* TODO test error cases in libcontacts_format_contact */
	/* TODO test error cases in libcontacts_save_contact */

	/* TODO check for memory leaks */
	/* TODO check for file descriptor leaks */
	return 0;
}
