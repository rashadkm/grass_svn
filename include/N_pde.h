
/*****************************************************************************
*
* MODULE:       Grass PDE Numerical Library
* AUTHOR(S):    Soeren Gebbert, Berlin (GER) Dec 2006
* 		soerengebbert <at> gmx <dot> de
*               
* PURPOSE:      This file contains definitions of variables and data types
*
* COPYRIGHT:    (C) 2000 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*               License (>=v2). Read the file COPYING that comes with GRASS
*               for details.
*
*****************************************************************************/

#include <grass/gis.h>
#include <grass/G3d.h>

#ifndef _N_PDE_H_
#define _N_PDE_H_

#define N_NORMAL_LES 0
#define N_SPARSE_LES 1

#define N_CELL_INACTIVE 0
#define N_CELL_ACTIVE 1
#define N_CELL_DIRICHLET 2

#define N_5_POINT_STAR 0
#define N_7_POINT_STAR 1
#define N_9_POINT_STAR 2
#define N_27_POINT_STAR 3

#define N_MAXIMUM_NORM 0
#define N_EUKLID_NORM 1

#define N_ARRAY_SUM 0 /* summ two arrays */
#define N_ARRAY_DIF 1 /* calc the difference between two arrays */
#define N_ARRAY_MUL 2 /* multiply two arrays */
#define N_ARRAY_DIV 3 /* array division, if div with 0 the NULL value is set */

/* *************************************************************** */
/* *************** LINEARE EQUATION SYSTEM PART ****************** */
/* *************************************************************** */

/*!
 * \brief The row vector of the sparse matrix
 * */
typedef struct
{
  int cols;			/*Number of entries */
  double *values;		/*The non null values of the row */
  int *index;			/*the index number */
} N_spvector;


/*!
 * \brief The linear equation system (les) structure 
 *
 * This structure manages the Ax = b system.
 * It manages regular quadratic matrices or
 * sparse matrices. The vector b and x are normal one dimensional 
 * memory structures of type double. Also the number of rows
 * and the matrix type are stored in this structure.
 * */
typedef struct
{
  double *x;			/*the value vector */
  double *b;			/*the right side of Ax = b */
  double **A;			/*the normal quadratic matrix */
  N_spvector **Asp;		/*the sparse matrix */
  int rows;			/*number of rows */
  int type;			/*the type of the les, normal == 0, sparse == 1 */
} N_les;

extern N_spvector *N_alloc_spvector (int cols);
extern N_les *N_alloc_les (int rows, int type);
extern void N_print_les (N_les * les);
extern int N_add_spvector_to_les (N_les * les, N_spvector * vector, int row);
extern void N_free_spvector (N_spvector * vector);
extern void N_free_les (N_les * les);

/* *************************************************************** */
/* *************** GEOMETRY INFORMATION ************************** */
/* *************************************************************** */

/*!
 * \brief Geometric informations about the structured grid
 * */
typedef struct
{

  double dx;
  double dy;
  double dz;

  double Ax;
  double Ay;
  double Az;

  int depths;
  int rows;
  int cols;

} N_geom_data;

extern N_geom_data *N_alloc_geom_data ();


/* *************************************************************** */
/* *************** LINEARE EQUATION SOLVER PART ****************** */
/* *************************************************************** */
extern int N_solver_gauss (N_les * les);
extern int N_solver_lu (N_les * les);
extern int N_solver_cg (N_les * les, int maxit, double error);
extern int N_solver_bicgstab (N_les * les, int maxit, double error);

/* *************************************************************** */
/* *************** READING RASTER AND VOLUME DATA **************** */
/* *************************************************************** */

/*
 * \brief The 2d data array keeping the data for matrix assembling 
 * */
typedef struct
{
  int type;			/* which raster type CELL_TYPE, FCELL_TYPE, DCELL_TYPE */
  int rows, cols;
  int rows_intern, cols_intern;
  int offset;			/*number of cols/rows offset at each boundary */
  CELL *cell_array;		/*The data is stored in an one dimensional array internally */
  FCELL *fcell_array;		/*The data is stored in an one dimensional array internally */
  DCELL *dcell_array;		/*The data is stored in an one dimensional array internally */
} N_array_2d;

extern N_array_2d *N_alloc_array_2d (int cols, int rows, int offset, int type);
extern void N_free_array_2d (N_array_2d * data_array);
extern int N_get_array_2d_type (N_array_2d * array2d);
extern inline void N_get_array_2d_value (N_array_2d * array2d, int col, int row, void *value);
extern inline CELL N_get_array_2d_value_cell (N_array_2d * array2d, int col, int row);
extern inline FCELL N_get_array_2d_value_fcell (N_array_2d * array2d, int col, int row);
extern inline DCELL N_get_array_2d_value_dcell (N_array_2d * array2d, int col, int row);
extern inline void N_put_array_2d_value (N_array_2d * array2d, int col, int row, char *value);
extern inline void N_put_array_2d_value_cell (N_array_2d * array2d, int col, int row, CELL value);
extern inline void N_put_array_2d_value_fcell (N_array_2d * array2d, int col, int row, FCELL value);
extern inline void N_put_array_2d_value_dcell (N_array_2d * array2d, int col, int row, DCELL value);
extern inline int N_is_array_2d_value_null (N_array_2d * array2d, int col, int row);
extern inline void N_put_array_2d_value_null (N_array_2d * array2d, int col, int row);
extern void N_print_array_2d (N_array_2d * data);
extern void N_copy_array_2d (N_array_2d * source, N_array_2d * target);
extern double N_norm_array_2d (N_array_2d * array1, N_array_2d * array2, int type);
extern N_array_2d * N_math_array_2d (N_array_2d * array1, N_array_2d * array2, N_array_2d * result, int type);
extern int N_convert_array_2d_null_to_zero (N_array_2d * a);
extern N_array_2d * N_read_rast_to_array_2d (char *name, N_array_2d * array);
extern void N_write_array_2d_to_rast (N_array_2d * array, char *name);
/*
 * \brief The 3d data array keeping the data for matrix assembling 
 * */
typedef struct
{
  int type;			/* which raster type G3D_FLOAT, G3D_DOUBLE */
  int rows, cols, depths;
  int rows_intern, cols_intern, depths_intern;
  int offset;			/*number of cols/rows/depths offset at each boundary */
  float *float_array;		/*The data is stored in an one dimensional array internally */
  double *double_array;		/*The data is stored in an one dimensional array internally */
} N_array_3d;

extern N_array_3d *N_alloc_array_3d (int cols, int rows, int depths, int offset, int type);
extern void N_free_array_3d (N_array_3d * data_array);
extern int N_get_array_3d_type (N_array_3d * array3d);
extern inline void N_get_array_3d_value (N_array_3d * array3d, int col, int row, int depth, void *value);
extern inline float N_get_array_3d_value_float (N_array_3d * array3d, int col, int row, int depth);
extern inline double N_get_array_3d_value_double (N_array_3d * array3d, int col, int row, int depth);
extern inline void N_put_array_3d_value (N_array_3d * array3d, int col, int row, int depth, char *value);
extern inline void N_put_array_3d_value_float (N_array_3d * array3d, int col, int row, int depth, float value);
extern inline void N_put_array_3d_value_double (N_array_3d * array3d, int col, int row, int depth, double value);
extern inline int N_is_array_3d_value_null (N_array_3d * array3d, int col, int row, int depth);
extern inline void N_put_array_3d_value_null (N_array_3d * array3d, int col, int row, int depth);
extern void N_print_array_3d (N_array_3d * data);
extern void N_copy_array_3d (N_array_3d * source, N_array_3d * target);
extern double N_norm_array_3d (N_array_3d * array1, N_array_3d * array2, int type);
extern N_array_3d * N_math_array_3d (N_array_3d * array1, N_array_3d * array2, N_array_3d * result, int type);
extern int N_convert_array_3d_null_to_zero (N_array_3d * a);
extern N_array_3d * N_read_rast3d_to_array_3d (char *name, N_array_3d * array, int mask);
extern void N_write_array_3d_to_rast3d (N_array_3d * array, char *name, int mask);

/* *************************************************************** */
/* *************** MATRIX ASSEMBLING METHODS ********************* */
/* *************************************************************** */
/*!
 * \brief Matrix entries for a mass balance 5/7/9 star system
 *
 * Matrix entries for the mass balance of a 5 star system
 *
 * The entries are center, east, west, north, south and the 
 * right side vector b of Ax = b. This system is typically used in 2d.

 \verbatim
     N
     |
 W-- C --E
     |
     S
 \endverbatim

 * Matrix entries for the mass balance of a 7 star system
 *
 * The entries are center, east, west, north, south, top, bottom and the 
 * right side vector b of Ax = b. This system is typically used in 3d.
 
 \verbatim
      T N
      |/
  W-- C --E
     /|
    S B
 \endverbatim
 
 * Matrix entries for the mass balance of a 9 star system
 *
 * The entries are center, east, west, north, south, north-east, south-east,
 * north-wast, south-west and the 
 * right side vector b of Ax = b. This system is typically used in 2d.
 
 \verbatim
  NW  N  NE
    \ | /
  W-- C --E
    / | \
  SW  S  SE
 \endverbatim

  * Matrix entries for the mass balance of a 27 star system
 *
 * The entries are center, east, west, north, south, north-east, south-east,
 * north-wast, south-west, same for top and bottom and the 
 * right side vector b of Ax = b. This system is typically used in 2d.
 
\verbatim
top:
NW_T N_Z NE_T
    \ | /
W_T-- T --E_T
    / | \
SW_T S_T SE_T

center:
  NW  N  NE
    \ | /
  W-- C --E
    / | \
  SW  S  SE

bottom:
NW_B N_B NE_B
    \ | /
W_B-- B --E_B
    / | \
SW_B S_B SE_B
\endverbatim

  */
typedef struct
{
  int type;
  int count;
  double C, W, E, N, S, NE, NW, SE, SW, V;
  /*top part*/
  double T, W_T, E_T, N_T, S_T, NE_T, NW_T, SE_T, SW_T;
  /*bottom part*/
  double B, W_B, E_B, N_B, S_B, NE_B, NW_B, SE_B, SW_B;
} N_data_star;

/*!
 * \brief callback structure for 3d matrix assembling
 * */
typedef struct
{
  N_data_star *(*callback) ();
} N_les_callback_3d;

/*!
 * \brief callback structure for 2d matrix assembling
 * */
typedef struct
{
  N_data_star *(*callback) ();
} N_les_callback_2d;


extern void N_set_les_callback_3d_func (N_les_callback_3d * data, N_data_star * (*callback_func_3d) ());
extern void N_set_les_callback_2d_func (N_les_callback_2d * data, N_data_star * (*callback_func_2d) ());
extern N_les_callback_3d *N_alloc_les_callback_3d ();
extern N_les_callback_2d *N_alloc_les_callback_2d ();
extern N_data_star *N_alloc_5star ();
extern N_data_star *N_alloc_7star ();
extern N_data_star *N_alloc_9star ();
extern N_data_star *N_alloc_27star ();
extern N_data_star *N_create_5star (double C, double W, double E, double N, double S, double V);
extern N_data_star *N_create_7star (double C, double W, double E, double N, double S, double T, double B,
						 double V);
extern N_data_star *N_create_9star (double C, double W, double E, double N, double S, double NW, double SW, double NE, double SE, double V);
extern N_data_star *N_create_27star (double C, double W, double E, double N, double S, double NW, 
						  double SW, double NE, double SE,
						  double T, double W_T, double E_T, double N_T, double S_T, double NW_T, 
						  double SW_T, double NE_T, double SE_T,
						  double B, double W_B, double E_B, double N_B, double S_B, double NW_B, 
						  double SW_B, double NE_B, double SE_B, double V);

extern N_data_star *N_callback_template_3d (void *data, N_geom_data * geom, int col, int row, int depth);
extern N_data_star *N_callback_template_2d (void *data, N_geom_data * geom, int col, int row);
extern N_les *N_assemble_les_3d (int les_type, N_geom_data * geom, N_array_3d * status, N_array_3d * start_val,
				 void *data, N_les_callback_3d * callback);
extern N_les *N_assemble_les_2d (int les_type, N_geom_data * geom, N_array_2d * status, N_array_2d * start_val,
				 void *data, N_les_callback_2d * callback);

/* *************************************************************** */
/* *************** METHODS FOR GRADIENT CALCULATION ************** */
/* *************************************************************** */
/*!
\verbatim

 ______________ 
|    |    |    |
|    |    |    |
|----|-NC-|----|
|    |    |    |
|   WC    EC   |
|    |    |    |
|----|-SC-|----|
|    |    |    |
|____|____|____|


      |  /
     TC NC
      |/
--WC-----EC--
     /|
   SC BC
   /  |

\endverbatim

*/

/*! \brief Gradient between the cells in X and Y direction */
typedef struct {

  double NC, SC, WC, EC;

} N_gradient_2d;

/*! \brief Gradient between the cells in X, Y and Z direction */
typedef struct {

  double NC, SC, WC, EC, TC, BC;

} N_gradient_3d;


/*!
\verbatim

Gradient in X direction between the cell neighbours
 ____ ____ ____
|    |    |    |
|   NWN  NEN   |
|____|____|____|
|    |    |    |
|   WN    EN   |
|____|____|____|
|    |    |    |
|   SWS  SES   |
|____|____|____|

Gradient in Y direction between the cell neighbours
 ______________ 
|    |    |    |
|    |    |    |
|NWW-|-NC-|-NEE|
|    |    |    |
|    |    |    |
|SWW-|-SC-|-SEE|
|    |    |    |
|____|____|____|

Gradient in Z direction between the cell neighbours
 /______________/
/|    |    |    |
 | NWZ| NZ | NEZ|
 |____|____|____|
/|    |    |    |
 | WZ | CZ | EZ |
 |____|____|____|
/|    |    |    |
 | SWZ| SZ | SEZ|
 |____|____|____|
/____/____/____/


\endverbatim
*/

/*! \brief Gradient between the cell neighbours in X direction */
typedef struct {

  double NWN, NEN, WC, EC, SWS, SES;

} N_gradient_neighbours_x;

/*! \brief Gradient between the cell neighbours in Y direction */
typedef struct {

  double NWW, NEE, NC, SC, SWW, SEE;

} N_gradient_neighbours_y;

/*! \brief Gradient between the cell neighbours in Z direction */
typedef struct {

  double NWZ, NZ, NEZ, WZ, CZ, EZ, SWZ, SZ, SEZ;

} N_gradient_neighbours_z;

/*! \brief Gradient between the cell neighbours in X and Y direction */
typedef struct {

  N_gradient_neighbours_x *x;
  N_gradient_neighbours_y *y;

} N_gradient_neighbours_2d;


/*! \brief Gradient between the cell neighbours in X, Y and Z direction */
typedef struct {

  N_gradient_neighbours_x *xt; /*top values*/
  N_gradient_neighbours_x *xc; /*center values*/
  N_gradient_neighbours_x *xb; /*bottom values*/

  N_gradient_neighbours_y *yt; /*top values*/
  N_gradient_neighbours_y *yc; /*center values*/
  N_gradient_neighbours_y *yb; /*bottom values*/

  N_gradient_neighbours_z *zt; /*top-center values*/
  N_gradient_neighbours_z *zb; /*bottom-center values*/

} N_gradient_neighbours_3d;


/*! Two dimensional gradient field*/
typedef struct {

  N_array_2d *x_array;
  N_array_2d *y_array;

} N_gradient_field_2d;

/*! Three dimensional gradient field*/
typedef struct {

  N_array_3d *x_array;
  N_array_3d *y_array;
  N_array_3d *z_array;

} N_gradient_field_3d;


extern N_gradient_2d * N_alloc_gradient_2d();
extern void N_free_gradient_2d(N_gradient_2d * grad);
extern N_gradient_2d * N_create_gradient_2d(double NC, double SC, double WC, double EC);
extern int N_copy_gradient_2d(N_gradient_2d * source, N_gradient_2d *target);
extern N_gradient_2d * N_get_gradient_2d(N_gradient_field_2d *field, N_gradient_2d * gradient, int col, int row);

extern N_gradient_3d * N_alloc_gradient_3d();
extern void N_free_gradient_3d(N_gradient_3d * grad);
extern N_gradient_3d * N_create_gradient_3d(double NC, double SC, double WC, double EC, double TC, double BC);
extern int N_copy_gradient_3d(N_gradient_3d * source, N_gradient_3d *target);
extern N_gradient_3d * N_get_gradient_3d(N_gradient_field_3d *field, N_gradient_3d * gradient, int col, int row, int depth);

extern N_gradient_neighbours_x  * N_alloc_gradient_neighbours_x();
extern void N_free_gradient_neighbours_x(N_gradient_neighbours_x  *grad);
extern N_gradient_neighbours_x  * N_create_gradient_neighbours_x(double NWN, double NEN, double WC, double EC, double SWS, double SES);
extern int N_copy_gradient_neighbours_x(N_gradient_neighbours_x * source, N_gradient_neighbours_x *target);

extern N_gradient_neighbours_y  * N_alloc_gradient_neighbours_y();
extern void N_free_gradient_neighbours_y(N_gradient_neighbours_y *grad);
extern N_gradient_neighbours_y  * N_create_gradient_neighbours_y(double NWW, double NEE, double NC, double SC, double SWW, double SEE);
extern int N_copy_gradient_neighbours_y(N_gradient_neighbours_y * source, N_gradient_neighbours_y *target);

extern N_gradient_neighbours_z  * N_alloc_gradient_neighbours_z();
extern void N_free_gradient_neighbours_z(N_gradient_neighbours_z  *grad);
extern N_gradient_neighbours_z  * N_create_gradient_neighbours_z(double NWZ, double NZ, double NEZ, double WZ, double CZ, double EZ, 
							  double SWZ, double SZ, double SEZ);
extern int N_copy_gradient_neighbours_z(N_gradient_neighbours_z * source, N_gradient_neighbours_z *target);

extern N_gradient_neighbours_2d * N_alloc_gradient_neighbours_2d();
extern void N_free_gradient_neighbours_2d(N_gradient_neighbours_2d *grad);
extern N_gradient_neighbours_2d * N_create_gradient_neighbours_2d(N_gradient_neighbours_x *x, N_gradient_neighbours_y *y);
extern int N_copy_gradient_neighbours_2d(N_gradient_neighbours_2d *source, N_gradient_neighbours_2d *target);

extern N_gradient_neighbours_3d * N_alloc_gradient_neighbours_3d();
extern void N_free_gradient_neighbours_3d(N_gradient_neighbours_3d *grad);
extern N_gradient_neighbours_3d * N_create_gradient_neighbours_3d(N_gradient_neighbours_x *xt, N_gradient_neighbours_x *xc, N_gradient_neighbours_x *xb, 
				     N_gradient_neighbours_y *yt, N_gradient_neighbours_y *yc, N_gradient_neighbours_y *yb,
				     N_gradient_neighbours_z *zt, N_gradient_neighbours_z *zb);
extern int N_copy_gradient_neighbours_3d(N_gradient_neighbours_3d *source, N_gradient_neighbours_3d *target);

extern N_gradient_field_2d * N_alloc_gradient_field_2d(int cols, int rows);
extern void N_free_gradient_field_2d(N_gradient_field_2d *field);
extern int N_copy_gradient_field_2d(N_gradient_field_2d *source, N_gradient_field_2d *target);
extern N_gradient_field_2d * N_compute_gradient_field_2d(N_array_2d *pot, N_array_2d *relax_x,  N_array_2d *relax_y,N_geom_data *geom);
extern void N_compute_gradient_field_components_2d(N_gradient_field_2d *field, N_array_2d *x_comp, N_array_2d *y_comp);

extern N_gradient_field_3d * N_alloc_gradient_field_3d(int cols, int rows, int depths);
extern void N_free_gradient_field_3d(N_gradient_field_3d *field);
extern int N_copy_gradient_field_3d(N_gradient_field_3d *source, N_gradient_field_3d *target);
extern N_gradient_field_3d * N_compute_gradient_field_3d(N_array_3d *pot, N_array_3d *relax_x, N_array_3d *relax_y, N_array_3d *relax_z, N_geom_data *geom);
extern void N_compute_gradient_field_components_3d(N_gradient_field_3d *field, N_array_3d *x_comp, N_array_3d *y_comp, N_array_3d *z_comp);

extern N_gradient_neighbours_2d * N_get_gradient_neighbours_2d(N_gradient_field_2d *field, int col, int row);
extern N_gradient_neighbours_3d * N_get_gradient_neighbours_3d(N_gradient_field_3d *field, int col, int row, int depth);

#endif
