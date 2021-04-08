/* See LICENSE file for copyright and license details. */
#ifndef LIBCONTACTS_H
#define LIBCONTACTS_H

#include <pwd.h>
#include <time.h>


/**
 * Gender of contact
 */
enum libcontacts_gender {
	LIBCONTACTS_UNSPECIFIED_GENDER,
	LIBCONTACTS_NOT_A_PERSON,
	LIBCONTACTS_MALE,
	LIBCONTACTS_FEMALE
};

/**
 * Block type for contact
 */
enum libcontacts_block_type {
	LIBCONTACTS_SILENT,      /* The contact is blocked blocked, the phone shall not call its owner's attention */
	LIBCONTACTS_BLOCK_OFF,   /* The contact is blocked, phone shall appear as turned off */
	LIBCONTACTS_BLOCK_BUSY,  /* The contact is blocked, phone shall appear as turned busy */
	LIBCONTACTS_BLOCK_IGNORE /* The contact is blocked, phone shall appear as on but with no one answering */
};

/**
 * Block for contact
 */
struct libcontacts_block {
	/**
	 * The service the block is applied to,
	 * must be begin with a dot, except if
	 * it is:
	 * - ".call"    Telephone calls
	 * - ".sms"     SMS, MMS, and similar
	 * - ".global"  Block everywhere (least prioritised)
	 */
	char *service;
	int explicit;                             /* Whether to make an explicit block if possible */
	enum libcontacts_block_type shadow_block; /* How block shall appear unless explicit */
	time_t soft_unblock;                      /* When (positive) to ask whether to unblock, 0 if never */
	time_t hard_unblock;                      /* When (positive) to unblock, 0 if never */
	char **unrecognised_data;                 /* Data not recognised by the library */
};

/**
 * Organisation information for contact
 */
struct libcontacts_organisation {
	char *organisation;       /* Orginisation the contact belongs to */
	char *title;              /* The contact's title/role in the orginisation */
	char **unrecognised_data; /* Data not recognised by the library */
};

/**
 * E-mail information for contact
 */
struct libcontacts_email {
	char *context;            /* Work e-mail (which job?)? Personal e-mail? … */
	char *address;            /* E-mail address */
	char **unrecognised_data; /* Data not recognised by the library */
};

/**
 * PGP-keys for contact
 */
struct libcontacts_pgpkey {
	char *context;            /* Work key (which job?)? Personal key? … */
	char *id;                 /* The key's fingerprint */
	char **unrecognised_data; /* Data not recognised by the library */
};

/**
 * Telephone number for contact
 */
struct libcontacts_number {
	char *context;            /* Work phone (which job?)? Personal phone (mobile?, which home?)? … */
	char *number;             /* The telephone number */
	int is_mobile;            /* Is this a mobile phone (does it receive SMS)? */
	int is_facsimile;         /* Is this facsimile (fax) machine? */
	char **unrecognised_data; /* Data not recognised by the library */
};

/**
 * Address for contact
 */
struct libcontacts_address {
	char *context;            /* Work address (which job)? Home? Summer cabin? … */
	char *care_of;            /* Care of address, if any */
	char *address;            /* Address, all lines in one */
	char *postcode;           /* Post code */
	char *city;               /* Which city is the post code tied to? */
	char *country;            /* Which country? */
	int have_coordinates;     /* Are `.latitude` and `.longitude` defined? */
	double latitude;          /* Latitudal GPS coordinate */
	double longitude;         /* Longitudal GPS coordinate */
	char **unrecognised_data; /* Data not recognised by the library */
};

/**
 * Site (e.g. web and gopher) for contact
 */
struct libcontacts_site {
	char *context;            /* Work site (which job?)? Personal site (what is it used for?)? … */
	char *address;            /* Address to the site, including protocol */
	char **unrecognised_data; /* Data not recognised by the library */
};

/**
 * Chat address for contact
 */
struct libcontacts_chat {
	char *context;            /* Work account (which job?)? Personal account? … */
	char *service;            /* What service is the account, must not begin with a dot */
	char *address;            /* What is the name/address/number of the account */
	char **unrecognised_data; /* Data not recognised by the library */
};

/**
 * Birthday of contact
 */
struct libcontacts_birthday {
	unsigned int year;        /* asis,        0 for unknown */
	unsigned char month;      /* january = 1, 0 for unknown */
	unsigned char day;        /* asis,        0 for unknown */

	/**
	 * This is applicable only if the birthday
	 * is on a leap day. One non-leap years, a
	 * birthday that occurs on a leap day is
	 * observed the day after the leap day had
	 * it existed; but if this flag is set, the
	 * person observes his birthday the day
	 * before instead.
	 * 
	 * (Even if it is not true (as it wasn't in
	 * the past), the leap day is treated as if at
	 * the end of the month, for example a person
	 * born 1970-02-27, would celebrate his birthday
	 * on 1980-02-27, not on 1980-02-28; similarly,
	 * person born 1980-02-27 would celebrate his
	 * birthday on 1989-02-27, not on 1989-02-26.)
	 * 
	 * For example, if a person is born 2000-02-29,
	 * his birthday is observed 2011-03-01, but if
	 * this flag is set, he observes it on 2011-02-28
	 * instead.
	 */
	unsigned char before_on_common;

	char **unrecognised_data; /* Data not recognised by the library */
};


/**
 * Contact information
 */
struct libcontacts_contact {
	/**
	 * The ID of the contact, used to select filename
	 * 
	 * Must not begin with a dot, except if it is:
	 * - ".me"       The user himself.
	 * - ".nobody"   Unused data, such as created groups without any members.
	 * Additionally, it must not end with ~ or contain an /,
	 * and it should be short enough for a filename
	 */
	char *id;

	/**
	 * The name of the contact as it should be displayed
	 */
	char *name;

	/**
	 * The first name of the contact
	 */
	char *first_name;

	/**
	 * The last name(s) of the contact, if any
	 */
	char *last_name;

	/**
	 * The full name of the contact
	 */
	char *full_name;

	/**
	 * Nick name of the contact, if any
	 */
	char *nickname;

	/**
	 * Pathname to photoes of the contact, use
	 * absolute paths or paths relative to the
	 * user's home directory
	 * 
	 * Applications may desired which photo to
	 * use based on their size, but put the in
	 * order of preference
	 * 
	 * For each telephone number, applications
	 * should only use photos that are shared
	 * exactly between the contacts that share
	 * that telephone number
	 */
	char **photos;

	/**
	 * Groups the contact is a member of
	 */
	char **groups;

	/**
	 * Personal notes about the contact
	 */
	char *notes;

	/**
	 * Block information for contact
	 */
	struct libcontacts_block **blocks;

	/**
	 * Organisation information for contact
	 */
	struct libcontacts_organisation **organisations;

	/**
	 * E-mail information for contact
	 */
	struct libcontacts_email **emails;

	/**
	 * PGP-keys for the contact
	 */
	struct libcontacts_pgpkey **pgpkeys;

	/**
	 * Telephone number for the contact
	 * 
	 * Phone number can be shared, in case of an
	 * incoming call where the phone number is
	 * shared, the application shall list contacts
	 * that phone number
	 */
	struct libcontacts_number **numbers;

	/**
	 * Address for the contact
	 */
	struct libcontacts_address **addresses;

	/**
	 * Site (e.g. web and gopher) for the contact
	 */
	struct libcontacts_site **sites;

	/**
	 * Chat address for the contact
	 */
	struct libcontacts_chat **chats;

	/**
	 * Birthday of the contact
	 */
	struct libcontacts_birthday *birthday;

	/**
	 * Whether the contact shall be listed as an ICE
	 * (In Case of Emergency) contact that can be
	 * view without unlocking the phone
	 */
	int in_case_of_emergency;

	/**
	 * The gender of the contact
	 */
	enum libcontacts_gender gender;

	/**
	 * Data not recognised by the library
	 */
	char **unrecognised_data;
};


void libcontacts_contact_destroy(struct libcontacts_contact *);
int libcontacts_list_contacts(char ***, const struct passwd *);
int libcontacts_load_contact(const char *, struct libcontacts_contact *, const struct passwd *); /* errno = 0 if malformatted */
int libcontacts_load_contacts(struct libcontacts_contact ***, const struct passwd *);
int libcontacts_save_contact(struct libcontacts_contact *, const struct passwd *);
int libcontacts_same_number(const char *, const char *, const char *, const char *); /* might be removed in the future */

char *libcontacts_get_path(const char *, const struct passwd *);
int libcontacts_parse_contact(char *, struct libcontacts_contact *); /* does not load .id, not stored in file, but is the filename */
int libcontacts_format_contact(const struct libcontacts_contact *, char **);

void libcontacts_block_destroy(struct libcontacts_block *);
void libcontacts_organisation_destroy(struct libcontacts_organisation *);
void libcontacts_email_destroy(struct libcontacts_email *);
void libcontacts_pgpkey_destroy(struct libcontacts_pgpkey *);
void libcontacts_number_destroy(struct libcontacts_number *);
void libcontacts_address_destroy(struct libcontacts_address *);
void libcontacts_site_destroy(struct libcontacts_site *);
void libcontacts_chat_destroy(struct libcontacts_chat *);
void libcontacts_birthday_destroy(struct libcontacts_birthday *);


#endif
