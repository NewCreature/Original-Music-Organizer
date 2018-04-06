#ifndef OMO_TRACK_H
#define OMO_TRACK_H

typedef struct
{

	OMO_CODEC_HANDLER * codec_handler;
	void * codec_data;
	char extracted_filename[1024];

} OMO_TRACK;

OMO_TRACK * omo_load_track(OMO_ARCHIVE_HANDLER_REGISTRY * archive_handler_registry, OMO_CODEC_HANDLER_REGISTRY * codec_handler_registry, const char * fn, const char * subfn, const char * track, ALLEGRO_PATH * temp_path, const char * codec_handler_id);
void omo_unload_track(OMO_TRACK * tp);

#endif
