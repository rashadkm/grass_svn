#include <grass/glocale.h>
#include "cairodriver.h"

#if CAIRO_HAS_FT_FONT
#include <cairo-ft.h>
#include <fontconfig/fontconfig.h>
#endif

#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif

static char *convert(const char *in)
{
    size_t ilen, olen;
    char *out;

    ilen = strlen(in);
    olen = 3 * ilen + 1;

    out = G_malloc(olen);

#ifdef HAVE_ICONV_H
    {
	char *p1 = (char *) in;
	char *p2 = out;
	size_t ret;
	iconv_t cd;

	if (!encoding)
	    encoding = G_store("US-ASCII");

	if ((cd = iconv_open("UTF-8", encoding)) < 0)
	    G_fatal_error(_("Unable to convert from <%s> to UTF-8"));

	ret = iconv(cd, &p1, &ilen, &p2, &olen);

	iconv_close(cd);

	*p2++ = '\0';

	if (ret > 0)
	    G_warning(_("Some characters could not be converted to UTF-8"));
    }
#else
    {
	const unsigned char *p1 = (const unsigned char *) in;
	unsigned char *p2 = (unsigned char *) out;
	int i, j;

	for (i = j = 0; i < len; i++) {
	    int c = p1[i];
	    if (c < 0x80)
		p2[j++] = c;
	    else {
		p2[j++] = 0xC0 + (c >> 6);
		p2[j++] = 0x80 + (c & 0x3F);
	    }
	}

	p2[j++] = '\0';
    }
#endif

    return out;
}

static void set_matrix(void)
{
    static cairo_matrix_t mat;

    if (matrix_valid)
	return;

    cairo_matrix_init_identity(&mat);
    cairo_matrix_scale(&mat, text_size_x * 25, text_size_y * 25);
    cairo_matrix_rotate(&mat, -text_rotation * M_PI / 180);

    cairo_set_font_matrix(cairo, &mat);

    matrix_valid = 1;
}

void Cairo_draw_text(const char *str)
{
    char *utf8 = convert(str);

    if (!utf8)
	return;

    set_matrix();

    cairo_move_to(cairo, cur_x, cur_y);
    cairo_show_text(cairo, utf8);

    G_free(utf8);

    modified = 1;
}

void Cairo_text_box(const char *str, double *t, double *b, double *l, double *r)
{
    char *utf8 = convert(str);
    cairo_text_extents_t ext;

    if (!utf8)
	return;

    set_matrix();

    cairo_text_extents(cairo, utf8, &ext);

    G_free(utf8);

    *l = cur_x + ext.x_bearing;
    *r = cur_x + ext.x_bearing + ext.width;
    *t = cur_x + ext.y_bearing;
    *b = cur_x + ext.y_bearing + ext.height;
}

static void set_font_toy(const char *name)
{
    char *font = G_store(name);
    cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_NORMAL;
    cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;

    for (;;) {
	char *p = strrchr(font, '-');
	if (!p)
	    break;

	if (G_strcasecmp(p, "-bold") == 0)
	    weight = CAIRO_FONT_WEIGHT_BOLD;
	else if (strcasecmp(p, "-italic") == 0)
	    slant = CAIRO_FONT_SLANT_ITALIC;
	else if (G_strcasecmp(p, "-oblique") == 0)
	    slant = CAIRO_FONT_SLANT_OBLIQUE;
	else
	    break;

	*p = '\0';
    }

    cairo_select_font_face(cairo, font, slant, weight);

    G_free(font);
}

#if CAIRO_HAS_FT_FONT

static void set_font_fc(const char *name)
{
    static cairo_font_face_t *face;
    static int initialized;
    FcPattern *pattern;
    FcResult result;

    if (!initialized) {
	FcInit();
	initialized = 1;
    }

    if (face) {
	cairo_font_face_destroy(face);
	face = NULL;
    }

    pattern = FcNameParse(name);
    FcDefaultSubstitute(pattern);
    FcConfigSubstitute(FcConfigGetCurrent(), pattern, FcMatchPattern);
    pattern = FcFontMatch(FcConfigGetCurrent(), pattern, &result);
    face = cairo_ft_font_face_create_for_pattern(pattern);
    cairo_set_font_face(cairo, face);
}

#endif

static const char *toy_fonts[12] = {
    "sans",
    "sans-italic",
    "sans-bold",
    "sans-bold-italic",
    "serif",
    "serif-italic",
    "serif-bold",
    "serif-bold-italic",
    "mono",
    "mono-italic",
    "mono-bold",
    "mono-bold-italic",
};
static const int num_toy_fonts = 12;

static int is_toy_font(const char *name)
{
    int i;

    for (i = 0; i < num_toy_fonts; i++)
	if (G_strcasecmp(name, toy_fonts[i]) == 0)
	    return 1;

    return 0;
}

void Cairo_set_font(const char *name)
{
#if CAIRO_HAS_FT_FONT
    if (is_toy_font(name))
	set_font_toy(name);
    else
	set_font_fc(name);
#else
    set_font_toy(name);
#endif
}

void Cairo_set_encoding(const char *enc)
{
    if (encoding)
	G_free(encoding);

    encoding = G_store(enc);
}

void Cairo_text_size(double width, double height)
{
    text_size_x = width;
    text_size_y = height;
    matrix_valid = 0;
}

void Cairo_text_rotation(double angle)
{
    text_rotation = angle;
    matrix_valid = 0;
}

static void font_list_toy(char ***list, int *count, int verbose)
{
    char **fonts = *list;
    int num_fonts = *count;
    int i;

    fonts = G_realloc(fonts, (num_fonts + num_toy_fonts) * sizeof(char *));

    for (i = 0; i < num_toy_fonts; i++) {
	char buf[256];
	sprintf(buf, "%s%s",
		toy_fonts[i],
		verbose ? "||1||0|utf-8|" : "");
	fonts[num_fonts++] = G_store(buf);
    }

    *list = fonts;
    *count = num_fonts;
}

void Cairo_font_list(char ***list, int *count)
{
    font_list(list, count, 0);
    font_list_toy(list, count, 0);
}

void Cairo_font_info(char ***list, int *count)
{
    font_list(list, count, 1);
    font_list_toy(list, count, 1);
}

