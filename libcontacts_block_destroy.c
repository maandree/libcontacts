/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libcontacts_block_destroy(struct libcontacts_block *this)
{
	free(this->service);
	DESTROY_ALL_STRINGS(this->unrecognised_data);
}
