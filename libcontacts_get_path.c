/* See LICENSE file for copyright and license details. */
#include "common.h"


char *
libcontacts_get_path(const char *id, const struct passwd *user)
{
	size_t len;
	char *buf;

	if (!id || !user || !user->pw_dir || !*user->pw_dir) {
		errno = EINVAL;
		return NULL;
	}

	len = strlen(user->pw_dir) + sizeof("/.config/contacts/") + strlen(id);
	buf = malloc(len);
	if (!buf)
		return NULL;

	stpcpy(stpcpy(stpcpy(buf, user->pw_dir), "/.config/contacts/"), id);
	return buf;
}
