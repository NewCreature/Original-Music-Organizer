#ifndef MD5_H
#define MD5_H

void md5(uint8_t *initial_msg, size_t initial_len);
void md5_file(const char * fn, uint32_t * h);

#endif
