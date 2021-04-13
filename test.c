/* See LICENSE file for copyright and license details. */
#include "common.h"


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

static size_t
sort_contact_ids(char **ids)
{
	size_t n;
	for (n = 0; ids[n]; n++);
	qsort(ids, n, sizeof(*ids), strpcmp);
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


int
main(void)
{
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
	touch(".testdir/.config/contacts/.groups");
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

	/* TODO test libcontacts_parse_contact */
	/* TODO test libcontacts_load_contact */
	/* TODO test libcontacts_load_contacts */
	/* TODO test libcontacts_format_contact */
	/* TODO test libcontacts_save_contact */
	/* TODO test libcontacts_contact_destroy */

	/* TODO check for memory leaks */
	/* TODO check for file descriptor leaks */
	return 0;
}
