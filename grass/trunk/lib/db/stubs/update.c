#include <grass/dbmi.h>

int
db__driver_update (dbCursor *cursor)
{
    db_procedure_not_implemented("db_update");
    return DB_FAILED;
}
