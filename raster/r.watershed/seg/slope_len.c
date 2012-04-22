#include "Gwater.h"

int slope_length(int r, int c, int dr, int dc)
{
    CELL top_alt, bot_alt, ridge;
    char asp_value;
    double res, top_ls, bot_ls;
    WAT_ALT wa;
    ASP_FLAG af;

    if (sides == 8) {
	if (r == dr)
	    res = window.ns_res;
	else if (c == dc)
	    res = window.ew_res;
	else
	    res = diag;
    }
    else {			/* sides == 4 */

	seg_get(&aspflag, (char *)&af, dr, dc);
	asp_value = af.asp;
	if (r == dr) {
	    if (asp_value == 2 || asp_value == 6)
		res = window.ns_res;
	    else		/* asp_value == 4, 8, -2, -4, -6, or -8 */
		res = diag;     /* how can res be diag with sides == 4??? */
	}
	else {			/* c == dc */

	    if (asp_value == 4 || asp_value == 8)
		res = window.ew_res;
	    else		/* asp_value == 2, 6, -2, -4, -6, or -8 */
		res = diag;
	}
    }
    dseg_get(&s_l, &top_ls, r, c);
    if (top_ls == half_res)
	top_ls = res;
    else
	top_ls += res;
    dseg_put(&s_l, &top_ls, r, c);
    seg_get(&watalt, (char *) &wa, r, c);
    top_alt = wa.ele;
    seg_get(&watalt, (char *) &wa, dr, dc);
    bot_alt = wa.ele;
    if (top_alt > bot_alt) {
	dseg_get(&s_l, &bot_ls, dr, dc);
	if (top_ls > bot_ls) {
	    bot_ls = top_ls + res;
	    dseg_put(&s_l, &bot_ls, dr, dc);
	    cseg_get(&r_h, &ridge, r, c);
	    cseg_put(&r_h, &ridge, dr, dc);
	}
    }

    return 0;
}
