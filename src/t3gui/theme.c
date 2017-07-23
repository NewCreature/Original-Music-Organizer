#include "theme.h"
#include "resource.h"

static T3GUI_THEME t3gui_default_theme;

static bool create_default_theme(T3GUI_THEME * theme)
{
    ALLEGRO_BITMAP * bp;
    ALLEGRO_FONT * font;
    ALLEGRO_STATE old_state;
    char np[7][7] =
    {
        {0, 0, 0, 1, 0, 0, 0},
        {0, 1, 1, 1, 1, 1, 0},
        {0, 1, 2, 2, 2, 1, 0},
        {1, 1, 2, 2, 2, 1, 0},
        {0, 1, 2, 2, 2, 1, 0},
        {0, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0, 0, 0}
    };
    int i, j;

    bp = al_create_bitmap(7, 7);
    if(!bp)
    {
        return false;
    }
    al_store_state(&old_state, ALLEGRO_STATE_TRANSFORM | ALLEGRO_STATE_TARGET_BITMAP);
    al_set_target_bitmap(bp);
    al_lock_bitmap(bp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);
    al_clear_to_color(al_map_rgba_f(0.0, 0.0, 0.0, 0.0));
    for(i = 0; i < 7; i++)
    {
        for(j = 0; j < 7; j++)
        {
            switch(np[j][i])
            {
                case 1:
                {
                    al_put_pixel(j, i, al_map_rgba_f(0.0, 0.0, 0.0, 1.0));
                    break;
                }
                case 2:
                {
                    al_put_pixel(j, i, al_map_rgba_f(1.0, 1.0, 1.0, 1.0));
                    break;
                }
            }
        }
    }
    al_unlock_bitmap(bp);
    al_restore_state(&old_state);
    theme->bitmap[0] = create_nine_patch_bitmap(bp, true);
    if(!theme->bitmap[0])
    {
        return false;
    }
    font = al_create_builtin_font();
    if(!font)
    {
        return false;
    }
    for(i = 0; i < T3GUI_ELEMENT_STATES; i++)
    {
        theme->state[i].bitmap[0] = theme->bitmap[0];
        theme->state[i].color[T3GUI_THEME_COLOR_BG] = t3gui_white;
        theme->state[i].color[T3GUI_THEME_COLOR_MG] = t3gui_silver;
        theme->state[i].color[T3GUI_THEME_COLOR_FG] = t3gui_black;
        theme->state[i].color[T3GUI_THEME_COLOR_EG] = t3gui_red;
        theme->state[i].font = font;
        theme->state[i].aux_font = NULL;
    }
    return true;
}

T3GUI_THEME * t3gui_get_default_theme(void)
{
    if(!t3gui_default_theme.state[0].bitmap[0])
    {
        if(!create_default_theme(&t3gui_default_theme))
        {
            return NULL;
        }
    }
    if(t3gui_default_theme.state[0].bitmap[0])
    {
        return &t3gui_default_theme;
    }
    return NULL;
}

static T3GUI_THEME * t3gui_create_theme(void)
{
    T3GUI_THEME * tp;
    T3GUI_THEME * dtp;

    tp = malloc(sizeof(T3GUI_THEME));
    if(tp)
    {
        dtp = t3gui_get_default_theme();
        if(dtp)
        {
            memcpy(tp, dtp, sizeof(T3GUI_THEME));
        }
    }
    return tp;
}

static ALLEGRO_COLOR get_color(const char * buf)
{
    char cbuf[3] = {0};
    int i;
    int ce[4] = {0, 0, 0, 255};
    int l = strlen(buf);

    for(i = 0; i < l / 2; i++)
    {
        cbuf[0] = buf[i * 2 + 0];
        cbuf[1] = buf[i * 2 + 1];
        ce[i] = strtol(cbuf, NULL, 16);
    }
    return al_map_rgba(ce[0], ce[1], ce[2], ce[3]);
}

static void t3gui_get_theme_state(ALLEGRO_CONFIG * cp, const char * section, T3GUI_THEME_STATE * sp)
{
    const char * val;
    const char * val2;
    char key_buf[64] = {0};
    int j;

    for(j = 0; j < T3GUI_THEME_MAX_BITMAPS; j++)
    {
        sprintf(key_buf, "bitmap %d", j);
        val = al_get_config_value(cp, section, key_buf);
        if(val)
        {
            t3gui_load_bitmap(&sp->bitmap[j], val);
        }
    }
    for(j = 0; j < T3GUI_THEME_MAX_COLORS; j++)
    {
        sprintf(key_buf, "color %d", j);
        val = al_get_config_value(cp, section, key_buf);
        if(val)
        {
            sp->color[j] = get_color(val);
        }
    }
    val = al_get_config_value(cp, section, "font");
    if(val)
    {
        val2 = al_get_config_value(cp, section, "font size");
        if(val2)
        {
            t3gui_load_font(&sp->font, val, atoi(val2));
        }
    }
}

T3GUI_THEME * t3gui_load_theme(const char * fn)
{
    ALLEGRO_CONFIG * cp;
    int i;
    char section_buf[64] = {0};
    T3GUI_THEME * tp;

    cp = al_load_config_file(fn);
    if(cp)
    {
        tp = t3gui_create_theme();
        if(tp)
        {
            /* Fill in all states with data from Default section. */
            for(i = 0; i < T3GUI_ELEMENT_STATES; i++)
            {
                t3gui_get_theme_state(cp, "Default", &tp->state[i]);
            }

            /* Load state-specific data. */
            for(i = 0; i < T3GUI_ELEMENT_STATES; i++)
            {
                sprintf(section_buf, "State %d", i);
                t3gui_get_theme_state(cp, section_buf, &tp->state[i]);
            }
            return tp;
        }
    }
    return NULL;
}

void t3gui_destroy_theme(T3GUI_THEME * tp)
{
    free(tp);
}
