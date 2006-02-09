#include <grass/dbmi.h>
#include <sys/types.h>
#include <sys/stat.h>

/*!
 \fn 
 \brief 
 \return 
 \param 
*/
int
db_isdir  (char *path)

{
    struct stat x;

    if (stat(path, &x) != 0)
	return DB_FAILED;
    return ( S_ISDIR (x.st_mode) ? DB_OK : DB_FAILED);
}
