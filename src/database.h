#ifndef OMO_DATABASE_H
#define OMO_DATABASE_H

typedef struct
{

	char * filename;
	ALLEGRO_CONFIG * config;
	ALLEGRO_MUTEX * mutex;

} OMO_DATABASE;

OMO_DATABASE * omo_create_database(const char * fn);
void omo_destroy_database(OMO_DATABASE * dp);
bool omo_save_database(OMO_DATABASE * dp);

void omo_set_database_value(OMO_DATABASE * dp, const char * section, const char * key, const char * val);
const char * omo_get_database_value(OMO_DATABASE * dp, const char * section, const char * key);
void omo_remove_database_key(OMO_DATABASE * dp, const char * section, const char * key);

#endif
