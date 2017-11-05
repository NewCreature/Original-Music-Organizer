#ifndef OMO_LIBRARY_CACHE_H
#define OMO_LIBRARY_CACHE_H

#include "t3f/t3f.h"
#include "library.h"

bool omo_load_library_cache(OMO_LIBRARY * lp, const char * fn);
bool omo_save_library_cache(OMO_LIBRARY * lp, const char * fn);
bool omo_load_library_artists_cache(OMO_LIBRARY * lp, const char * fn);
bool omo_save_library_artists_cache(OMO_LIBRARY * lp, const char * fn);
bool omo_load_library_albums_cache(OMO_LIBRARY * lp, const char * fn);
bool omo_save_library_albums_cache(OMO_LIBRARY * lp, const char * fn);
bool omo_load_library_songs_cache(OMO_LIBRARY * lp, const char * fn);
bool omo_save_library_songs_cache(OMO_LIBRARY * lp, const char * fn);

#endif
