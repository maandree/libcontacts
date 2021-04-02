/* See LICENSE file for copyright and license details. */
#include "common.h"


void
libcontacts_contact_destroy(struct libcontacts_contact *this)
{
	free(this->id);
	free(this->name);
	free(this->first_name);
	free(this->last_name);
	free(this->full_name);
	free(this->nickname);
	DESTROY_ALL_STRINGS(this->photos);
	DESTROY_ALL_STRINGS(this->groups);
	free(this->notes);
	DESTROY_ALL_OBJECTS(this->blocks,        libcontacts_block_destroy);
	DESTROY_ALL_OBJECTS(this->organisations, libcontacts_organisation_destroy);
	DESTROY_ALL_OBJECTS(this->emails,        libcontacts_email_destroy);
	DESTROY_ALL_OBJECTS(this->pgpkeys,       libcontacts_pgpkey_destroy);
	DESTROY_ALL_OBJECTS(this->numbers,       libcontacts_number_destroy);
	DESTROY_ALL_OBJECTS(this->addresses,     libcontacts_address_destroy);
	DESTROY_ALL_OBJECTS(this->sites,         libcontacts_site_destroy);
	DESTROY_ALL_OBJECTS(this->chats,         libcontacts_chat_destroy);
	if (this->birthday) {
		libcontacts_birthday_destroy(this->birthday);
		free(this->birthday);
	}
	DESTROY_ALL_STRINGS(this->unrecognised_data);
}
