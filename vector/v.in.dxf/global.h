/* modified 1998-OCT-06 Benjamin Horner-Johnson - 80->256 char dxf_line */
/* 1/28/98 change from Jacques Bouchard <bouchard@onera.fr> */

#include <stdio.h>
#include <grass/gis.h>
#include <grass/Vect.h>

/* TODO */
/* #define LABEL */

#define MAX_FILES 15

#define DXF_ASCII 0
#define DXF_LABEL 1
#define DXF_LABEL_LINE 2

struct dxf_dig
{
    char *name;
    int type;
    FILE *fd;
    struct Map_info *Map;
    int status;
};

#define ARR_INCR	256

#ifdef MAIN
#define GLOBAL
char *dxf_file = NULL;
int num_open_layers = 0;
int num_closed_layers = 0;
int from_table = 0;
#else
#define GLOBAL extern
GLOBAL char *dxf_file;
GLOBAL int num_open_layers;
GLOBAL int num_closed_layers;
GLOBAL int from_table;
#endif

GLOBAL struct line_pnts *Points;
GLOBAL unsigned long file_size;
GLOBAL int percent;
GLOBAL char dig_path[240];
GLOBAL char base_name[100];	/* dpg */
GLOBAL int ARR_MAX;
GLOBAL double n, s, e, w;
GLOBAL long n_off, s_off, e_off, w_off;
GLOBAL char zzero[8];
GLOBAL char eeight[8];
GLOBAL char tten[8];
GLOBAL char ttwenty[8];
GLOBAL char eelev[8];
GLOBAL char ttwentyone[8];
GLOBAL char entitie[12];
GLOBAL char header[12];
GLOBAL char extmin[12];
GLOBAL char extmax[12];
GLOBAL char polyline[12];
GLOBAL char circle[12];
GLOBAL char text[12];		/* dpg */
GLOBAL char line[8];
GLOBAL char point[8];
GLOBAL char arc[8];
GLOBAL char vertex[8];
GLOBAL char seqend[8];
GLOBAL char dxf_line[256];
GLOBAL struct dxf_dig layers[MAX_FILES];
GLOBAL struct dxf_dig *closed_layers;
GLOBAL double XMAX, XMIN, YMAX, YMIN, ZMAX, ZMIN;
GLOBAL int BOUNDARIES;
GLOBAL struct Map_info head;
GLOBAL double *xinfo, *yinfo, *zinfo;
#ifdef LABEL
GLOBAL int txtbox;
#endif


/* debug.c */
int debuginit(void);
int debugf(char *, ...);


/* open_layer.c */
int open_layer(int, int);
/* reopen_layer.c */
int reopen_layer(int, int, int);
/* which_layer.c */
struct dxf_dig *which_layer(char *, int);
/* close_layer.c */
int close_layer(int);


/* layer_map.c */
int add_line_layer(char *);
int add_att_layer(char *);
char *remap(char *, int);


/* read_dxf.c */
int dxf_readcode(FILE *);
char *dxf_fgets(char *, int, FILE *);
int dxf_find_header(FILE *);
int dxf_find_entities(FILE *);
int big_percent(unsigned long, unsigned long, int);


/* init_chars.c */
int init_chars(void);
/* make_header.c */
int make_header(struct dxf_dig *);


/* create_layers.c */
int create_layers(FILE *);
int check_ext(double, double);
/* add_arc.c */
int add_arc(FILE *);
/* add_circle.c */
int add_circle(FILE *);
/* add_labelbox.c */
int add_labelbox(FILE *);
/* add_line.c */
int add_line(FILE *);
/* add_point.c */
int add_point(FILE *);
/* add_polyline.c */
int add_polyline(FILE *);
/* make_arc.c */
int make_arc(int, double, double, double, double, double, double, int);
/* write_polylines.c */
int write_polylines(struct dxf_dig *, int);
