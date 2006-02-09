#include <grass/gis.h>

/*! 
 \fn G_colors_count ( struct Colors *colors )
 \brief get both modular and fixed rules count 
 \return number of rules in color table
 \param colors pointer to color table structure
*/
int G_colors_count ( struct Colors *colors )
{
    int count = 0;
    struct _Color_Rule_ *rule;
    
    if ( colors->fixed.rules ) {
	count++;
	rule = colors->fixed.rules;

	while (rule->next) {
	    count++;
	    rule = rule->next;
	}
    }
    if ( colors->modular.rules ) {
	count++;
	rule = colors->modular.rules;

	while (rule->next) {
	    count++;
	    rule = rule->next;
	}
    }
    return count;
}

/*! 
 \fn G_get_f_color_rule ( struct Colors *colors )
 \brief get color rule from both modular and fixed rules
        rules are returned in the order as stored in the table (i.e. unexpected, high values first)
 \return 0 success 
 \return 1 index out of range  
 \param colors pointer to color table structure
 \param rule rule index from 0 to G_color_count()-1
*/
int G_get_f_color_rule ( DCELL *val1, unsigned char *r1, unsigned char *g1, unsigned char *b1,
			 DCELL *val2, unsigned char *r2, unsigned char *g2, unsigned char *b2, 
			 struct Colors *colors, int rule )
{
    int index = -1;
    int found = 0;
    struct _Color_Rule_ *rl;

    *val1 = *val2 = 0.0;
    *r1 = *g1 = *b1 = *r2 = *g2 = *b2 = 0;
    
    /* Find the rule */
    if ( colors->fixed.rules ) {
	rl = colors->fixed.rules;
        index++;
	if ( index == rule ) found = 1;

	while ( !found && rl->next) {
	    rl = rl->next;
	    index++;
	    if ( index == rule ) found = 1;
	}
    } 
    if ( !found && colors->modular.rules ) {
	rl = colors->modular.rules;
	index++;
	if ( index == rule ) found = 1;

	while ( !found && rl->next) {
	    rl = rl->next;
	    index++;
	    if ( index == rule ) found = 1;
	}
    }

    if ( !found ) return 1;

    /* Set values */
    *val1 = rl->low.value;
    *val2 = rl->high.value;

    *r1 = rl->low.red;
    *g1 = rl->low.grn;
    *b1 = rl->low.blu;

    *r2 = rl->high.red;
    *g2 = rl->high.grn;
    *b2 = rl->high.blu;

    return 0;
}

