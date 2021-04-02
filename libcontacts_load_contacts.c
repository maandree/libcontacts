/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libcontacts_load_contacts(struct libcontacts_contact ***contactsp, const struct passwd *user)
{
	int saved_errno = errno;
	char **ids;
	void *temp;
	size_t i, j, n;
	struct libcontacts_contact contact;

	*contactsp = NULL;

	if (libcontacts_list_contacts(&ids, user))
		return -1;

	for (n = 0; ids[n]; n++);
	*contactsp = calloc(n + 1, sizeof(**contactsp));
	if (!*contactsp)
		goto fail;

	for (i = j = 0; i < n; i++) {
		memset(&contact, 0, sizeof(contact));
		if (libcontacts_load_contact(ids[i], &contact, user)) {
			switch (errno) {
			case ENOENT:
			case EACCES:
				continue;
			default:
				goto fail;
			}
		}
		(*contactsp)[j] = calloc(1, sizeof(***contactsp));
		if (!(*contactsp)[j])
			goto fail;
		*(*contactsp)[j++] = contact;
	}

	for (i = 0; i < n; i++)
		free(ids[i]);
	free(ids);
	errno = saved_errno;
	return 0;

fail:
	for (i = 0; i < n; i++)
		free(ids[i]);
	free(ids);
	if ((temp = *contactsp)) {
		for (; **contactsp; ++*contactsp) {
			libcontacts_contact_destroy(**contactsp);
			free(**contactsp);
		}
	}
	free(temp);
	*contactsp = NULL;
	return -1;
}
