/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libcontacts_organisation_destroy(struct libcontacts_organisation *this)
{
	free(this->organisation);
	free(this->title);
	DESTROY_ALL_STRINGS(this->unrecognised_data);
}
