/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libcontacts_address_destroy(struct libcontacts_address *this)
{
	free(this->context);
	free(this->country);
	free(this->care_of);
	free(this->address);
	free(this->postcode);
	free(this->city);
	DESTROY_ALL_STRINGS(this->unrecognised_data);
}
