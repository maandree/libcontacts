/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libcontacts_list_contacts(char ***idsp, const struct passwd *user)
{
	char *dirnam;
	DIR *dir;
	struct dirent *f;
	size_t i = 0;
	void *new;
	int saved_errno = errno;

	*idsp = NULL;

	dirnam = libcontacts_get_path("", user);
	if (!dirnam)
		return -1;

	dir = opendir(dirnam);
	if (!dir) {
		if (errno == ENOENT) {
			errno = saved_errno;
			new = malloc(sizeof(**idsp));
			if (new) {
				*idsp = new;
				**idsp = NULL;
				free(dirnam);
				return 0;
			}
		}
		free(dirnam);
		return -1;
	}

	goto start;
	while ((f = readdir(dir))) {
		if (strchr(f->d_name, '.'))
			continue;
		if (!f->d_name[0] || strchr(f->d_name, '\0')[-1] == '~')
			continue;
		if (!((*idsp)[i++] = strdup(f->d_name)))
			goto fail;
	start:
		new = realloc(*idsp, (i + 1) * sizeof(**idsp));
		if (!new)
			goto fail;
		*idsp = new;
	}
	(*idsp)[i] = NULL;

	if (errno)
		goto fail;
	closedir(dir);
	free(dirnam);
	errno = saved_errno;
	return 0;

fail:
	saved_errno = errno;
	closedir(dir);
	free(dirnam);
	if (*idsp) {
		for (i = 0; (*idsp)[i]; i++)
			free((*idsp)[i]);
		free(*idsp);
	}
	errno = saved_errno;
	return -1;
}
