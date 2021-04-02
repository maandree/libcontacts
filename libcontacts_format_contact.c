/* See LICENSE file for copyright and license details. */
#include "common.h"


int
libcontacts_format_contact(const struct libcontacts_contact *contact, char **datap)
{
	FILE *fp;
	size_t siz = 0;
	char **list, *p, *q;
	const char *suffix;
	struct libcontacts_block **blocks, *block;
	struct libcontacts_organisation **organisations, *organisation;
	struct libcontacts_email **emails, *email;
	struct libcontacts_pgpkey **pgpkeys, *pgpkey;
	struct libcontacts_number **numbers, *number;
	struct libcontacts_address **addresses, *address;
	struct libcontacts_site **sites, *site;
	struct libcontacts_chat **chats, *chat;

	*datap = NULL;
	fp = open_memstream(datap, &siz);
	if (!fp)
		return -1;

	if (contact->name)
		fprintf(fp, "NAME %s\n", contact->name);

	if (contact->first_name)
		fprintf(fp, "FNAME %s\n", contact->first_name);

	if (contact->last_name)
		fprintf(fp, "LNAME %s\n", contact->last_name);

	if (contact->nickname)
		fprintf(fp, "NICK %s\n", contact->nickname);

	if ((list = contact->photos))
		for (; *list; list++)
			fprintf(fp, "PHOTO %s\n", *list);

	if ((list = contact->groups))
		for (; *list; list++)
			fprintf(fp, "GROUP %s\n", *list);

	if ((p = contact->notes)) {
		for (; *p; p = q) {
			q = strchr(p, '\n');
			suffix = q ? "" : "\n";
			q = q ? &q[1] : strchr(p, '\0');
			fprintf(fp, "NOTES %.*s%s", (int)(q - p), p, suffix);
		}
	}

	if ((blocks = contact->blocks)) {
		for (; (block = *blocks); blocks++) {
			fprintf(fp, "BLOCK:\n");
			if (block->service)
				fprintf(fp, "\tSRV %s\n", block->service);
			if (block->explicit)
				fprintf(fp, "\tEXPLICIT\n");
			if (block->soft_unblock > 0)
				fprintf(fp, "\tASK %ji\n", block->soft_unblock);
			if (block->hard_unblock > 0)
				fprintf(fp, "\tREMOVE %ji\n", block->hard_unblock);
			if (block->shadow_block == LIBCONTACTS_BLOCK_OFF)
				fprintf(fp, "\tOFF\n");
			else if (block->shadow_block == LIBCONTACTS_BLOCK_BUSY)
				fprintf(fp, "\tBUSY\n");
			else if (block->shadow_block == LIBCONTACTS_BLOCK_IGNORE)
				fprintf(fp, "\tIGNORE\n");
			if ((list = block->unrecognised_data))
				for (; *list; list++)
					fprintf(fp, "\t%s\n", *list);
		}
	}

	if ((organisations = contact->organisations)) {
		for (; (organisation = *organisations); organisations++) {
			fprintf(fp, "ORG:\n");
			if (organisation->organisation)
				fprintf(fp, "\tORG %s\n", organisation->organisation);
			if (organisation->title)
				fprintf(fp, "\tTITLE %s\n", organisation->title);
			if ((list = organisation->unrecognised_data))
				for (; *list; list++)
					fprintf(fp, "\t%s\n", *list);
		}
	}

	if ((emails = contact->emails)) {
		for (; (email = *emails); emails++) {
			fprintf(fp, "EMAIL:\n");
			if (email->context)
				fprintf(fp, "\tCTX %s\n", email->context);
			if (email->address)
				fprintf(fp, "\tADDR %s\n", email->address);
			if ((list = email->unrecognised_data))
				for (; *list; list++)
					fprintf(fp, "\t%s\n", *list);
		}
	}

	if ((pgpkeys = contact->pgpkeys)) {
		for (; (pgpkey = *pgpkeys); pgpkeys++) {
			fprintf(fp, "KEY:\n");
			if (pgpkey->context)
				fprintf(fp, "\tCTX %s\n", pgpkey->context);
			if (pgpkey->id)
				fprintf(fp, "\tID %s\n", pgpkey->id);
			if ((list = pgpkey->unrecognised_data))
				for (; *list; list++)
					fprintf(fp, "\t%s\n", *list);
		}
	}

	if ((numbers = contact->numbers)) {
		for (; (number = *numbers); numbers++) {
			fprintf(fp, "PHONE:\n");
			if (number->context)
				fprintf(fp, "\tCTX %s\n", number->context);
			if (number->number)
				fprintf(fp, "\tNUMBER %s\n", number->number);
			if (number->is_mobile)
				fprintf(fp, "\tMOBILE\n");
			if (number->is_facsimile)
				fprintf(fp, "\tFAX\n");
			if ((list = number->unrecognised_data))
				for (; *list; list++)
					fprintf(fp, "\t%s\n", *list);
		}
	}

	if ((addresses = contact->addresses)) {
		for (; (address = *addresses); addresses++) {
			fprintf(fp, "ADDR:\n");
			if (address->context)
				fprintf(fp, "\tCTX %s\n", address->context);
			if (address->country)
				fprintf(fp, "\tCOUNTRY %s\n", address->country);
			if (address->care_of)
				fprintf(fp, "\tC/O %s\n", address->care_of);
			if (address->address)
				fprintf(fp, "\tADDR %s\n", address->address);
			if (address->postcode)
				fprintf(fp, "\tCODE %s\n", address->postcode);
			if (address->city)
				fprintf(fp, "\tCITY %s\n", address->city);
			if (address->have_coordinates)
				fprintf(fp, "\tCOORD %.20lg %.20lg\n", address->latitude, address->longitude);
			if ((list = address->unrecognised_data))
				for (; *list; list++)
					fprintf(fp, "\t%s\n", *list);
		}
	}

	if ((sites = contact->sites)) {
		for (; (site = *sites); sites++) {
			fprintf(fp, "SITE:\n");
			if (site->context)
				fprintf(fp, "\tCTX %s\n", site->context);
			if (site->address)
				fprintf(fp, "\tADDR %s\n", site->address);
			if ((list = site->unrecognised_data))
				for (; *list; list++)
					fprintf(fp, "\t%s\n", *list);
		}
	}

	if ((chats = contact->chats)) {
		for (; (chat = *chats); chats++) {
			fprintf(fp, "CHAT:\n");
			if (chat->context)
				fprintf(fp, "\tCTX %s\n", chat->context);
			if (chat->service)
				fprintf(fp, "\tSRV %s\n", chat->service);
			if (chat->address)
				fprintf(fp, "\tADDR %s\n", chat->address);
			if ((list = chat->unrecognised_data))
				for (; *list; list++)
					fprintf(fp, "\t%s\n", *list);
		}
	}

	if (contact->birthday) {
		if (contact->birthday->year && contact->birthday->day) {
			fprintf(fp, "BIRTH %04i-%02i-%02i\n",
				contact->birthday->year, contact->birthday->month, contact->birthday->day);
		} else if (contact->birthday->year && contact->birthday->month) {
			fprintf(fp, "BIRTH %04i-%02i\n", contact->birthday->year, contact->birthday->month);
		} else if (contact->birthday->year) {
			fprintf(fp, "BIRTH %04i\n", contact->birthday->year);
		} else if (contact->birthday->day) {
			fprintf(fp, "BIRTH %02i-%02\n", contact->birthday->month, contact->birthday->day);
		} else if (contact->birthday->month) {
			fprintf(fp, "BIRTH %02i\n", contact->birthday->month);
		}
	}

	if (contact->in_case_of_emergency)
		fprintf(fp, "ICE\n");

	if (contact->gender == LIBCONTACTS_NOT_A_PERSON)
		fprintf(fp, "NPERSON\n");
	else if (contact->gender == LIBCONTACTS_MALE)
		fprintf(fp, "MALE\n");
	else if (contact->gender == LIBCONTACTS_FEMALE)
		fprintf(fp, "FEMALE\n");

	if ((list = contact->unrecognised_data))
		for (; *list; list++)
			fprintf(fp, "%s\n", *list);

	if (fclose(fp)) {
		free(*datap);
		*datap = NULL;
		return -1;
	}

	return 0;
}
