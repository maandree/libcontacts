/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libcontacts_pgpkey_destroy(struct libcontacts_pgpkey *this)
{
	free(this->context);
	free(this->id);
	DESTROY_ALL_STRINGS(this->unrecognised_data);
}
