#include <grass/dbmi.h>

/* these routines manage a mapping between tokens (ints) and memory addresses */
#define NONE ( (dbAddress) NULL )

static dbAddress *list = NONE;
static dbToken count = 0;

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
dbAddress 
db_find_token  (dbToken token)

{
    if (token >= 0 && token < count)
	return list[token];
    return (NONE);
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
void
db_drop_token  (dbToken token)

{
    if (token >= 0 && token < count)
	list[token] = NONE;
}

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
dbToken
db_new_token  (dbAddress address)

{
    dbToken token;
    dbAddress *p;

    for (token = 0; token < count; token++)
	if (list[token] == NONE)
	{
	    list[token] = address;
	    return token;
	}
    
    p = (dbAddress *) db_realloc ((void *)list, sizeof(*list) * (count+1));
    if (p == NULL)
	return -1;
    
    list = p;
    token = count++;
    list[token] = address;
    return (token);
}
