#include "globals.h"
#include "display.h"

static int use = 1;
static int set_blue(void);
static int set_gray(void);
static int set_green(void);
static int set_red(void);
static int set_white(void);
static int set_yellow(void);
static int set_cur_clr( int);
static int setmap_blue(void);
static int setmap_gray(void);
static int setmap_green(void);
static int setmap_red(void);
static int setmap_white(void);
static int setmap_yellow(void);
static int done(void);

int set_colors (struct Colors *colors)
{
    D_set_colors (colors);

    return 0;
}

int set_menu_colors (struct Colors *colors)
{

/* SCREEN OUTLINE and CURSOR */
    G_add_color_rule((CELL)241,180, 180, 180, (CELL)241,180, 180, 180, colors);
/* RED */
    G_add_color_rule((CELL)242,200,  90,  90, (CELL)242,200,  90,  90, colors);
/* ORANGE */
    G_add_color_rule((CELL)243,150, 100,  50, (CELL)243,150, 100,  50, colors);
/* YELLOW */
    G_add_color_rule((CELL)244,200, 200,  10, (CELL)244,200, 200,  10, colors);
/* GREEN */
    G_add_color_rule((CELL)245, 90, 200,  90, (CELL)245, 90, 200,  90, colors);
/* BLUE */
    G_add_color_rule((CELL)246, 90,  90, 200, (CELL)246, 90,  90, 200, colors);
/* INDIGO */
    G_add_color_rule((CELL)247,100, 100,  10, (CELL)247,100, 100,  10, colors);
/* VIOLET */
    G_add_color_rule((CELL)248,150, 150,  10, (CELL)248,150, 150,  10, colors);
/* WHITE */
    G_add_color_rule((CELL)249,250, 250, 250, (CELL)249,250, 250, 250, colors);
/* BLACK */
    G_add_color_rule((CELL)250,  0,   0,   0, (CELL)250,  0,   0,   0, colors);
/* GRAY */
    G_add_color_rule((CELL)251,180, 180, 180, (CELL)251,180, 180, 180, colors);
/* BROWN */
    G_add_color_rule((CELL)252,100, 100,  30, (CELL)252,100, 100,  30, colors);
/* MAGENTA */
    G_add_color_rule((CELL)253,150,  90, 150, (CELL)253,150,  90, 150, colors);
/* AQUA */
    G_add_color_rule((CELL)254, 50, 120, 120, (CELL)254, 50, 120, 120, colors); 
/*      */
    G_add_color_rule((CELL)255,250,   0,   0, (CELL)255,250,   0,   0, colors); 

    set_colors (colors);

    return 0;
}

int cursor_color (void)
{
    static Objects objects[] =
    {
	MENU("DONE", done, &use),
 	INFO("Pick a Color ->", &use),
	MENU("BLUE", set_blue, &use),
	MENU("GRAY", set_gray, &use),
	MENU("GREEN", set_green, &use),
	MENU("RED", set_red, &use),
	MENU("WHITE", set_white, &use),
	MENU("YELLOW", set_yellow, &use),
	{0} 
    };

    while( (Input_pointer(objects) != -1) );
    return 0;  /* return but don't quit */
}

static int set_blue (void)
{
	set_cur_clr(BLUE);
	return 0;
}

static int set_gray (void)
{
	set_cur_clr(GREY);
	return 0;
}

static int set_green (void)
{
	set_cur_clr(GREEN);
	return 0;
}

static int set_red (void)
{
	set_cur_clr(RED);
	return 0;
}

static int set_white (void)
{
	set_cur_clr(WHITE);
	return 0;
}

static int set_yellow (void)
{
	set_cur_clr(YELLOW);
	return 0;
}

static int set_cur_clr(int curs_color)
{
    struct Colors *colors;

    colors = &VIEW_MAP1->cell.colors;
    
    switch(curs_color)
	{
	case 5:    /* BLUE */

    G_add_color_rule((CELL)241, 90,  90, 200, (CELL)241, 90,  90, 200, colors);
	break;

	case 10:   /* GRAY */

    G_add_color_rule((CELL)241,180, 180, 180, (CELL)241,180, 180, 180, colors);
	break;

	case 4:    /* GREEN */

    G_add_color_rule((CELL)241, 90, 200,  90, (CELL)241, 90, 200,  90, colors);
	break;

	case 1:    /* RED */

    G_add_color_rule((CELL)241,200,  90,  90, (CELL)241,200,  90,  90, colors);
	break;

	case 8:    /* WHITE */

    G_add_color_rule((CELL)241,250, 250, 250, (CELL)241,250, 250, 250, colors);
	break;

	case 3:   /* YELLOW */

    G_add_color_rule((CELL)241,200, 200,  10, (CELL)241,200, 200,  10, colors);
	break;
    }

    set_colors (colors);
    return 0;
}
 
int get_vector_color (void)
{
    int x, y;
	
    static Objects objects[] =
    {
 	INFO("Pick color for vectors ->", &use),
	MENU("BLUE", setmap_blue, &use),
	MENU("GRAY", setmap_gray, &use),
	MENU("GREEN", setmap_green, &use),
	MENU("RED", setmap_red, &use),
	MENU("WHITE", setmap_white, &use),
	MENU("YELLOW", setmap_yellow, &use),
	{0} 
    };

    x = (SCREEN_LEFT + SCREEN_RIGHT) / 2;
    y = SCREEN_BOTTOM;
    Set_mouse_xy( x,y );

    Input_pointer(objects);
    return 0;  /* return but don't quit */
}

static int setmap_blue (void)
{
	line_color = BLUE;
	return 0;
}

static int setmap_gray (void)
{
	line_color = GREY;
	return 0;
}

static int setmap_green (void)
{
	return line_color = GREEN;
}

static int setmap_red (void)
{
	return line_color = RED;
}

static int setmap_white (void)
{
	return line_color = WHITE;
}

static int setmap_yellow (void)
{
	return line_color = YELLOW;
}

static int done (void)
{
	return -1;
} 
