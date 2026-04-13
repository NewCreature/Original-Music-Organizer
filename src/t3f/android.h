#ifndef T3F_ANDROID_H
#define T3F_ANDROID_H

void t3f_android_support_helper(void);
void t3f_open_edit_box(const char * title, char * text, int text_size, const char * flags, void(*callback)(void * data), void * data);
void t3f_show_soft_keyboard(bool toogle);
void _t3f_reset_android_bg_color(void);
void t3f_open_url(const char *url);
int t3f_run_url(const char * url, const char ** post_data, const char * out_path, char ** out_data);

#endif
