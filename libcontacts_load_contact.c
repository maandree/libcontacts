/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libcontacts_load_contact(const char *id, struct libcontacts_contact *contactp, const struct passwd *user)
{
	int ret, fd, saved_errno;
	char *data = NULL, *path;
	size_t p = 0, n = 0;
	ssize_t r;
	void *new;

	if (!contactp) {
		errno = EINVAL;
		return -1;
	}

	path = libcontacts_get_path(id, user);
	if (!path)
		return -1;
	fd = open(path, O_RDONLY);
	free(path);
	if (fd < 0)
		return -1;

	for (;;) {
		if (p == n) {
			new = realloc(data, n + (8 << 10) + 1);
			if (!new)
				goto fail;
			n += 8 << 10;
			data = new;
		}
		r = read(fd, &data[p], n - p);
		if (r <= 0) {
			if (r)
				goto fail;
			break;
		}
		p += (size_t)r;
	}
	data[p] = '\0';

	if (memchr(data, '\0', p)) {
		errno = 0;
		goto fail;
	}

	close(fd);
	ret = libcontacts_parse_contact(data, contactp);
	free(data);
	if (!ret && !(contactp->id = strdup(id))) {
		libcontacts_contact_destroy(contactp);
		memset(contactp, 0, sizeof(*contactp));
		return -1;
	}
	return ret;

fail:
	saved_errno = errno;
	free(data);
	close(fd);
	errno = saved_errno;
	return -1;
}
