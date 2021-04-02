/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libcontacts_number_destroy(struct libcontacts_number *this)
{
	free(this->context);
	free(this->number);
	DESTROY_ALL_STRINGS(this->unrecognised_data);
}
