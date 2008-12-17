
#define DEF_WIDTH  640
#define DEF_HEIGHT 480

#include "driver.h"

extern const struct driver *driver;

/* Utility Functions */

/* Font.c */
int font_is_freetype(void);
void font_list(char ***, int *, int);

/* Text2.c */
void get_text_ext(const char *, double *, double *, double *, double *);
void soft_text(const char *);

/* Text3.c */
void soft_text_freetype(const char *);
void get_text_ext_freetype(const char *, double *, double *, double *, double *);

/* font2.c */
int font_init(const char *);
int get_char_vects(unsigned char, int *, unsigned char **, unsigned char **);

/* font_freetype.c */
int font_init_freetype(const char *, int);
const char *font_get_freetype_name(void);
int font_get_index(void);

/* parse_ftcap.c */
extern int font_exists(const char *);
extern struct GFONT_CAP *parse_freetypecap(void);
extern void free_freetypecap(struct GFONT_CAP *);
extern void free_font_list(char **, int);

