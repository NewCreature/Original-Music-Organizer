/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Unicode support routines.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#ifndef ALLEGRO_UNICODE__H
#define ALLEGRO_UNICODE__H

#ifdef __cplusplus
   extern "C" {
#endif

#define AL_ID(a,b,c,d)     (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))

#define U_ASCII         AL_ID('A','S','C','8')
#define U_ASCII_CP      AL_ID('A','S','C','P')
#define U_UNICODE       AL_ID('U','N','I','C')
#define U_UTF8          AL_ID('U','T','F','8')
#define U_CURRENT       AL_ID('c','u','r','.')

#ifndef AL_FUNC
   #define AL_FUNC(type, name, args)               type name args
#endif

#ifndef AL_METHOD
   #define AL_METHOD(type, name, args)             type (*name) args
#endif

#ifndef AL_FUNCPTR
   #define AL_FUNCPTR(type, name, args)            extern type (*name) args
#endif

#ifndef AL_PRINTFUNC
   #define AL_PRINTFUNC(type, name, args, a, b)    AL_FUNC(type, name, args)
#endif

#ifndef AL_ARRAY
   #define AL_ARRAY(type, name)                    extern type name[]
#endif

AL_FUNC(void, set_uformat, (int type));
AL_FUNC(int, get_uformat, (void));
AL_FUNC(void, register_uformat, (int type, AL_METHOD(int, u_getc, (const char *s)), AL_METHOD(int, u_getx, (char **s)), AL_METHOD(int, u_setc, (char *s, int c)), AL_METHOD(int, u_width, (const char *s)), AL_METHOD(int, u_cwidth, (int c)), AL_METHOD(int, u_isok, (int c)), int u_width_max));
AL_FUNC(void, set_ucodepage, (const unsigned short *table, const unsigned short *extras));

AL_FUNC(int, need_uconvert, (const char *s, int type, int newtype));
AL_FUNC(int, uconvert_size, (const char *s, int type, int newtype));
AL_FUNC(void, do_uconvert, (const char *s, int type, char *buf, int newtype, int size));
AL_FUNC(char *, uconvert, (const char *s, int type, char *buf, int newtype, int size));
AL_FUNC(int, uwidth_max, (int type));

#define uconvert_ascii(s, buf)      uconvert(s, U_ASCII, buf, U_CURRENT, sizeof(buf))
#define uconvert_toascii(s, buf)    uconvert(s, U_CURRENT, buf, U_ASCII, sizeof(buf))

AL_FUNCPTR(int, ugetc, (const char *s));
AL_FUNCPTR(int, ugetx, (char **s));
AL_FUNCPTR(int, ugetxc, (const char **s));
AL_FUNCPTR(int, usetc, (char *s, int c));
AL_FUNCPTR(int, uwidth, (const char *s));
AL_FUNCPTR(int, ucwidth, (int c));
AL_FUNCPTR(int, uisok, (int c));
AL_FUNC(int, uoffset, (const char *s, int idx));
AL_FUNC(int, ugetat, (const char *s, int idx));
AL_FUNC(int, usetat, (char *s, int idx, int c));
AL_FUNC(int, uinsert, (char *s, int idx, int c));
AL_FUNC(int, uremove, (char *s, int idx));
AL_FUNC(int, utolower, (int c));
AL_FUNC(int, utoupper, (int c));
AL_FUNC(int, uisspace, (int c));
AL_FUNC(int, uisdigit, (int c));
AL_FUNC(int, ustrsize, (const char *s));
AL_FUNC(int, ustrsizez, (const char *s));
AL_FUNC(char *, _ustrdup, (const char *src, AL_METHOD(void *, malloc_func, (size_t))));
AL_FUNC(char *, ustrzcpy, (char *dest, int size, const char *src));
AL_FUNC(char *, ustrzcat, (char *dest, int size, const char *src));
AL_FUNC(int, ustrlen, (const char *s));
AL_FUNC(int, ustrcmp, (const char *s1, const char *s2));
AL_FUNC(char *, ustrzncpy, (char *dest, int size, const char *src, int n));
AL_FUNC(char *, ustrzncat, (char *dest, int size, const char *src, int n));
AL_FUNC(int, ustrncmp, (const char *s1, const char *s2, int n));
AL_FUNC(int, ustricmp, (const char *s1, const char *s2));
AL_FUNC(int, ustrnicmp, (const char *s1, const char *s2, int n));
AL_FUNC(char *, ustrlwr, (char *s));
AL_FUNC(char *, ustrupr, (char *s));
AL_FUNC(char *, ustrchr, (const char *s, int c));
AL_FUNC(char *, ustrrchr, (const char *s, int c));
AL_FUNC(char *, ustrstr, (const char *s1, const char *s2));
AL_FUNC(char *, ustrpbrk, (const char *s, const char *set));
AL_FUNC(char *, ustrtok, (char *s, const char *set));
AL_FUNC(char *, ustrtok_r, (char *s, const char *set, char **last));
AL_FUNC(double, uatof, (const char *s));
AL_FUNC(long, ustrtol, (const char *s, char **endp, int base));
AL_FUNC(double, ustrtod, (const char *s, char **endp));
AL_FUNC(const char *, ustrerror, (int err));
AL_PRINTFUNC(int, uszprintf, (char *buf, int size, const char *format, ...), 3, 4);
AL_FUNC(int, uvszprintf, (char *buf, int size, const char *format, va_list args));
AL_PRINTFUNC(int, usprintf, (char *buf, const char *format, ...), 2, 3);

#ifndef ustrdup
   #ifdef FORTIFY
      #define ustrdup(src)            _ustrdup(src, Fortify_malloc)
   #else
      #define ustrdup(src)            _ustrdup(src, malloc)
   #endif
#endif

#define ustrcpy(dest, src)            ustrzcpy(dest, INT_MAX, src)
#define ustrcat(dest, src)            ustrzcat(dest, INT_MAX, src)
#define ustrncpy(dest, src, n)        ustrzncpy(dest, INT_MAX, src, n)
#define ustrncat(dest, src, n)        ustrzncat(dest, INT_MAX, src, n)
#define uvsprintf(buf, format, args)  uvszprintf(buf, INT_MAX, format, args)

#ifdef __cplusplus
   }
#endif

#endif          /* ifndef ALLEGRO_UNICODE__H */


