/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libcontacts_site_destroy(struct libcontacts_site *this)
{
	free(this->context);
	free(this->address);
	DESTROY_ALL_STRINGS(this->unrecognised_data);
}
