#ifndef _T3F_SCREENSHOT_H
#define _T3F_SCREENSHOT_H

bool t3f_capture_screenshot(const char * filename);

bool t3f_capture_screenshots(const char * base_filename, float interval);
void t3f_stop_capturing_screenshots(void);

#endif
