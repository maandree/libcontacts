/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libcontacts_save_contact(struct libcontacts_contact *contact, const struct passwd *user)
{
	char *data = NULL, *path = NULL, *tmppath;
	int oflags = O_WRONLY | O_CREAT | O_TRUNC;
	int fd = -1, saved_errno = errno;
	ssize_t r;
	size_t p, n, num = 0;
	char *basenam = NULL;

	if (libcontacts_format_contact(contact, &data))
		goto fail;

	if (!contact->id) {
		oflags = O_WRONLY | O_CREAT | O_EXCL;
	generate_id:
		if (!num) {
			if (!contact->name || !*contact->name) {
				contact->id = strdup("unnamed");
				if (!contact->id)
					goto fail;
			} else {
				contact->id = strdup(contact->name);
				if (!contact->id)
					goto fail;
				for (p = 0; contact->id[p]; p++) {
					if (isalpha(contact->id[p]))
						contact->id[p] = (char)tolower(contact->id[p]);
					else
						contact->id[p] = '-';
				}
			}
		} else {
			free(contact->id);
			snprintf(NULL, 0, "%s-%zu%zn", basenam, num, &r);
			contact->id = malloc((size_t)r + 1);
			if (!contact->id)
				goto fail;
			sprintf(contact->id, "%s-%zu", basenam, num);
		}
	}

	path = libcontacts_get_path(contact->id, user);
	if (!path)
		goto fail;

	tmppath = alloca(strlen(path) + sizeof("~"));
	stpcpy(stpcpy(tmppath, path), "~");

	fd = open(tmppath, oflags, 0644);
	if (fd < 0) {
		if ((oflags & O_EXCL) && errno == EEXIST) {
			if (!num++) {
				basenam = contact->id;
				contact->id = NULL;
			}
			goto generate_id;
		}
		goto fail;
	}

	n = strlen(data);
	for (p = 0; p < n; p += (size_t)r) {
		r = write(fd, &data[p], n - p);
		if (r < 0)
			goto fail;
	}

	if (fsync(fd))
		goto fail;
	if (close(fd)) {
		fd = -1;
		goto fail;
	}
	fd = -1;

	if (rename(tmppath, path))
		goto fail;

	free(data);
	free(path);
	free(basenam);
	errno = saved_errno;
	return 0;

fail:
	saved_errno = errno;
	if (fd >= 0)
		close(fd);
	free(data);
	free(path);
	free(basenam);
	errno = saved_errno;
	return -1;
}
