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
parse_coord(char *s, double *lat, double *lon)
{
	int withsign = 0;
	int saved_errno = errno;

	errno = 0;

	withsign = (s[0] == '-' || s[0] == '+');
	if (s[withsign] != '.' && !isdigit(s[withsign]))
		goto bad;
	*lat = strtod(s, &s);
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
	*lon = strtod(s, &s);
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
libcontacts_parse_contact(char *data, struct libcontacts_contact *contact)
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

	memset(contact, 0, sizeof(*contact));

	for (p = data; p; p = q) {
		q = strpbrk(p, "\n\r\f");
		if (q)
			*q++ = '\0';

		switch ((*p == ' ' || *p == '\t') ? state : 0) {
		default:
			state = 0;
			if (!*p);
			else if (TEST(p, "NAME") && !contact->name) {
				if (!(contact->name = strdup(getstr(p))))
					goto fail;

			} else if (TEST(p, "FNAME") && !contact->first_name) {
				if (!(contact->first_name = strdup(getstr(p))))
					goto fail;

			} else if (TEST(p, "LNAME") && !contact->last_name) {
				if (!(contact->last_name = strdup(getstr(p))))
					goto fail;

			} else if (TEST(p, "FLNAME") && !contact->full_name) {
				if (!(contact->full_name = strdup(getstr(p))))
					goto fail;

			} else if (TEST(p, "NICK") && !contact->nickname) {
				if (!(contact->nickname = strdup(getstr(p))))
					goto fail;

			} else if (TEST(p, "PHOTO")) {
				if (addstr(&contact->photos, getstr(p)))
					goto fail;

			} else if (TEST(p, "GROUP")) {
				if (addstr(&contact->groups, getstr(p)))
					goto fail;

			} else if (TEST(p, "NOTES")) {
				if (appendstr(&contact->notes, getstr(p)))
					goto fail;

			} else if (!strcmp(p, "ORG:")) {
				ADD(contact->organisations);
				state = 1;
				break;
			case 1:
				if (TEST(unindent(p), "ORG") && !contact->organisations[i]->organisation) {
					if (!(contact->organisations[i]->organisation = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "TITLE") && !contact->organisations[i]->title) {
					if (!(contact->organisations[i]->title = strdup(getstr(p))))
						goto fail;
				} else {
					if (addstr(&contact->organisations[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "EMAIL:")) {
				ADD(contact->emails);
				state = 2;
				break;
			case 2:
				if (TEST(unindent(p), "CTX") && !contact->emails[i]->context) {
					if (!(contact->emails[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "ADDR") && !contact->emails[i]->address) {
					if (!(contact->emails[i]->address = strdup(getstr(p))))
						goto fail;
				} else {
					if (addstr(&contact->emails[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "KEY:")) {
				ADD(contact->pgpkeys);
				state = 3;
				break;
			case 3:
				if (TEST(unindent(p), "CTX") && !contact->pgpkeys[i]->context) {
					if (!(contact->pgpkeys[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "ID") && !contact->pgpkeys[i]->id) {
					if (!(contact->pgpkeys[i]->id = strdup(getstr(p))))
						goto fail;
				} else {
					if (addstr(&contact->pgpkeys[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "PHONE:")) {
				ADD(contact->numbers);
				state = 4;
				break;
			case 4:
				if (TEST(unindent(p), "CTX") && !contact->numbers[i]->context) {
					if (!(contact->numbers[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "NUMBER") && !contact->numbers[i]->number) {
					if (!(contact->numbers[i]->number = strdup(getstr(p))))
						goto fail;
				} else if (!strcmp(unindent(p), "MOBILE")) {
					contact->numbers[i]->is_mobile = 1;
				} else if (!strcmp(unindent(p), "FAX")) {
					contact->numbers[i]->is_facsimile = 1;
				} else {
					if (addstr(&contact->numbers[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "ADDR:")) {
				ADD(contact->addresses);
				state = 5;
				break;
			case 5:
				if (TEST(unindent(p), "CTX") && !contact->addresses[i]->context) {
					if (!(contact->addresses[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "COUNTRY") && !contact->addresses[i]->country) {
					if (!(contact->addresses[i]->country = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "C/O") && !contact->addresses[i]->care_of) {
					if (!(contact->addresses[i]->care_of = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "ADDR") && !contact->addresses[i]->address) {
					if (!(contact->addresses[i]->address = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "CODE") && !contact->addresses[i]->postcode) {
					if (!(contact->addresses[i]->postcode = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "CITY") && !contact->addresses[i]->city) {
					if (!(contact->addresses[i]->city = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "COORD") && !contact->addresses[i]->have_coordinates) {
					if (!parse_coord(getstr(p),
					                 &contact->addresses[i]->latitude,
					                 &contact->addresses[i]->longitude)) {
						contact->addresses[i]->have_coordinates = 1;
					} else {
						if (addstr(&contact->addresses[i]->unrecognised_data, unindent(p)))
							goto fail;
					}
				} else {
					if (addstr(&contact->addresses[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "SITE:")) {
				ADD(contact->sites);
				state = 6;
				break;
			case 6:
				if (TEST(unindent(p), "CTX") && !contact->sites[i]->context) {
					if (!(contact->sites[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "ADDR") && !contact->sites[i]->address) {
					if (!(contact->sites[i]->address = strdup(getstr(p))))
						goto fail;
				} else {
					if (addstr(&contact->sites[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "CHAT:")) {
				ADD(contact->chats);
				state = 7;
				break;
			case 7:
				if (TEST(unindent(p), "CTX") && !contact->chats[i]->context) {
					if (!(contact->chats[i]->context = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "SRV") && !contact->chats[i]->service) {
					if (!(contact->chats[i]->service = strdup(getstr(p))))
						goto fail;
				} else if (TEST(unindent(p), "ADDR") && !contact->chats[i]->address) {
					if (!(contact->chats[i]->address = strdup(getstr(p))))
						goto fail;
				} else {
					if (addstr(&contact->chats[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "BLOCK:")) {
				ADD(contact->blocks);
				state = 8;
				break;
			case 8:
				if (TEST(unindent(p), "SRV") && !contact->blocks[i]->service) {
					if (!(contact->blocks[i]->service = strdup(getstr(p))))
						goto fail;
				} else if (!strcmp(p, "OFF") && !contact->blocks[i]->shadow_block) {
					contact->blocks[i]->shadow_block = LIBCONTACTS_BLOCK_OFF;
				} else if (!strcmp(p, "BUSY") && !contact->blocks[i]->shadow_block) {
					contact->blocks[i]->shadow_block = LIBCONTACTS_BLOCK_BUSY;
				} else if (!strcmp(p, "IGNORE") && !contact->blocks[i]->shadow_block) {
					contact->blocks[i]->shadow_block = LIBCONTACTS_BLOCK_IGNORE;
				} else if (!strcmp(p, "EXPLICIT")) {
					contact->blocks[i]->explicit = 1;
				} else if (TEST(unindent(p), "ASK") && !contact->blocks[i]->soft_unblock && (t = gettime(p))) {
					contact->blocks[i]->soft_unblock = t;
				} else if (TEST(unindent(p), "REMOVE") && !contact->blocks[i]->hard_unblock && (t = gettime(p))) {
					contact->blocks[i]->hard_unblock = t;
				} else {
					if (addstr(&contact->blocks[i]->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "BIRTH:") && !contact->birthday) {
				contact->birthday = calloc(1, sizeof(*contact->birthday));
				if (!contact->birthday)
					goto fail;
				state = 9;
				break;
			case 9:
				if (TEST(unindent(p), "YEAR") && !contact->birthday->year && (u = getposuint(p))) {
					contact->birthday->year = u;
				} else if (TEST(unindent(p), "MONTH") && !contact->birthday->month && (uc = getposuchar(p))) {
					contact->birthday->month = uc;
				} else if (TEST(unindent(p), "DAY") && !contact->birthday->day && (uc = getposuchar(p))) {
					contact->birthday->day = uc;
				} else if (!strcmp(unindent(p), "EARLY")) {
					contact->birthday->before_on_common = 1;
				} else {
					if (addstr(&contact->birthday->unrecognised_data, unindent(p)))
						goto fail;
				}
				break;

			} else if (!strcmp(p, "ICE")) {
				contact->in_case_of_emergency = 1;

			} else if (!strcmp(p, "NPERSON") && !contact->gender) {
				contact->gender = LIBCONTACTS_NOT_A_PERSON;

			} else if (!strcmp(p, "MALE") && !contact->gender) {
				contact->gender = LIBCONTACTS_MALE;

			} else if (!strcmp(p, "FEMALE") && !contact->gender) {
				contact->gender = LIBCONTACTS_FEMALE;

			} else {
				if (addstr(&contact->unrecognised_data, p))
					goto fail;
			}
		}
	}

	return 0;

fail:
	libcontacts_contact_destroy(contact);
	memset(contact, 0, sizeof(*contact));
	return -1;

#undef TEST
#undef ADD
}
