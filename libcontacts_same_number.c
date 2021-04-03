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
canonicalise(const char *number, const char *country)
{
	size_t nlen = 0, clen = 0, skip;
	const char *digit, *p;
	char *ret, *r;

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

	ret = malloc(nlen + clen + 3);
	if (!ret)
		return NULL;

	for (r = ret, p = number, skip = 0; *p; p++) {
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

	if (ret[0] != '0' || ret[1] == '0' || !*country)
		return ret;

	memmove(&ret[clen], &ret[1], nlen);

	for (r = ret, p = country, skip = 0; *p; p++) {
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

	if (ret[0] == '0' && ret[1] == '0')
		return ret;

	memmove(&ret[2], ret, nlen + clen);
	ret[0] = ret[1] = '0';
	return ret;
}

int
libcontacts_same_number(const char *a, const char *a_country, const char *b, const char *b_country)
{
	char *a_full, *b_full;
	int ret;

	if (!a || !*a || !b || !*b) {
		errno = EINVAL;
		return -1;
	}

	a_full = canonicalise(a, a_country ? a_country : "");
	if (!a_full)
		return -1;
	b_full = canonicalise(b, b_country ? b_country : "");
	if (!b_full) {
		free(a_full);
		return -1;
	}
	ret = !strcmp(a_full, b_full);
	free(a_full);
	free(b_full);
	return ret;
}
