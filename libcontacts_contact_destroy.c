/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libcontacts_contact_destroy(struct libcontacts_contact *this)
{
	free(this->id);
	free(this->name);
	free(this->first_name);
	free(this->last_name);
	free(this->nickname);
	DESTROY_ALL(this->photos, free);
	DESTROY_ALL(this->groups, free);
	free(this->notes);
	DESTROY_ALL(this->blocks,        libcontacts_block_destroy);
	DESTROY_ALL(this->organisations, libcontacts_organisation_destroy);
	DESTROY_ALL(this->emails,        libcontacts_email_destroy);
	DESTROY_ALL(this->pgpkeys,       libcontacts_pgpkey_destroy);
	DESTROY_ALL(this->numbers,       libcontacts_number_destroy);
	DESTROY_ALL(this->addresses,     libcontacts_address_destroy);
	DESTROY_ALL(this->sites,         libcontacts_site_destroy);
	DESTROY_ALL(this->chats,         libcontacts_chat_destroy);
	libcontacts_birthday_destroy(this->birthday);
	DESTROY_ALL(this->unrecognised_data, free);
}
