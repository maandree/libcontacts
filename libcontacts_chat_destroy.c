/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libcontacts_chat_destroy(struct libcontacts_chat *this)
{
	free(this->context);
	free(this->service);
	free(this->address);
	DESTROY_ALL(this->unrecognised_data, free);
}
