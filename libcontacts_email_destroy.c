/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libcontacts_email_destroy(struct libcontacts_email *this)
{
	free(this->context);
	free(this->address);
	DESTROY_ALL_STRINGS(this->unrecognised_data);
}
