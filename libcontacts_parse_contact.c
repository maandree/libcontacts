/* See LICENSE file for copyright and license details. */
#include "common.h"


ATTRIBUTE_PURE
static time_t
gettime(const char *data)
{
	time_t ret = 0;
	while (*data == ' ' || *data == '\t') data++;
	data = &strchr(data, ' ')[1];
	if ('1' > *data || *data > '9')
		return 0;
	for (; isdigit(*data); data++) {
		if (ret > (TIME_MAX - (*data & 15)) / 10)
			return 0;
		ret = ret * 10 + (*data & 15);
	}
	if (*data)
		return 0;
	return ret;
}

ATTRIBUTE_PURE
static unsigned int
getposuint(const char *data)
{
	unsigned int ret = 0;
	while (*data == ' ' || *data == '\t') data++;
	data = &strchr(data, ' ')[1];
	if ('1' > *data || *data > '9')
		return 0;
	for (; isdigit(*data); data++) {
		if (ret > (UINT_MAX - (*data & 15)) / 10)
			return 0;
		ret = ret * 10 + (*data & 15);
	}
	if (*data)
		return 0;
	return ret;
}

ATTRIBUTE_PURE
static unsigned char
getposuchar(const char *data)
{
	unsigned char ret = 0;
	while (*data == ' ' || *data == '\t') data++;
	data = &strchr(data, ' ')[1];
	if ('1' > *data || *data > '9')
		return 0;
	for (; isdigit(*data); data++) {
		if (ret > (UCHAR_MAX - (*data & 15)) / 10)
			return 0;
		ret = (unsigned char)(ret * 10 + (*data & 15));
	}
	if (*data)
		return 0;
	return ret;
}

ATTRIBUTE_PURE
static char *
getstr(char *data)
{
	while (*data == ' ' || *data == '\t') data++;
	return &strchr(data, ' ')[1];
}

ATTRIBUTE_PURE
static char *
unindent(char *data)
{
	while (*data == ' ' || *data == '\t') data++;
	return data;
}

static int
addstr(char ***listp, const char *new)
{
	size_t i;
	void *temp, *add;
	int error;
	add = strdup(new);
	if (!add)
		return -1;
	if (!*listp) {
		*listp = calloc(2, sizeof(char *));
		if (!*listp)
			goto fail_errno;
		(*listp)[0] = add;
	} else {
		for (i = 0; (*listp)[i]; i++);
		if (i > SIZE_MAX / sizeof(char *) + 2) {
			error = ENOMEM;
			goto fail;
		}
		temp = realloc(*listp, (i + 2) * sizeof(char *));
		if (!temp)
			goto fail_errno;
		*listp = temp;
		(*listp)[i + 0] = add;
		(*listp)[i + 1] = NULL;
	}
	return 0;
fail_errno:
	error = errno;
fail:
	free(add);
	errno = error;
	return -1;
}

static int
appendstr(char **strp, char *new)
{
	size_t newlen = strlen(new) + 2;
	size_t oldlen = 0;
	char *p;
	if (*strp)
		oldlen = strlen(*strp);
	newlen += oldlen;
	p = realloc(*strp, newlen);
	if (!p)
		return -1;
	*strp = p;
	stpcpy(stpcpy(&p[oldlen], new), "\n");
	return 0;
}

static int
parse_coord(const char *s, double *lat, double *lon)
{
	int withsign = 0;
	int saved_errno = errno;

	errno = 0;

	withsign = (s[0] == '-' || s[0] == '+');
	if (s[withsign] != '.' && !isdigit(s[withsign]))
		goto bad;
	*lat = strtod(s, (char **)(void *)&s);
	if (errno)
		return -1;
	if (!withsign && (s[0] == 'N' || s[0] == 'S')) {
		if (s[0] == 'S')
			*lat = -*lat;
		s = &s[1];
	}

	if (s[0] != ' ')
		goto bad;
	s = &s[1];

	withsign = (s[0] == '-' || s[0] == '+');
	if (s[withsign] != '.' && !isdigit(s[withsign]))
		goto bad;
	*lon = strtod(s, (char **)(void *)&s);
	if (errno)
		return -1;
	if (!withsign && (s[0] == 'E' || s[0] == 'W')) {
		if (s[0] == 'W')
			*lon = -*lon;
		s = &s[1];
	}

	if (s[0])
		goto bad;

	errno = saved_errno;
	return 0;

bad:
	errno = EINVAL;
	return -1;
}


int
libcontacts_parse_contact(char *data, struct libcontacts_contact *contactp)
{
#define TEST(S, L)\
	(!strncmp((test_tmp = (S)), L, sizeof(L) - 1) &&\
	 (test_tmp[sizeof(L) - 1] == ' ' || test_tmp[sizeof(L) - 1] == '\t'))

#define ADD(LIST)\
	do {\
		i = 0;\
		if (LIST)\
			for (; (LIST)[i]; i++);\
		if (i > SIZE_MAX / sizeof(*(LIST)) - 2) {\
			errno = ENOMEM;\
			goto fail;\
		}\
		temp = realloc((LIST), (i + 2) * sizeof(*(LIST)));\
		if (!temp)\
			goto fail;\
		(LIST) = temp;\
		(LIST)[i + 1] = NULL;\
		if (!((LIST)[i] = calloc(1, sizeof(**(LIST)))))\
			goto fail;\
	} while (0)

	char *p, *q, *test_tmp;
	size_t i = 0; /* initialised to make compiler happy */
	time_t t;
	void *temp;
	int state = 0;
	unsigned int u;
	unsigned char uc;
	char c = 0;

	memset(contactp, 0, sizeof(*contactp));

	for (p = data; p; p = q ? (*q = c, &q[1]) : NULL) {
		q = strpbrk(p, "\n\r\f");
		if (q) {
			c = *q;
			*q = '\0';
		}

		switch ((*p == ' ' || *p == '\t') ? state : 0) {
		default:
			state = 0;
			if (!*p);
			else if (TEST(p, "NAME") && !contactp->name) {
				if (!(contactp->name = strdup(getstr(p))))
					goto fail;

			} else if (TEST(p, "FNAME") && !contactp->first_name) {
				if (!(contactp->first_name = strdup(getstr(p))))
					goto fail;

			} else if (TEST(p, "LNAME") && !contactp->last_name) {
				if (!(contactp->last_name = strdup(getstr(p))))
					goto fail;

			} else if (TEST(p, "FLNAME") && !contactp->full_name) {
				if (!(contactp->full_name = strdup(getstr(p))))
					goto fail;

			} else if (TEST(p, "NICK") && !contactp->nickname) {
				if (!(contactp->nickname = strdup(getstr(p))))
					goto fail;

			} else if (TEST(p, "PHOTO")) {
				if (addstr(&contactp->photos, getstr(p)))
					goto fail;

			} else if (TEST(p, "GROUP")) {
				if (addstr(&contactp->groups, getstr(p)))
					goto fail;

			} else if (TEST(p, "NOTES")) {
				if (appendstr(&contactp->notes, getstr(p)))
					goto fail;

			} else if (!strcmp(p, "ORG:")) {
				ADD(contactp->organisations);
				state = 1;
				break;
			case 1:
				if (TEST(unindent(p), "ORG") && !contactp->organisations[i]->organisation) {
					if (!(contactp->organisations[i]->organisation = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "TITLE") && !contactp->organisations[i]->title) {
					if (!(contactp->organisations[i]->title = strdup(getstr(p))))
						goto fail;
				} else {
					if (addstr(&contactp->organisations[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "EMAIL:")) {
				ADD(contactp->emails);
				state = 2;
				break;
			case 2:
				if (TEST(unindent(p), "CTX") && !contactp->emails[i]->context) {
					if (!(contactp->emails[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "ADDR") && !contactp->emails[i]->address) {
					if (!(contactp->emails[i]->address = strdup(getstr(p))))
						goto fail;
				} else {
					if (addstr(&contactp->emails[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "KEY:")) {
				ADD(contactp->pgpkeys);
				state = 3;
				break;
			case 3:
				if (TEST(unindent(p), "CTX") && !contactp->pgpkeys[i]->context) {
					if (!(contactp->pgpkeys[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "ID") && !contactp->pgpkeys[i]->id) {
					if (!(contactp->pgpkeys[i]->id = strdup(getstr(p))))
						goto fail;
				} else {
					if (addstr(&contactp->pgpkeys[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "PHONE:")) {
				ADD(contactp->numbers);
				state = 4;
				break;
			case 4:
				if (TEST(unindent(p), "CTX") && !contactp->numbers[i]->context) {
					if (!(contactp->numbers[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "NUMBER") && !contactp->numbers[i]->number) {
					if (!(contactp->numbers[i]->number = strdup(getstr(p))))
						goto fail;
				} else if (!strcmp(unindent(p), "MOBILE")) {
					contactp->numbers[i]->is_mobile = 1;
				} else if (!strcmp(unindent(p), "FAX")) {
					contactp->numbers[i]->is_facsimile = 1;
				} else {
					if (addstr(&contactp->numbers[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "ADDR:")) {
				ADD(contactp->addresses);
				state = 5;
				break;
			case 5:
				if (TEST(unindent(p), "CTX") && !contactp->addresses[i]->context) {
					if (!(contactp->addresses[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "COUNTRY") && !contactp->addresses[i]->country) {
					if (!(contactp->addresses[i]->country = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "C/O") && !contactp->addresses[i]->care_of) {
					if (!(contactp->addresses[i]->care_of = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "ADDR") && !contactp->addresses[i]->address) {
					if (!(contactp->addresses[i]->address = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "CODE") && !contactp->addresses[i]->postcode) {
					if (!(contactp->addresses[i]->postcode = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "CITY") && !contactp->addresses[i]->city) {
					if (!(contactp->addresses[i]->city = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "COORD") && !contactp->addresses[i]->have_coordinates) {
					if (!parse_coord(getstr(p),
					                 &contactp->addresses[i]->latitude,
					                 &contactp->addresses[i]->longitude)) {
						contactp->addresses[i]->have_coordinates = 1;
					} else {
						if (addstr(&contactp->addresses[i]->unrecognised_data, unindent(p)))
							goto fail;
					}
				} else {
					if (addstr(&contactp->addresses[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "SITE:")) {
				ADD(contactp->sites);
				state = 6;
				break;
			case 6:
				if (TEST(unindent(p), "CTX") && !contactp->sites[i]->context) {
					if (!(contactp->sites[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "ADDR") && !contactp->sites[i]->address) {
					if (!(contactp->sites[i]->address = strdup(getstr(p))))
						goto fail;
				} else {
					if (addstr(&contactp->sites[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "CHAT:")) {
				ADD(contactp->chats);
				state = 7;
				break;
			case 7:
				if (TEST(unindent(p), "CTX") && !contactp->chats[i]->context) {
					if (!(contactp->chats[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "SRV") && !contactp->chats[i]->service) {
					if (!(contactp->chats[i]->service = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "ADDR") && !contactp->chats[i]->address) {
					if (!(contactp->chats[i]->address = strdup(getstr(p))))
						goto fail;
				} else {
					if (addstr(&contactp->chats[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "BLOCK:")) {
				ADD(contactp->blocks);
				state = 8;
				break;
			case 8:
				if (TEST(unindent(p), "SRV") && !contactp->blocks[i]->service) {
					if (!(contactp->blocks[i]->service = strdup(getstr(p))))
						goto fail;
				} else if (!strcmp(unindent(p), "OFF") && !contactp->blocks[i]->shadow_block) {
					contactp->blocks[i]->shadow_block = LIBCONTACTS_BLOCK_OFF;
				} else if (!strcmp(unindent(p), "BUSY") && !contactp->blocks[i]->shadow_block) {
					contactp->blocks[i]->shadow_block = LIBCONTACTS_BLOCK_BUSY;
				} else if (!strcmp(unindent(p), "IGNORE") && !contactp->blocks[i]->shadow_block) {
					contactp->blocks[i]->shadow_block = LIBCONTACTS_BLOCK_IGNORE;
				} else if (!strcmp(unindent(p), "EXPLICIT")) {
					contactp->blocks[i]->explicit = 1;
				} else if (TEST(unindent(p), "ASK") && !contactp->blocks[i]->soft_unblock && (t = gettime(p))) {
					contactp->blocks[i]->soft_unblock = t;
				} else if (TEST(unindent(p), "REMOVE") && !contactp->blocks[i]->hard_unblock && (t = gettime(p))) {
					contactp->blocks[i]->hard_unblock = t;
				} else {
					if (addstr(&contactp->blocks[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "BIRTH:") && !contactp->birthday) {
				contactp->birthday = calloc(1, sizeof(*contactp->birthday));
				if (!contactp->birthday)
					goto fail;
				state = 9;
				break;
			case 9:
				if (TEST(unindent(p), "YEAR") && !contactp->birthday->year && (u = getposuint(p))) {
					contactp->birthday->year = u;
				} else if (TEST(unindent(p), "MONTH") && !contactp->birthday->month && (uc = getposuchar(p))) {
					contactp->birthday->month = uc;
				} else if (TEST(unindent(p), "DAY") && !contactp->birthday->day && (uc = getposuchar(p))) {
					contactp->birthday->day = uc;
				} else if (!strcmp(unindent(p), "EARLY")) {
					contactp->birthday->before_on_common = 1;
				} else {
					if (addstr(&contactp->birthday->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "ICE")) {
				contactp->in_case_of_emergency = 1;

			} else if (!strcmp(p, "NPERSON") && !contactp->gender) {
				contactp->gender = LIBCONTACTS_NOT_A_PERSON;

			} else if (!strcmp(p, "MALE") && !contactp->gender) {
				contactp->gender = LIBCONTACTS_MALE;

			} else if (!strcmp(p, "FEMALE") && !contactp->gender) {
				contactp->gender = LIBCONTACTS_FEMALE;

			} else {
				if (addstr(&contactp->unrecognised_data, p))
					goto fail;
			}
		}
	}

	return 0;

fail:
	libcontacts_contact_destroy(contactp);
	memset(contactp, 0, sizeof(*contactp));
	return -1;

#undef TEST
#undef ADD
}
