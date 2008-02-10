
/****************************************************************
 *
 * MODULE:     v.generalize
 *
 * AUTHOR(S):  Daniel Bundala
 *
 * PURPOSE:    Module for line simplification and smoothing
 *
 * COPYRIGHT:  (C) 2002-2005 by the GRASS Development Team
 *
 *             This program is free software under the
 *             GNU General Public License (>=v2).
 *             Read the file COPYING that comes with GRASS
 *             for details.
 *
 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <grass/gis.h>
#include <grass/Vect.h>
#include <grass/glocale.h>
#include "point.h"
#include "matrix.h"

/* boyle's forward looking algorithm
 * return the number of points in the result = Points->n_points
 */
int boyle(struct line_pnts *Points, int look_ahead, int with_z)
{
    POINT last, npoint, ppoint;
    int next, n, i, p;
    double c1, c2;

    n = Points->n_points;

    /* if look_ahead is too small or line too short, there's nothing
     * to smooth */
    if (look_ahead < 2 || look_ahead > n) {
	return n;
    }

    point_assign(Points, 0, with_z, &last);
    c1 = (double)1 / (double)(look_ahead - 1);
    c2 = (double)1 - c1;
    next = 1;

    for (i = 0; i < n - 2; i++) {
	p = i + look_ahead;
	if (p >= n)
	    p = n - 1;
	point_assign(Points, p, with_z, &ppoint);
	point_scalar(ppoint, c1, &ppoint);
	point_scalar(last, c2, &last);
	point_add(last, ppoint, &npoint);
	Points->x[next] = npoint.x;
	Points->y[next] = npoint.y;
	Points->z[next] = npoint.z;
	next++;
	last = npoint;
    }

    points_copy_last(Points, next);
    return Points->n_points;

}

/* mcmaster's sliding averaging algorithm. Return the number of points
 * in the output line. This equals to the number of points in the
 * input line */
int sliding_averaging(struct line_pnts *Points, double slide, int look_ahead,
		      int with_z)
{

    int n, half, i;
    double sc;
    POINT p, tmp, s;
    POINT *res;
    n = Points->n_points;
    half = look_ahead / 2;

    if (look_ahead % 2 == 0) {
	G_fatal_error(_("Look ahead parameter must be odd"));
	return n;
    }

    if (look_ahead >= n || look_ahead == 1)
	return n;

    res = G_malloc(sizeof(POINT) * n);
    if (!res) {
	G_fatal_error(_("Out of memory"));
	return n;
    }

    sc = (double)1.0 / (double)look_ahead;

    point_assign(Points, 0, with_z, &p);
    for (i = 1; i < look_ahead; i++) {
	point_assign(Points, i, with_z, &tmp);
	point_add(p, tmp, &p);
    }

    /* and calculate the average of remaining points */
    for (i = half; i + half < n; i++) {
	point_assign(Points, i, with_z, &s);
	point_scalar(s, 1.0 - slide, &s);
	point_scalar(p, sc * slide, &tmp);
	point_add(tmp, s, &res[i]);
	if (i + half + 1 < n) {
	    point_assign(Points, i - half, with_z, &tmp);
	    point_subtract(p, tmp, &p);
	    point_assign(Points, i + half + 1, with_z, &tmp);
	    point_add(p, tmp, &p);
	}
    }


    for (i = half; i + half < n; i++) {
	Points->x[i] = res[i].x;
	Points->y[i] = res[i].y;
	Points->z[i] = res[i].z;
    }

    G_free(res);
    return Points->n_points;

}

/* mcmaster's distance weighting algorithm. Return the number
 * of points in the output line which equals to Points->n_points */
int distance_weighting(struct line_pnts *Points, double slide, int look_ahead,
		       int with_z)
{
    POINT p, c, s, tmp;
    int n, i, half, j;
    double dists, d;
    POINT *res;

    n = Points->n_points;

    if (look_ahead % 2 == 0) {
	G_fatal_error(_("Look ahead parameter must be odd"));
	return n;
    }

    res = (POINT *) G_malloc(sizeof(POINT) * n);
    if (!res) {
	G_fatal_error(_("Out of memory"));
	return n;
    }

    point_assign(Points, 0, with_z, &res[0]);

    half = look_ahead / 2;

    for (i = half; i + half < n; i++) {
	point_assign(Points, i, with_z, &c);
	s.x = s.y = s.z = 0;
	dists = 0;


	for (j = i - half; j <= i + half; j++) {
	    if (j == i)
		continue;
	    point_assign(Points, j, with_z, &p);
	    d = point_dist(p, c);
	    if (d < GRASS_EPSILON)
		continue;
	    d = (double)1.0 / d;
	    dists += d;
	    point_scalar(p, d, &tmp);
	    s.x += tmp.x;
	    s.y += tmp.y;
	    s.z += tmp.z;
	}
	point_scalar(s, slide / dists, &tmp);
	point_scalar(c, (double)1.0 - slide, &s);
	point_add(s, tmp, &res[i]);
    }

    for (i = half; i + half < n; i++) {
	Points->x[i] = res[i].x;
	Points->y[i] = res[i].y;
	Points->z[i] = res[i].z;
    }

    G_free(res);
    return Points->n_points;
}


/* Chaiken's algorithm. Return the number of points in smoothed line 
 */
int chaiken(struct line_pnts *Points, double thresh, int with_z)
{

    int n, i;
    POINT_LIST head, *cur;
    POINT p0, p1, p2, m1, tmp;

    n = Points->n_points;

    /* line is too short */
    if (n < 3)
	return n;

    thresh *= thresh;

    head.next = NULL;
    cur = &head;
    point_assign(Points, 0, with_z, &head.p);
    point_assign(Points, 0, with_z, &p0);

    for (i = 2; i <= n; i++) {
	if (i == n)
	    point_assign(Points, i - 1, with_z, &p2);
	else
	    point_assign(Points, i, with_z, &p2);
	point_assign(Points, i - 1, with_z, &p1);

	while (1) {
	    point_add(p1, p2, &tmp);
	    point_scalar(tmp, 0.5, &m1);

	    point_list_add(cur, m1);

	    if (point_dist_square(p0, m1) > thresh) {
		point_add(p1, m1, &tmp);	/* need to refine the partition */
		point_scalar(tmp, 0.5, &p2);
		point_add(p1, p0, &tmp);
		point_scalar(tmp, 0.5, &p1);
	    }
	    else {
		break;		/* good approximatin */
	    }
	}

	while (cur->next != NULL)
	    cur = cur->next;

	p0 = cur->p;
    }

    point_assign(Points, n - 1, with_z, &p0);
    point_list_add(cur, p0);

    if (point_list_copy_to_line_pnts(head, Points) == -1) {
	G_fatal_error(_("Out of memory"));
    }
    point_list_free(head);
    return Points->n_points;
}


/* use for refining tangent in hermite interpolation */
void refine_tangent(POINT * p)
{
    double l = point_dist2(*p);
    if (l < GRASS_EPSILON) {
	point_scalar(*p, 0.0, p);
    }
    else {
	point_scalar(*p, (double)1.0 / sqrt(sqrt(sqrt(l))), p);
    }
    return;
}

/* approximates given line using hermite cubic spline
 * interpolates by steps of length step
 * returns the number of point in result
 */
int hermite(struct line_pnts *Points, double step, double angle_thresh,
	    int with_z)
{
    POINT_LIST head, *last, *point;
    POINT p0, p1, t0, t1, tmp, res;
    double length, next, length_begin, l;
    double s;
    double h1, h2, h3, h4;
    int n, i;
    int ni;

    n = Points->n_points;

    /* line is too short */
    if (n <= 2) {
	return n;
    }

    /* convert degrees=>radians */
    angle_thresh *= M_PI / 180.0;

    head.next = NULL;
    point = last = &head;

    /* length of p[0]..p[i+1] */
    i = 0;
    point_assign(Points, 0, with_z, &p0);
    point_assign(Points, 1, with_z, &p1);
    /* length of line 0..i */
    length_begin = 0;
    /* length of line from point 0 to i+1 */
    length = point_dist(p0, p1);
    next = 0;
    /* tangent at p0, p1 */
    point_subtract(p1, p0, &t0);
    refine_tangent(&t0);
    point_assign(Points, 2, with_z, &tmp);
    point_subtract(tmp, p0, &t1);
    refine_tangent(&t1);


    /* we always operate on the segment point[i]..point[i+1] */
    while (i < n - 1) {
	if (next > length || (length - length_begin < GRASS_EPSILON)) {	/* segmet i..i+1 is finished or too short */
	    i++;
	    if (i >= n - 1)
		break;		/* we are already out out of line */
	    point_assign(Points, i, with_z, &p0);
	    point_assign(Points, i + 1, with_z, &p1);
	    length_begin = length;
	    length += point_dist(p0, p1);
	    ni = i + 2;
	    if (ni > n - 1)
		ni = n - 1;	/* ensure that we are in the line */
	    t0 = t1;
	    point_assign(Points, ni, with_z, &tmp);
	    point_subtract(tmp, p0, &t1);
	    refine_tangent(&t1);
	}
	else {
	    l = length - length_begin;	/* length of actual segment */
	    s = (next - length_begin) / l;	/* 0<=s<=1 where we want to add new point on the line */

	    /* then we need to calculate 4 control polynomials */
	    h1 = 2 * s * s * s - 3 * s * s + 1;
	    h2 = -2 * s * s * s + 3 * s * s;
	    h3 = s * s * s - 2 * s * s + s;
	    h4 = s * s * s - s * s;

	    point_scalar(p0, h1, &res);
	    point_scalar(p1, h2, &tmp);
	    point_add(res, tmp, &res);
	    point_scalar(t0, h3, &tmp);
	    point_add(res, tmp, &res);
	    point_scalar(t1, h4, &tmp);
	    point_add(res, tmp, &res);
	    point_list_add(last, res);
	    last = last->next;

	    next += step;
	}
	/* if the angle between 2 vectors is less then eps, remove the
	 * middle point */
	if (point->next && point->next->next && point->next->next->next) {
	    if (point_angle_between
		(point->next->p, point->next->next->p,
		 point->next->next->next->p) < angle_thresh) {
		point_list_delete_next(point->next);
	    }
	    else
		point = point->next;
	}
    }


    point_assign(Points, n - 1, with_z, &p0);
    point_list_add(last, p0);

    if (point_list_copy_to_line_pnts(head, Points) == -1)
	G_fatal_error(_("Out of memory"));

    point_list_free(head);
    return Points->n_points;
}

/* snakes algorithm for line simplification/generalization 
 * returns the number of points in the output line. This is
 * always equal to the number of points in the original line
 * 
 * alpha, beta are 2 parameters which change the behaviour of the algorithm
 * 
 * TODO: Add parameter iterations, so the runnining time is O(N^3 * log iterations)
 * instead of O(N^3 * itearations). Probably not needed, for many iterations,
 * the result is almost straight line
 */
int snakes(struct line_pnts *Points, double alpha, double beta, int with_z)
{
    MATRIX g, ginv, xcoord, ycoord, zcoord, xout, yout, zout;

    int n = Points->n_points;
    int i, j;

    if (n < 3)
	return n;

    int plus = 4;

    if (!matrix_init(n + 2 * plus, n + 2 * plus, &g)) {
	G_fatal_error(_("Out of memory"));
	return n;
    }
    matrix_init(n + 2 * plus, 1, &xcoord);
    matrix_init(n + 2 * plus, 1, &ycoord);
    matrix_init(n + 2 * plus, 1, &zcoord);
    matrix_init(n + 2 * plus, 1, &xout);
    matrix_init(n + 2 * plus, 1, &yout);
    matrix_init(n + 2 * plus, 1, &zout);

    double x0 = Points->x[0];
    double y0 = Points->y[0];
    double z0 = Points->z[0];

    /* store the coordinates in the column vectors */
    for (i = 0; i < n; i++) {
	xcoord.a[i + plus][0] = Points->x[i] - x0;
	ycoord.a[i + plus][0] = Points->y[i] - y0;
	zcoord.a[i + plus][0] = Points->z[i] - z0;
    }

    /* repeat first and last point at the beginning and end
     * of each vector respectively */
    for (i = 0; i < plus; i++) {
	xcoord.a[i][0] = 0;
	ycoord.a[i][0] = 0;
	zcoord.a[i][0] = 0;
    }

    for (i = n + plus; i < n + 2 * plus; i++) {
	xcoord.a[i][0] = Points->x[n - 1] - x0;
	ycoord.a[i][0] = Points->y[n - 1] - y0;
	zcoord.a[i][0] = Points->z[n - 1] - z0;
    }


    /* calculate the matrix */
    double a = 2.0 * alpha + 6.0 * beta;
    double b = -alpha - 4.0 * beta;
    double c = beta;

    double val[5] = { c, b, a, b, c };

    for (i = 0; i < n + 2 * plus; i++)
	for (j = 0; j < n + 2 * plus; j++) {
	    int index = j - i + 2;
	    if (index >= 0 && index <= 4)
		g.a[i][j] = val[index];
	    else
		g.a[i][j] = 0;
	}

    matrix_add_identity((double)1.0, &g);

    /* find its inverse */
    if (!matrix_inverse(g, &ginv, 0)) {
	G_fatal_error(_("Unable to find the inverse matrix"));
	return n;
    }

    if (!matrix_mult(ginv, xcoord, &xout)
	|| !matrix_mult(ginv, ycoord, &yout)
	|| !matrix_mult(ginv, zcoord, &zout)) {
	G_fatal_error(_("Unable to calculate the output vectors"));
	return n;
    }

    /* copy the new values of coordinates, but
     * never move the last and first point */
    for (i = 1; i < n - 1; i++) {
	Points->x[i] = xout.a[i + plus][0] + x0;
	Points->y[i] = yout.a[i + plus][0] + y0;
	if (with_z)
	    Points->z[i] = zout.a[i + plus][0] + z0;
    }

    matrix_free(g);
    matrix_free(ginv);
    matrix_free(xcoord);
    matrix_free(ycoord);
    matrix_free(zcoord);
    matrix_free(xout);
    matrix_free(yout);
    matrix_free(zout);
    return Points->n_points;
}
