/*!
  \file db/dbmi_base/column.c
  
  \brief DBMI Library (base) - columns management
  
  (C) 1999-2009 by the GRASS Development Team
  
  This program is free software under the GNU General Public
  License (>=v2). Read the file COPYING that comes with GRASS
  for details.
  
  \author Joel Jones (CERL/UIUC), Radim Blazek
 */

#include <stdlib.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/dbmi.h>

/*!
  \brief Returns column structure for given table and column number
  
  \param table pointer to dbTable
  \param n     column index (starting with '0')

  \return pointer to dbColumn
  \return NULL on error
*/
dbColumn *db_get_table_column(dbTable * table, int n)
{
    if (n < 0 || n >= table->numColumns)
	return ((dbColumn *) NULL);
    return &table->columns[n];
}

/*!
  \brief Returns column value for given column structure

  \param column pointer to dbColumn

  \return pointer to dbValue
*/
dbValue *db_get_column_value(dbColumn * column)
{
    return &column->value;
}

/*!
  \brief Returns column default value for given column structure
   
  \param column pointer to dbColumn
  
  \return pointer to dbValue
*/
dbValue *db_get_column_default_value(dbColumn * column)
{
    return &column->defaultValue;
}

/*!
  \brief Define column sqltype for column (the function db_sqltype_name() 
  returns sqltype description)

  \verbatim
  #define DB_SQL_TYPE_UNKNOWN          0
  
  #define DB_SQL_TYPE_CHARACTER        1
  #define DB_SQL_TYPE_SMALLINT         2
  #define DB_SQL_TYPE_INTEGER          3
  #define DB_SQL_TYPE_REAL             4
  #define DB_SQL_TYPE_DOUBLE_PRECISION 6
  #define DB_SQL_TYPE_DECIMAL          7
  #define DB_SQL_TYPE_NUMERIC          8
  #define DB_SQL_TYPE_DATE             9
  #define DB_SQL_TYPE_TIME            10
  #define DB_SQL_TYPE_TIMESTAMP       11
  #define DB_SQL_TYPE_INTERVAL        12
  #define DB_SQL_TYPE_TEXT            13
  
  #define DB_SQL_TYPE_SERIAL          21
  \endverbatim

  \param column pointer to dbColumn
  \param sqltype SQL data type (see list)
*/
void db_set_column_sqltype(dbColumn * column, int sqltype)
{
    column->sqlDataType = sqltype;
}

/*!
  \brief Set column host data type

  \param column pointer to dbColumn
  \param type data type
*/
void db_set_column_host_type(dbColumn * column, int type)
{
    column->hostDataType = type;
}

/*!
  \brief Get column scale

  \param column pointer to dbColumn

  \return scale
*/
int db_get_column_scale(dbColumn * column)
{
    return column->scale;
}

/*!
  \brief Set column scale

  \param column pointer to dbColumn
  \param scale  column scale value
*/
void db_set_column_scale(dbColumn * column, int scale)
{
    column->scale = scale;
}

/*!
  \brief Get column precision

  \param column pointer to dbColumn
  
  \return precision
*/
int db_get_column_precision(dbColumn * column)
{
    return column->precision;
}

/*!
  \brief Set column precision

  \param column pointer to dbColumn
  \param precision value
*/
void db_set_column_precision(dbColumn * column, int precision)
{
    column->precision = precision;
}

/*!
  \brief Returns column sqltype for column (the function
  db_sqltype_name() returns sqltype description) \return \param
  
  \param column pointer to dbColumn
  
  \return sql data type (see include/dbmi.h)
*/
int db_get_column_sqltype(dbColumn * column)
{
    return column->sqlDataType;
}

/*!
  \brief Get column host type

  \param column pointer to dbColumn
  
  \return data type (see include/dbmi.h)
*/
int db_get_column_host_type(dbColumn * column)
{
    return column->hostDataType;
}

/*!
  \brief Set default value identificator

  \param column pointer to dbColumn
*/
void db_set_column_has_defined_default_value(dbColumn * column)
{
    column->hasDefaultValue = 1;
}

/*!
  \brief Unset default value identificator

  \param column pointer to dbColumn
*/
void db_set_column_has_undefined_default_value(dbColumn * column)
{
    column->hasDefaultValue = 0;
}

/*!
  \brief Unset default value identificator

  \param column pointer to dbColumn
*/
void db_unset_column_has_default_value(dbColumn * column)
{
    column->hasDefaultValue = 0;
}

/*!
  \brief Check if column has defined default value

  \param column pointer to dbColumn

  \return 1 if true
  \return 0 if false
*/
int db_test_column_has_default_value(dbColumn * column)
{
    return (column->hasDefaultValue != 0);
}

/*!
  \brief Check if column has defined default value

  \param column pointer to dbColumn

  \return 1 if true
  \return 0 if false
*/
int db_test_column_has_defined_default_value(dbColumn * column)
{
    return (column->hasDefaultValue);
}

/*!
  \brief Check if column has defined default value

  \param column pointer to dbColumn

  \return 1 if false
  \return 0 if true
*/
int db_test_column_has_undefined_default_value(dbColumn * column)
{
    return (!column->hasDefaultValue);
}

/*!
  \brief Set default value to be used

  \param column pointer to dbColumn
*/
void db_set_column_use_default_value(dbColumn * column)
{
    column->useDefaultValue = 1;
}

/*!
  \brief Unset default value to be used

  \param column pointer to dbColumn
*/
void db_unset_column_use_default_value(dbColumn * column)
{
    column->useDefaultValue = 0;
}

/*!
  \brief Checks if default value is used

  \param column pointer to dbColumn

  \return 1 if true
  \return 0 if false
*/
int db_test_column_use_default_value(dbColumn * column)
{
    return (column->useDefaultValue != 0);
}

/*!
  \brief Set null value to be allowed

  \param column pointer to dbColumn
*/
void db_set_column_null_allowed(dbColumn * column)
{
    column->nullAllowed = 1;
}

/*!
  \brief Unset null value to be allowed

  \param column pointer to dbColumn
*/
void db_unset_column_null_allowed(dbColumn * column)
{
    column->nullAllowed = 0;
}

/*!
  \brief Checks if null value is allowed

  \param column pointer to dbColumn

  \return 1 if true
  \return 0 if false
 */
int db_test_column_null_allowed(dbColumn * column)
{
    return (column->nullAllowed != 0);
}

/*!
  \brief Get column's length

  \param column pointer to dbColumn

  \return length
*/
int db_get_column_length(dbColumn * column)
{
    return column->dataLen;
}

/*!
  \brief Set column's length

  \param column pointer to dbColumn
  \param length value
*/
void db_set_column_length(dbColumn * column, int length)
{
    column->dataLen = length;
}

/*!
  \brief Set select privilages to be granted

  \param column pointer to dbColumn
*/
void db_set_column_select_priv_granted(dbColumn * column)
{
    column->select = DB_GRANTED;
}

/*!
  \brief Unset select privilages

  \param column pointer to dbColumn
*/
void db_set_column_select_priv_not_granted(dbColumn * column)
{
    column->select = DB_NOT_GRANTED;
}

/*!
  \brief Get select privilages

  \param column pointer to dbColumn

  \return privilages
*/
int db_get_column_select_priv(dbColumn * column)
{
    return column->select;
}

/*!
  \brief Set update privilages to be granted

  \param column pointer to dbColumn
*/
void db_set_column_update_priv_granted(dbColumn * column)
{
    column->update = DB_GRANTED;
}

/*!
  \brief Unset update privilages

  \param column pointer to dbColumn
*/
void db_set_column_update_priv_not_granted(dbColumn * column)
{
    column->update = DB_NOT_GRANTED;
}

/*!
  \brief Get update privilages

  \param column pointer to dbColumn

  \return privilages
*/
int db_get_column_update_priv(dbColumn * column)
{
    return column->update;
}

/*!
  \brief Initialize dbColumn

  \param column pointer to dbColumn to be initialized
*/
void db_init_column(dbColumn * column)
{
    db_zero((void *)column, sizeof(dbColumn));
    db_init_string(&column->columnName);
    db_init_string(&column->description);
    db_init_string(&column->value.s);
    db_init_string(&column->defaultValue.s);
}

/*!
  \brief Set column name

  \param column pointer to dbColumn
  \param name   column name

  \return DB_OK on success
  \return error code on failure
*/
int db_set_column_name(dbColumn * column, const char *name)
{
    return db_set_string(&column->columnName, name);
}

/*!
  \brief Returns column name for given column

  \param column pointer to dbColumn

  \return pointer to string with column name
*/
const char *db_get_column_name(dbColumn * column)
{
    return db_get_string(&column->columnName);
}

/*!
  \brief Set column description

  \param column pointer to dbColumn
  \param description column's description

  \return DB_OK on success
  \return error code on failure
 */
int db_set_column_description(dbColumn * column, const char *description)
{
    return db_set_string(&column->description, description);
}

/*!
  \brief Returns column description for given column

  \param column pointer to dbColumn

  \return pointer to string with column's description
*/
const char *db_get_column_description(dbColumn * column)
{
    return db_get_string(&column->description);
}

/*!
  \brief Frees column structure

  \param column pointer to dbColumn 
*/
void db_free_column(dbColumn * column)
{
    db_free_string(&column->columnName);
    db_free_string(&column->value.s);
    /* match init? */
    db_free_string(&column->description);
    db_free_string(&column->defaultValue.s);
}
