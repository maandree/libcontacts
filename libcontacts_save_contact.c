/* See LICENSE file for copyright and license details. */
#include "common.h"


static int
makedirs(char *path)
{
	char *p, *last;

	last = strrchr(path, '/');
	if (!last)
		return 0;
	*last = '\0';

	for (p = path; *p == '/'; p++);

	while (*p) {
		for (; *p && *p != '/'; p++);
		*p = '\0';
		if (mkdir(path, 0777) && errno != EEXIST) {
			*last = *p = '/';
			return -1;
		}
		for (*p++ = '/'; *p == '/'; p++);
	}

	*last = '/';
	return 0;
}

int
libcontacts_save_contact(struct libcontacts_contact *contact, const struct passwd *user)
{
	char *data = NULL, *path = NULL, *tmppath = NULL;
	int oflags = O_WRONLY | O_CREAT | O_TRUNC, newid = 0;
	int fd = -1, saved_errno = errno, dirs_created = 0;
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
				newid = 1;
			} else {
				contact->id = strdup(contact->name);
				if (!contact->id)
					goto fail;
				newid = 1;
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

	tmppath = malloc(strlen(path) + sizeof("~"));
	stpcpy(stpcpy(tmppath, path), "~");

	if (oflags & O_EXCL) {
	open_excl_again:
		fd = open(path, oflags, 0666);
		if (fd < 0) {
			if (errno == ENOENT && !dirs_created) {
				if (makedirs(path))
					goto fail;
				dirs_created = 1;
				goto open_excl_again;
			}
			if (errno != EEXIST)
				goto fail;
			if (!num++) {
				basenam = contact->id;
				contact->id = NULL;
			}
			free(path);
			free(tmppath);
			goto generate_id;
		}
		close(fd);
		oflags ^= O_EXCL ^ O_TRUNC;
		dirs_created = 1;
	}

open_again:
	fd = open(tmppath, oflags, 0666);
	if (fd < 0) {
		if (errno == ENOENT && !dirs_created) {
			if (makedirs(path))
				goto fail;
			dirs_created = 1;
			goto open_again;
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
	free(tmppath);
	free(basenam);
	errno = saved_errno;
	return 0;

fail:
	saved_errno = errno;
	if (fd >= 0)
		close(fd);
	free(data);
	free(path);
	free(tmppath);
	free(basenam);
	if (newid) {
		free(contact->id);
		contact->id = NULL;
	}
	errno = saved_errno;
	return -1;
}
