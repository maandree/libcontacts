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


int
main(void)
{
	struct passwd user;
	char *s;

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
	TEST(!mkdir(user.pw_dir, 0777));

	/* TODO test libcontacts_list_contacts */
	/* TODO test libcontacts_parse_contact */
	/* TODO test libcontacts_load_contact */
	/* TODO test libcontacts_load_contacts */
	/* TODO test libcontacts_format_contact */
	/* TODO test libcontacts_save_contact */

	/* TODO check for memory leaks */
	return 0;
}
