/* Line thinning program */
/*   Input/output and file support functions */

/* Mike Baba */
/* DBA Systems */
/* Fairfax, Va */
/* Jan 1990 */

/* Jean Ezell */
/* US Army Corps of Engineers */
/* Construction Engineering Research Laboratory */
/* Modelling and Simulation Team */
/* Champaign, IL  61820 */
/* January - February 1988 */

/* Entry points: */
/*   get_a_row     get row from temporary work file */
/*   open_file     open input raster map and read it into work file */
/*   close_file    copy work file into new raster map */
/*   map_size      get size of map and its pad */

/* Global variables: */
/*   row_io        place to store pointer to row manager stuff */
/*   n_rows        number of rows in the work file (includes pads) */
/*   n_cols        number of columns in the work file (includes pads) */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <grass/config.h>
#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/glocale.h>
#include <grass/rowio.h>

#define PAD 2
#define MAX_ROW 7

		       /*extern int errno; *//* included #include <errno.h> instead 1/2000 */
extern char *error_prefix;
static int n_rows, n_cols;
static int work_file;
static char *work_file_name;
static ROWIO row_io;


/* function prototypes */
static int write_row(int file, const void *buf, int row, int buf_len);
static int read_row(int file, void *buf, int row, int buf_len);


CELL *get_a_row(int row)
{
    if (row < 0 || row >= n_rows)
	return (NULL);
    return ((CELL *) Rowio_get(&row_io, row));
}

int put_a_row(int row, CELL * buf)
{
    /* rowio.h defines this withe 2nd argument as char * */
    Rowio_put(&row_io, (char *)buf, row);

    return 0;
}


static int read_row(int file, void *buf, int row, int buf_len)
{
    lseek(file, ((off_t) row) * buf_len, 0);
    return (read(file, buf, buf_len) == buf_len);
}

static int write_row(int file, const void *buf, int row, int buf_len)
{
    lseek(file, ((off_t) row) * buf_len, 0);
    return (write(file, buf, buf_len) == buf_len);
}

int open_file(char *name)
{
    int cell_file, buf_len;
    int i, row;
    CELL *buf;

    /* open raster map */
    cell_file = Rast_open_old(name, "");
    
    if (Rast_get_map_type(cell_file) != CELL_TYPE) {
	Rast_close(cell_file);
	G_fatal_error(_("Input raster must be of type CELL."));
    }

    n_rows = Rast_window_rows();
    n_cols = Rast_window_cols();
    /* GTC First is the file name, second and third - number of rows and cols */
    G_message(_("File %s -- %s X %s"), name, 
        _n("%d row", "%d rows", n_rows), 
        _n("%d column", "%d columns", n_cols));
    n_cols += (PAD << 1);

    /* copy raster map into our read/write file */
    work_file_name = G_tempfile();

    /* create the file and then open it for read and write */
    close(creat(work_file_name, 0666));
    if ((work_file = open(work_file_name, 2)) < 0) {
	unlink(work_file_name);
	G_fatal_error(_("%s: Unable to create temporary file <%s> -- errno = %d"),
		      error_prefix, work_file_name, errno);
    }
    buf_len = n_cols * sizeof(CELL);
    buf = (CELL *) G_malloc(buf_len);
    Rast_set_c_null_value(buf, n_cols);
    for (i = 0; i < PAD; i++) {
	if (write(work_file, buf, buf_len) != buf_len) {
	    unlink(work_file_name);
	    G_fatal_error(_("%s: Error writing temporary file"),
			  error_prefix);
	}
    }
    for (row = 0; row < n_rows; row++) {
	Rast_get_c_row(cell_file, buf + PAD, row);
	if (write(work_file, buf, buf_len) != buf_len) {
	    unlink(work_file_name);
	    G_fatal_error(_("%s: Error writing temporary file"),
			  error_prefix);
	}
    }

    Rast_set_c_null_value(buf, n_cols);

    for (i = 0; i < PAD; i++) {
	if (write(work_file, buf, buf_len) != buf_len) {
	    unlink(work_file_name);
	    G_fatal_error(_("%s: Error writing temporary file"),
			  error_prefix);
	}
    }
    n_rows += (PAD << 1);
    G_free(buf);
    Rast_close(cell_file);
    Rowio_setup(&row_io, work_file, MAX_ROW, n_cols * sizeof(CELL), read_row,
		write_row);

    return 0;
}

int close_file(char *name)
{
    int cell_file, row, k;
    int row_count, col_count;
    CELL *buf;

    cell_file = Rast_open_c_new(name);

    row_count = n_rows - (PAD << 1);
    col_count = n_cols - (PAD << 1);
    /* GTC %s will be replaced with number of rows and columns */
    G_message(_("Output file %s X %s"), 
        _n("%d row", "%d rows", row_count), 
        _n("%d column", "%d columns", col_count));
    
    /* GTC %s will be replaced with number of rows and columns */
    G_message(_("Window %s X %s"), 
        _n("%d row", "%d rows", Rast_window_rows()), 
        _n("%d column", "%d columns", Rast_window_cols()));

    for (row = 0, k = PAD; row < row_count; row++, k++) {
	buf = get_a_row(k);
	Rast_put_row(cell_file, buf + PAD, CELL_TYPE);
    }
    Rast_close(cell_file);
    Rowio_flush(&row_io);
    close(Rowio_fileno(&row_io));
    Rowio_release(&row_io);
    unlink(work_file_name);

    return 0;
}

int map_size(int *r, int *c, int *p)
{
    *r = n_rows;
    *c = n_cols;
    *p = PAD;

    return 0;
}
