/* See LICENSE file for copyright and license details. */
#include "common.h"


static const char *digits[255] = {
	['#'] = "#", ['*'] = "*", ['+'] = "00",
	['0'] = "0", ['1'] = "1", ['2'] = "2", ['3'] = "3", ['4'] = "4",
	['5'] = "5", ['6'] = "6", ['7'] = "7", ['8'] = "8", ['9'] = "9",
	['A'] = "2", ['B'] = "2", ['C'] = "2",
	['D'] = "3", ['E'] = "3", ['F'] = "3",
	['G'] = "4", ['H'] = "4", ['I'] = "4",
	['J'] = "5", ['K'] = "5", ['L'] = "5",
	['M'] = "6", ['N'] = "6", ['O'] = "6",
	['P'] = "7", ['Q'] = "7", ['R'] = "7", ['S'] = "7",
	['T'] = "8", ['U'] = "8", ['V'] = "8",
	['W'] = "8", ['X'] = "9", ['Y'] = "9", ['Z'] = "9",
	['('] = "(", [')'] = ")" /* For skipping ranges, e.g. the 0 in +46(0)7… shall be skipped when +46 is included */
	/* Lower case letters are intentionally left out */
};

static char *
canonicalise(const char *number, const char *country, char **post_cccp)
{
	size_t nlen = 0, clen = 0, skip;
	const char *digit, *p;
	char *ret, *r;

	*post_cccp = NULL;

	for (p = number, skip = 0; *p; p++) {
		digit = digits[*p & 255];
		if (digit) {
			if (*digit == '(')
				skip += 1;
			else if (*digit == ')')
				skip -= !!skip;
			else
				nlen += digit[1] ? 2 : 1;
		}
	}

	for (p = country, skip = 0; *p; p++) {
		digit = digits[*p & 255];
		if (digit) {
			if (*digit == '(')
				skip += 1;
			else if (*digit == ')')
				skip -= !!skip;
			else
				clen += digit[1] ? 2 : 1;
		}
	}

	r = ret = malloc(nlen + clen + 3);
	if (!ret)
		return NULL;

	if (clen) {
		for (p = country, skip = 0; *p; p++) {
			digit = digits[*p & 255];
			if (digit) {
				if (*digit == '(') {
					skip += 1;
				} else if (*digit == ')') {
					skip -= !!skip;
				} else if (!skip) {
					*r++ = digit[0];
					if (digit[1])
						*r++ = digit[1];
				}
			}
		}
		if (clen < 2 || ret[0] != '0' || ret[1] != '0') {
			memmove(&ret[2], ret, clen);
			ret[0] = ret[1] = '0';
			clen += 2;
			r += 2;
		}
		*post_cccp = r;
	}

	for (p = number, skip = 0; *p; p++) {
		digit = digits[*p & 255];
		if (digit) {
			if (*digit == '(') {
				skip += 1;
			} else if (*digit == ')') {
				skip -= !!skip;
			} else if (!skip) {
				*r++ = digit[0];
				if (digit[1])
					*r++ = digit[1];
			}
		}
	}
	*r = '\0';

	if (r[clen] != '0') {
		*post_cccp = NULL;
		memmove(ret, &ret[clen], nlen + 1);
	} else {
		memmove(&ret[clen], &ret[clen + 1], nlen--);
		if (nlen >= 2 && ret[clen + 1] == '0') {
			if (nlen < clen || memcmp(&ret[0], &ret[clen], clen))
				*post_cccp = NULL;
			memmove(ret, &ret[clen], nlen + 1);
		}
	}

	if (ret[0] == '0' && ret[1] != '0')
		*post_cccp = &ret[1];

	return ret;
}

int
libcontacts_same_number(const char *a, const char *a_country, const char *b, const char *b_country)
{
	char *a_full, *b_full, *a_post_ccc, *b_post_ccc;
	int ret, a_has_ccc, b_has_ccc;

	if (!a || !*a || !b || !*b) {
		errno = EINVAL;
		return -1;
	}

	if (!a_country)
		a_country = "";
	if (!b_country)
		b_country = "";

	a_full = canonicalise(a, a_country, &a_post_ccc);
	if (!a_full)
		return -1;
	b_full = canonicalise(b, b_country, &b_post_ccc);
	if (!b_full) {
		free(a_full);
		return -1;
	}

	a_has_ccc = (a_full[0] == '0' && a_full[1] == '0');
	b_has_ccc = (b_full[0] == '0' && b_full[1] == '0');

	a = a_full;
	b = b_full;
	if (a_has_ccc != b_has_ccc) {
		if (a_post_ccc && b_post_ccc) {
			a = a_post_ccc;
			b = b_post_ccc;
		}
	}

	ret = !strcmp(a, b);
	free(a_full);
	free(b_full);
	return ret;
}
