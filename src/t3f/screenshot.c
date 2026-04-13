#include "t3f.h"

static ALLEGRO_THREAD * _t3f_screenshot_thread = NULL;
static char * _t3f_screenshot_base_filename = NULL;
static float _t3f_screenshot_interval = 0.0;

bool t3f_capture_screenshot(const char * filename)
{
    ALLEGRO_BITMAP * bitmap = al_get_backbuffer(t3f_display);

    return al_save_bitmap(filename, bitmap);
}

static void * _t3f_capture_screenshots_thread(ALLEGRO_THREAD * thread, void * data)
{
    ALLEGRO_TIMER * timer = NULL;
    ALLEGRO_EVENT_QUEUE * event_queue = NULL;
    ALLEGRO_EVENT event;
    ALLEGRO_BITMAP * bitmap;
    int count = 0;
    char buf[1024];

    event_queue = al_create_event_queue();
    if(!event_queue)
    {
        goto fail;
    }
    timer = al_create_timer(_t3f_screenshot_interval);
    if(!timer)
    {
        goto fail;
    }
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_start_timer(timer);
    while(!al_get_thread_should_stop(thread))
    {
        al_wait_for_event(event_queue, &event);
        bitmap = al_get_backbuffer(t3f_display);
        sprintf(buf, "%s%04d.png", _t3f_screenshot_base_filename, count);
        al_save_bitmap(buf, bitmap);
        count++;
    }
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
    free(_t3f_screenshot_base_filename);

    return NULL;

    fail:
    {
        if(event_queue)
        {
            al_destroy_event_queue(event_queue);
        }
        if(timer)
        {
            al_destroy_timer(timer);
        }
        return NULL;
    }
}

bool t3f_capture_screenshots(const char * base_filename, float interval)
{
    if(_t3f_screenshot_thread)
    {
        goto fail;
    }
    _t3f_screenshot_thread = al_create_thread(_t3f_capture_screenshots_thread, NULL);
    if(!_t3f_screenshot_thread)
    {
        goto fail;
    }
    _t3f_screenshot_base_filename = strdup(base_filename);
    if(!_t3f_screenshot_base_filename)
    {
        goto fail;
    }
    _t3f_screenshot_interval = interval;
    al_start_thread(_t3f_screenshot_thread);

    return true;

    fail:
    {
        if(_t3f_screenshot_base_filename)
        {
            free(_t3f_screenshot_base_filename);
        }
        return false;
    }
}

void t3f_stop_capturing_screenshots(void)
{
    if(_t3f_screenshot_thread)
    {
        al_destroy_thread(_t3f_screenshot_thread);
        _t3f_screenshot_thread = NULL;
    }
}
