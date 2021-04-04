/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libcontacts_birthday_destroy(struct libcontacts_birthday *this)
{
	DESTROY_ALL_STRINGS(this->unrecognised_data);
}
