#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include "resource.h"

static T3GUI_RESOURCE * t3gui_resource[T3GUI_MAX_RESOURCES] = {NULL};
static int t3gui_resources = 0;
static NINE_PATCH_BITMAP ** t3gui_bitmap[T3GUI_MAX_BITMAPS] = {NULL};
static int t3gui_bitmaps = 0;
static ALLEGRO_FONT ** t3gui_font[T3GUI_MAX_FONTS] = {NULL};
static int t3gui_fonts = 0;

static void t3gui_update_font(ALLEGRO_FONT * fp, ALLEGRO_FONT * new_fp)
{
    int i;

    for(i = 0; i < t3gui_fonts; i++)
    {
        if(*t3gui_font[i] == fp)
        {
            *t3gui_font[i] = new_fp;
        }
    }
}

static void t3gui_update_bitmap(NINE_PATCH_BITMAP * bp, NINE_PATCH_BITMAP * new_bp)
{
    int i;

    for(i = 0; i < t3gui_bitmaps; i++)
    {
        if(*t3gui_bitmap[i] == bp)
        {
            *t3gui_bitmap[i] = new_bp;
        }
    }
}

static T3GUI_RESOURCE * t3gui_find_resource(const char * fn, int data_i)
{
    int i;

    /* see if we have already loaded this resource */
    for(i = 0; i < t3gui_resources; i++)
    {
        if(!strcmp(fn ? fn : "", t3gui_resource[i]->path) && t3gui_resource[i]->data_i == data_i && t3gui_resource[i]->display == al_get_current_display())
        {
            return t3gui_resource[i];
        }
    }
    return NULL;
}

static T3GUI_RESOURCE * t3gui_get_resource(void)
{
    if(t3gui_resources < T3GUI_MAX_RESOURCES)
    {
        t3gui_resource[t3gui_resources] = malloc(sizeof(T3GUI_RESOURCE));
        if(t3gui_resource[t3gui_resources])
        {
            memset(t3gui_resource[t3gui_resources], 0, sizeof(T3GUI_RESOURCE));
            return t3gui_resource[t3gui_resources];
        }
    }
    return NULL;
}

bool t3gui_load_font(ALLEGRO_FONT ** fp, const char * fn, int size)
{
    T3GUI_RESOURCE * rp;

    if(t3gui_fonts >= T3GUI_MAX_FONTS)
    {
        return false;
    }
    rp = t3gui_find_resource(fn, size);
    if(rp)
    {
        *fp = rp->data;
        t3gui_font[t3gui_fonts] = fp;
        t3gui_fonts++;
        return true;
    }
    rp = t3gui_get_resource();
    if(rp)
    {
        if(!fn)
        {
            rp->data = al_create_builtin_font();
        }
        else
        {
            rp->data = al_load_ttf_font(fn, size, 0);
        }
        if(rp->data)
        {
            strcpy(rp->path, fn ? fn : "");
            rp->data_i = size;
            rp->type = fn ? T3GUI_RESOURCE_TYPE_FONT : T3GUI_RESOURCE_TYPE_DEFAULT_FONT;
            rp->display = al_get_current_display();
            t3gui_resources++;
            *fp = rp->data;
            t3gui_font[t3gui_fonts] = fp;
            t3gui_fonts++;
            return true;
        }
    }
    return false;
}

static int count_ranges(int * ranges)
{
  int i = 0;

  while(ranges[i] >= 0)
  {
    i++;
  }
  return i / 2;
}

static ALLEGRO_FONT * load_bitmap_font(const char * fn, int * ranges)
{
  ALLEGRO_FONT * fp = NULL;
  ALLEGRO_BITMAP * bp = NULL;
  int r;

  bp = al_load_bitmap(fn);
  if(!bp)
  {
    goto fail;
  }
  r = count_ranges(ranges);
  fp = al_grab_font_from_bitmap(bp, r, ranges);
  if(!fp)
  {
    goto fail;
  }
  al_destroy_bitmap(bp);
  return fp;

  fail:
  {
    if(bp)
    {
      al_destroy_bitmap(bp);
    }
    if(fp)
    {
      al_destroy_font(fp);
    }
  }
  return NULL;
}

bool t3gui_load_bitmap_font(ALLEGRO_FONT ** fp, const char * fn, int * ranges)
{
    T3GUI_RESOURCE * rp;
    int i, s;

    if(t3gui_fonts >= T3GUI_MAX_FONTS)
    {
        return false;
    }
    rp = t3gui_find_resource(fn, count_ranges(ranges));
    if(rp)
    {
        *fp = rp->data;
        t3gui_font[t3gui_fonts] = fp;
        t3gui_fonts++;
        return true;
    }
    rp = t3gui_get_resource();
    if(rp)
    {
      rp->data = load_bitmap_font(fn, ranges);
      if(rp->data)
      {
        strcpy(rp->path, fn);
        rp->data_i = count_ranges(ranges);
        s = sizeof(int) * (rp->data_i * 2 + 1);
        rp->data_a = malloc(s);
        for(i = 0; i < rp->data_i; i += 2)
        {
          rp->data_a[i] = ranges[i];
          rp->data_a[i + 1] = ranges[i + 1];
        }
        rp->data_a[i] = -1;
        rp->type = T3GUI_RESOURCE_TYPE_BITMAP_FONT;
        rp->display = al_get_current_display();
        t3gui_resources++;
        *fp = rp->data;
        t3gui_font[t3gui_fonts] = fp;
        t3gui_fonts++;
        return true;
      }
    }
    return false;
}

bool t3gui_load_bitmap(NINE_PATCH_BITMAP ** bp, const char * fn)
{
    T3GUI_RESOURCE * rp;

    if(t3gui_bitmaps >= T3GUI_MAX_BITMAPS)
    {
        return false;
    }
    rp = t3gui_find_resource(fn, 0);
    if(rp)
    {
        *bp = rp->data;
        t3gui_bitmap[t3gui_bitmaps] = bp;
        t3gui_bitmaps++;
        return true;
    }
    rp = t3gui_get_resource();
    if(rp)
    {
        rp->data = load_nine_patch_bitmap(fn);
        if(rp->data)
        {
            strcpy(rp->path, fn);
            rp->data_i = 0;
            rp->type = T3GUI_RESOURCE_TYPE_BITMAP;
            rp->display = al_get_current_display();
            t3gui_resources++;
            *bp = rp->data;
            t3gui_bitmap[t3gui_bitmaps] = bp;
            t3gui_bitmaps++;
            return true;
        }
    }
    return false;
}

static void t3gui_unload_resource(T3GUI_RESOURCE * rp)
{
    if(rp->data)
    {
        switch(rp->type)
        {
            case T3GUI_RESOURCE_TYPE_BITMAP:
            {
                if(rp->data)
                {
                    destroy_nine_patch_bitmap(rp->data);
                }
                break;
            }
            case T3GUI_RESOURCE_TYPE_FONT:
            case T3GUI_RESOURCE_TYPE_DEFAULT_FONT:
            {
                if(rp->data)
                {
                    al_destroy_font(rp->data);
                }
                break;
            }
            case T3GUI_RESOURCE_TYPE_BITMAP_FONT:
            {
                if(rp->data)
                {
                    al_destroy_font(rp->data);
                }
                if(rp->data_a)
                {
                  free(rp->data_a);
                }
                break;
            }
        }
    }
}

static int count_references(ALLEGRO_DISPLAY * dp, const char * fn, int data_i)
{
  int i;
  int c = 0;

  for(i = 0; i < t3gui_resources; i++)
  {
    if(dp == t3gui_resource[i]->display && !strcmp(fn, t3gui_resource[i]->path) && t3gui_resource[i]->data_i == data_i)
    {
      c++;
    }
  }
  return c;
}

void t3gui_unload_resources(ALLEGRO_DISPLAY * dp, bool delete)
{
  int i, j, r;

  r = t3gui_resources;
  for(i = r - 1; i >= 0; i--)
  {
    if(t3gui_resource[i] && (!dp || t3gui_resource[i]->display == dp))
    {
      if(count_references(t3gui_resource[i]->display, t3gui_resource[i]->path, t3gui_resource[i]->data_i) <= 1)
      {
        t3gui_unload_resource(t3gui_resource[i]);
      }
      if(delete)
      {
        free(t3gui_resource[i]);
        for(j = i; j < t3gui_resources - 1; j++)
        {
          t3gui_resource[j] = t3gui_resource[j + 1];
        }
        t3gui_resources--;
      }
    }
  }
}

bool t3gui_reload_resource(T3GUI_RESOURCE * rp)
{
    int ret = true;

    if(rp->data)
    {
        switch(rp->type)
        {
            case T3GUI_RESOURCE_TYPE_BITMAP:
            {
                NINE_PATCH_BITMAP * bp;
                bp = load_nine_patch_bitmap(rp->path);
                if(bp)
                {
                    t3gui_update_bitmap(rp->data, bp);
                    rp->data = bp;
                }
                else
                {
                    ret = false;
                }
                break;
            }
            case T3GUI_RESOURCE_TYPE_FONT:
            {
                ALLEGRO_FONT * fp;
                fp = al_load_font(rp->path, rp->data_i, 0);
                if(fp)
                {
                    t3gui_update_font(rp->data, fp);
                    rp->data = fp;
                }
                else
                {
                    ret = false;
                }
                break;
            }
            case T3GUI_RESOURCE_TYPE_DEFAULT_FONT:
            {
                ALLEGRO_FONT * fp;
                fp = al_create_builtin_font();
                if(fp)
                {
                    t3gui_update_font(rp->data, fp);
                    rp->data = fp;
                }
                else
                {
                    ret = false;
                }
                break;
            }
            case T3GUI_RESOURCE_TYPE_BITMAP_FONT:
            {
                ALLEGRO_FONT * fp;
                fp = load_bitmap_font(rp->path, rp->data_a);
                if(fp)
                {
                    t3gui_update_font(rp->data, fp);
                    rp->data = fp;
                }
                else
                {
                    ret = false;
                }
                break;
            }
        }
    }
    return ret;
}

bool t3gui_reload_resources(ALLEGRO_DISPLAY * dp)
{
    int i;
    bool ret = true;

    for(i = 0; i < t3gui_resources; i++)
    {
        if(t3gui_resource[i] && t3gui_resource[i]->display == dp)
        {
            if(!t3gui_reload_resource(t3gui_resource[i]))
            {
                ret = false;
            }
        }
    }
    return ret;
}

void t3gui_remove_font_reference(ALLEGRO_FONT ** fp)
{
    int i, j;

    for(i = 0; i < t3gui_fonts; i++)
    {
        if(t3gui_font[i] == fp)
        {
            for(j = i; j < t3gui_fonts - 1; j++)
            {
                t3gui_font[j] = t3gui_font[j + 1];
            }
            t3gui_fonts--;
            break;
        }
    }
}

void t3gui_remove_bitmap_reference(NINE_PATCH_BITMAP ** bp)
{
    int i, j;

    for(i = 0; i < t3gui_bitmaps; i++)
    {
        if(t3gui_bitmap[i] == bp)
        {
            for(j = i; j < t3gui_bitmaps - 1; j++)
            {
                t3gui_bitmap[j] = t3gui_bitmap[j + 1];
            }
            t3gui_bitmaps--;
            break;
        }
    }
}
