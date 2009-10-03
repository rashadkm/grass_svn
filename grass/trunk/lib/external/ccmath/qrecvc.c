/*  qrecvc.c    CCMATH mathematics library source code.
 *
 *  Copyright (C)  2000   Daniel A. Atkinson    All rights reserved.
 *  This code may be redistributed under the terms of the GNU library
 *  public license (LGPL). ( See the lgpl.license file for details.)
 * ------------------------------------------------------------------------
 */
#include "ccmath.h"
void qrecvc(double *ev, Cpx * evec, double *dp, int n)
{
    double cc, sc = 0.0, d, x = 0.0, y, h = 0.0, tzr = 1.e-15;

    int i, j, k, m, nqr = 50 * n;

    Cpx *p;

    for (j = 0, m = n - 1; j < nqr; ++j) {
	while (1) {
	    if (m < 1)
		break;
	    k = m - 1;
	    if (fabs(dp[k]) <= fabs(ev[m]) * tzr)
		--m;
	    else {
		x = (ev[k] - ev[m]) / 2.;
		h = sqrt(x * x + dp[k] * dp[k]);
		if (m > 1 && fabs(dp[m - 2]) > fabs(ev[k]) * tzr)
		    break;
		if ((cc = sqrt((1. + x / h) / 2.)) != 0.)
		    sc = dp[k] / (2. * cc * h);
		else
		    sc = 1.;
		x += ev[m];
		ev[m--] = x - h;
		ev[m--] = x + h;
		for (i = 0, p = evec + n * (m + 1); i < n; ++i, ++p) {
		    h = p[0].re;
		    p[0].re = cc * h + sc * p[n].re;
		    p[n].re = cc * p[n].re - sc * h;
		    h = p[0].im;
		    p[0].im = cc * h + sc * p[n].im;
		    p[n].im = cc * p[n].im - sc * h;
		}
	    }
	}
	if (x > 0.)
	    d = ev[m] + x - h;
	else
	    d = ev[m] + x + h;
	cc = 1.;
	y = 0.;
	ev[0] -= d;
	for (k = 0; k < m; ++k) {
	    x = ev[k] * cc - y;
	    y = dp[k] * cc;
	    h = sqrt(x * x + dp[k] * dp[k]);
	    if (k > 0)
		dp[k - 1] = sc * h;
	    ev[k] = cc * h;
	    cc = x / h;
	    sc = dp[k] / h;
	    ev[k + 1] -= d;
	    y *= sc;
	    ev[k] = cc * (ev[k] + y) + ev[k + 1] * sc * sc + d;
	    for (i = 0, p = evec + n * k; i < n; ++i, ++p) {
		h = p[0].re;
		p[0].re = cc * h + sc * p[n].re;
		p[n].re = cc * p[n].re - sc * h;
		h = p[0].im;
		p[0].im = cc * h + sc * p[n].im;
		p[n].im = cc * p[n].im - sc * h;
	    }
	}
	ev[k] = ev[k] * cc - y;
	dp[k - 1] = ev[k] * sc;
	ev[k] = ev[k] * cc + d;
    }
}
