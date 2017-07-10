#ifndef T3F_ANDROID_H
#define T3F_ANDROID_H

void t3f_android_support_helper(void);
void t3f_open_edit_box(const char * title, char * text, int text_size, const char * flags, void(*callback)(void * data), void * data);
void t3f_show_soft_keyboard(bool toogle);
void t3f_open_url(const char *url);

#endif
