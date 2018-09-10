#include "t3f.h"
#include "file.h"
#include "vector.h"
#include "view.h"

/* vector object creation */
T3F_VECTOR_OBJECT * t3f_create_vector_object(void)
{
	T3F_VECTOR_OBJECT * vp;

	vp = malloc(sizeof(T3F_VECTOR_OBJECT));
	if(!vp)
	{
		return NULL;
	}
	vp->segments = 0;
	return vp;
}

void t3f_destroy_vector_object(T3F_VECTOR_OBJECT * vp)
{
	int i;

	for(i = 0; i < vp->segments; i++)
	{
		free(vp->segment[i]);
	}
	free(vp);
}

bool t3f_add_vector_segment(T3F_VECTOR_OBJECT * vp, float sx, float sy, float sz, float ex, float ey, float ez, ALLEGRO_COLOR color, float thickness)
{
	if(vp->segments >= T3F_VECTOR_OBJECT_MAX_SEGMENTS)
	{
		return false;
	}
	vp->segment[vp->segments] = malloc(sizeof(T3F_VECTOR_SEGMENT));
	if(!vp->segment[vp->segments])
	{
		return false;
	}
	vp->segment[vp->segments]->point[0].x = sx;
	vp->segment[vp->segments]->point[0].y = sy;
	vp->segment[vp->segments]->point[0].z = sz;
	vp->segment[vp->segments]->point[1].x = ex;
	vp->segment[vp->segments]->point[1].y = ey;
	vp->segment[vp->segments]->point[1].z = ez;
	vp->segment[vp->segments]->color = color;
	vp->segment[vp->segments]->thickness = thickness;
	vp->segments++;
	return true;
}

bool t3f_remove_vector_segment(T3F_VECTOR_OBJECT * vp, unsigned int segment)
{
	int i;

	if((int)segment < vp->segments)
	{
		free(vp->segment[segment]);
		for(i = segment; i < vp->segments - 1; i++)
		{
			vp->segment[i] = vp->segment[i + 1];
		}
		vp->segments--;
		return true;
	}
	return false;
}

/* vector font creation */
T3F_VECTOR_FONT * t3f_create_vector_font(void)
{
	T3F_VECTOR_FONT * vfp;
	int i;

	vfp = malloc(sizeof(T3F_VECTOR_FONT));
	if(!vfp)
	{
		return NULL;
	}
	for(i = 0; i < T3F_VECTOR_FONT_MAX_CHARACTERS; i++)
	{
		vfp->character[i] = NULL;
	}
	return vfp;
}

void t3f_destroy_vector_font(T3F_VECTOR_FONT * vfp)
{
	int i;

	for(i = 0; i < T3F_VECTOR_FONT_MAX_CHARACTERS; i++)
	{
		if(vfp->character[i])
		{
			free(vfp->character[i]->object);
			free(vfp->character[i]);
		}
	}
	free(vfp);
}

bool t3f_add_vector_character(T3F_VECTOR_FONT * vfp, unsigned int character, T3F_VECTOR_OBJECT * vp, float width)
{
	vfp->character[character] = malloc(sizeof(T3F_VECTOR_FONT_CHARACTER));
	if(!vfp->character[character])
	{
		return false;
	}
	vfp->character[character]->object = vp;
	vfp->character[character]->width = width;
	return true;
}

bool t3f_remove_vector_character(T3F_VECTOR_FONT * vp, unsigned int character)
{
	free(vp->character[character]->object);
	free(vp->character[character]);
	vp->character[character] = NULL;
	return true;
}

/* vector object IO */
T3F_VECTOR_OBJECT * t3f_load_vector_object_f(ALLEGRO_FILE * fp)
{
	T3F_VECTOR_OBJECT * vp = NULL;
	T3F_VECTOR_SEGMENT segment;
	int r, g, b, a;
	char header[16] = {0};
	int i;
	int segments = 0;

	if(al_fread(fp, header, 16) != 16)
	{
		return NULL;
	}
	if(strcmp(header, "T3FV"))
	{
		return NULL;
	}
	vp = t3f_create_vector_object();
	if(!vp)
	{
		return NULL;
	}
	segments = al_fread32le(fp);
	for(i = 0; i < segments; i++)
	{
		segment.point[0].x = t3f_fread_float(fp);
		segment.point[0].y = t3f_fread_float(fp);
		segment.point[0].z = t3f_fread_float(fp);
		segment.point[1].x = t3f_fread_float(fp);
		segment.point[1].y = t3f_fread_float(fp);
		segment.point[1].z = t3f_fread_float(fp);
		r = al_fgetc(fp);
		g = al_fgetc(fp);
		b = al_fgetc(fp);
		a = al_fgetc(fp);
		segment.color = al_map_rgba(r, g, b, a);
		segment.thickness = t3f_fread_float(fp);
		if(!t3f_add_vector_segment(vp, segment.point[0].x, segment.point[0].y, segment.point[0].z, segment.point[1].x, segment.point[1].y, segment.point[1].z, segment.color, segment.thickness))
		{
			return NULL;
		}
	}
	return vp;
}

T3F_VECTOR_OBJECT * t3f_load_vector_object(const char * fn)
{
	ALLEGRO_FILE * fp;
	T3F_VECTOR_OBJECT * vp = NULL;

	fp = al_fopen(fn, "rb");
	if(!fp)
	{
		return NULL;
	}
	vp = t3f_load_vector_object_f(fp);
	al_fclose(fp);
	return vp;
}

bool t3f_save_vector_object_f(T3F_VECTOR_OBJECT * vp, ALLEGRO_FILE * fp)
{
	unsigned char r, g, b, a;
	char header[16] = {'T', '3', 'F', 'V'};
	int i;

	if(al_fwrite(fp, header, 16) != 16)
	{
		return false;
	}
	al_fwrite32le(fp, vp->segments);
	for(i = 0; i < vp->segments; i++)
	{
		t3f_fwrite_float(fp, vp->segment[i]->point[0].x);
		t3f_fwrite_float(fp, vp->segment[i]->point[0].y);
		t3f_fwrite_float(fp, vp->segment[i]->point[0].z);
		t3f_fwrite_float(fp, vp->segment[i]->point[1].x);
		t3f_fwrite_float(fp, vp->segment[i]->point[1].y);
		t3f_fwrite_float(fp, vp->segment[i]->point[1].z);
		al_unmap_rgba(vp->segment[i]->color, &r, &g, &b, &a);
		al_fputc(fp, r);
		al_fputc(fp, g);
		al_fputc(fp, b);
		al_fputc(fp, a);
		t3f_fwrite_float(fp, vp->segment[i]->thickness);
	}
	return true;
}

bool t3f_save_vector_object(T3F_VECTOR_OBJECT * vp, const char * fn)
{
	ALLEGRO_FILE * fp;
	bool ret;

	fp = al_fopen(fn, "wb");
	if(!fp)
	{
		return false;
	}
	ret = t3f_save_vector_object_f(vp, fp);
	al_fclose(fp);
	return ret;
}

/* vector font IO */
T3F_VECTOR_FONT * t3f_load_vector_font_f(ALLEGRO_FILE * fp)
{
	T3F_VECTOR_FONT * vfp = NULL;
	T3F_VECTOR_OBJECT * vp;
	float w;
	float max_y = 0.0;
	char header[16] = {0};
	int i, j;

	if(al_fread(fp, header, 16) != 16)
	{
		return NULL;
	}
	if(strcmp(header, "T3FVF"))
	{
		return NULL;
	}
	vfp = t3f_create_vector_font();
	if(!vfp)
	{
		return NULL;
	}
	for(i = 0; i < T3F_VECTOR_FONT_MAX_CHARACTERS; i++)
	{
		if(al_fgetc(fp))
		{
			vp = t3f_load_vector_object_f(fp);
			w = t3f_fread_float(fp);
			t3f_add_vector_character(vfp, i, vp, w);
			for(j = 0; j < vp->segments; j++)
			{
				if(vp->segment[j]->point[0].y > max_y)
				{
					max_y = vp->segment[j]->point[0].y;
				}
				if(vp->segment[j]->point[1].y > max_y)
				{
					max_y = vp->segment[j]->point[1].y;
				}
			}
		}
	}
	vfp->height = max_y;
	return vfp;
}

T3F_VECTOR_FONT * t3f_load_vector_font(const char * fn)
{
	ALLEGRO_FILE * fp;
	T3F_VECTOR_FONT * vfp = NULL;

	fp = al_fopen(fn, "rb");
	if(!fp)
	{
		return NULL;
	}
	vfp = t3f_load_vector_font_f(fp);
	al_fclose(fp);
	return vfp;
}

bool t3f_save_vector_font_f(T3F_VECTOR_FONT * vfp, ALLEGRO_FILE * fp)
{
	char header[16] = {'T', '3', 'F', 'V', 'F'};
	int i;

	if(al_fwrite(fp, header, 16) != 16)
	{
		return false;
	}
	for(i = 0; i < T3F_VECTOR_FONT_MAX_CHARACTERS; i++)
	{
		if(vfp->character[i])
		{
			al_fputc(fp, 1);
			t3f_save_vector_object_f(vfp->character[i]->object, fp);
			t3f_fwrite_float(fp, vfp->character[i]->width);
		}
		else
		{
			al_fputc(fp, 0);
		}
	}
	return true;
}

bool t3f_save_vector_font(T3F_VECTOR_FONT * vfp, const char * fn)
{
	ALLEGRO_FILE * fp;
	bool ret;

	fp = al_fopen(fn, "wb");
	if(!fp)
	{
		return false;
	}
	ret = t3f_save_vector_font_f(vfp, fp);
	al_fclose(fp);
	return ret;
}

/* vector font utility */
float t3f_get_vector_text_width(T3F_VECTOR_FONT * vfp, const char * text)
{
	return t3f_get_morphed_vector_text_width(vfp, 1.0, text);
}

float t3f_get_morphed_vector_text_width(T3F_VECTOR_FONT * vfp, float sx, const char * text)
{
	float width = 0.0;
	unsigned int i;

	for(i = 0; i < strlen(text); i++)
	{
		if(vfp->character[(int)text[i]])
		{
			width += vfp->character[(int)text[i]]->width * sx;
		}
	}
	return width;
}

float t3f_get_vector_text_height(T3F_VECTOR_FONT * vfp)
{
	return vfp->height;
}

float t3f_get_morphed_vector_text_height(T3F_VECTOR_FONT * vfp, float sy)
{
	return vfp->height * sy;
}

/* vector object rendering */
void t3f_draw_vector_object(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float tscale)
{
	t3f_draw_morphed_vector_object(vp, x, y, z, 1.0, 1.0, 1.0, tscale);
}

void t3f_draw_tinted_vector_object(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float tscale, ALLEGRO_COLOR color)
{
	t3f_draw_tinted_morphed_vector_object(vp, x, y, z, 1.0, 1.0, 1.0, tscale, color);
}

void t3f_draw_morphed_vector_object(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float sx, float sy, float sz, float tscale)
{
	int i;

	for(i = 0; i < vp->segments; i++)
	{
		al_draw_line(t3f_project_x(x + vp->segment[i]->point[0].x * sx, vp->segment[i]->point[0].z * sz + z) + 0.5, t3f_project_y(y + vp->segment[i]->point[0].y * sy, vp->segment[i]->point[0].z * sz + z) + 0.5, t3f_project_x(x + vp->segment[i]->point[1].x * sx, vp->segment[i]->point[1].z * sz + z) + 0.5, t3f_project_y(y + vp->segment[i]->point[1].y * sy, vp->segment[i]->point[1].z * sz + z) + 0.5, vp->segment[i]->color, vp->segment[i]->thickness * tscale);
	}
}

void t3f_draw_morphed_vector_object_extrusion(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float sx, float sy, float sz, float lz, float tscale)
{
	int i;

	for(i = 0; i < vp->segments; i++)
	{
		al_draw_line(t3f_project_x(x + vp->segment[i]->point[0].x * sx, vp->segment[i]->point[0].z * sz + z) + 0.5, t3f_project_y(y + vp->segment[i]->point[0].y * sy, vp->segment[i]->point[0].z * sz + z) + 0.5, t3f_project_x(x + vp->segment[i]->point[0].x * sx, vp->segment[i]->point[0].z * sz + z + lz) + 0.5, t3f_project_y(y + vp->segment[i]->point[0].y * sy, vp->segment[i]->point[0].z * sz + z + lz) + 0.5, vp->segment[i]->color, vp->segment[i]->thickness * tscale);
	}
}

void t3f_draw_tinted_morphed_vector_object(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float sx, float sy, float sz, float tscale, ALLEGRO_COLOR color)
{
	int i;

	for(i = 0; i < vp->segments; i++)
	{
		al_draw_line(t3f_project_x(x + vp->segment[i]->point[0].x * sx, vp->segment[i]->point[0].z * sz + z) + 0.5, t3f_project_y(y + vp->segment[i]->point[0].y * sy, vp->segment[i]->point[0].z * sz + z) + 0.5, t3f_project_x(x + vp->segment[i]->point[1].x * sx, vp->segment[i]->point[1].z * sz + z) + 0.5, t3f_project_y(y + vp->segment[i]->point[1].y * sy, vp->segment[i]->point[1].z * sz + z) + 0.5, color, vp->segment[i]->thickness * tscale);
	}
}

void t3f_draw_tinted_morphed_vector_object_extrusion(T3F_VECTOR_OBJECT * vp, float x, float y, float z, float sx, float sy, float sz, float lz, float tscale, ALLEGRO_COLOR color)
{
	int i;

	for(i = 0; i < vp->segments; i++)
	{
		al_draw_line(t3f_project_x(x + vp->segment[i]->point[0].x * sx, vp->segment[i]->point[0].z * sz + z) + 0.5, t3f_project_y(y + vp->segment[i]->point[0].y * sy, vp->segment[i]->point[0].z * sz + z) + 0.5, t3f_project_x(x + vp->segment[i]->point[0].x * sx, vp->segment[i]->point[0].z * sz + z + lz) + 0.5, t3f_project_y(y + vp->segment[i]->point[0].y * sy, vp->segment[i]->point[0].z * sz + z + lz) + 0.5, color, vp->segment[i]->thickness * tscale);
	}
}

/* vector font rendering */
void t3f_draw_vector_text(T3F_VECTOR_FONT * vfp, ALLEGRO_COLOR color, float x, float y, float z, float tscale, const char * text)
{
	unsigned int i;
	float ox = x;

	for(i = 0; i < strlen(text); i++)
	{
		if(vfp->character[(int)text[i]])
		{
			t3f_draw_tinted_vector_object(vfp->character[(int)text[i]]->object, ox, y, z, tscale, color);
			ox += vfp->character[(int)text[i]]->width;
		}
	}
}

void t3f_draw_morphed_vector_text(T3F_VECTOR_FONT * vfp, ALLEGRO_COLOR color, float x, float y, float z, float sx, float sy, float sz, float tscale, const char * text)
{
	unsigned int i;
	float ox = x;

	for(i = 0; i < strlen(text); i++)
	{
		if(vfp->character[(int)text[i]])
		{
			t3f_draw_tinted_morphed_vector_object(vfp->character[(int)text[i]]->object, ox, y, z, sx, sy, sz, tscale, color);
			ox += vfp->character[(int)text[i]]->width * sx;
		}
	}
}

void t3f_draw_morphed_vector_text_extrusion(T3F_VECTOR_FONT * vfp, ALLEGRO_COLOR color, float x, float y, float z, float sx, float sy, float sz, float lz, float tscale, const char * text)
{
	unsigned int i;
	float ox = x;

	for(i = 0; i < strlen(text); i++)
	{
		if(vfp->character[(int)text[i]])
		{
			t3f_draw_tinted_morphed_vector_object_extrusion(vfp->character[(int)text[i]]->object, ox, y, z, sx, sy, sz, lz, tscale, color);
			ox += vfp->character[(int)text[i]]->width * sx;
		}
	}
}
