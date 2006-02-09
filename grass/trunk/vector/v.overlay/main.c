/* ****************************************************************************
 *
 *  MODULE: v.overlay 
 *
 *  AUTHOR(S): Radim Blazek
 *  
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <grass/gis.h>
#include <grass/dbmi.h>
#include <grass/Vect.h>
#include "local.h"

int 
main (int argc, char *argv[])
{
    int    i, input, line, nlines, operator;
    int    type[2], field[2], ofield[3];
    char   *mapset[2];
    char   *pre[2];
    struct GModule *module;
    struct Option *in_opt[2], *out_opt, *type_opt[2], *field_opt[2], *ofield_opt, *operator_opt;
    struct Flag *table_flag;
    struct Map_info In[2], Out;
    struct line_pnts *Points;
    struct line_cats *Cats;

    struct   field_info *Fi=NULL;
    char     buf[1000];
    dbString stmt;
    dbString sql, value_string, col_defs;
    dbDriver *driver;
    ATTRIBUTES attr[2];

    G_gisinit (argv[0]);

    pre[0] ="a";
    pre[1] ="b";

    module = G_define_module();
    module->description = "Overlay 2 vector maps.";

    in_opt[0] = G_define_standard_option(G_OPT_V_INPUT);
    in_opt[0]->key = "ainput";

    type_opt[0] = G_define_standard_option(G_OPT_V_TYPE) ;
    type_opt[0]->key = "atype";
    type_opt[0]->options = "line,area";
    type_opt[0]->answer = "area";

    field_opt[0] = G_define_standard_option(G_OPT_V_FIELD);
    field_opt[0]->key = "alayer";

    in_opt[1] = G_define_standard_option(G_OPT_V_INPUT);
    in_opt[1]->key = "binput";

    type_opt[1] = G_define_standard_option(G_OPT_V_TYPE) ;
    type_opt[1]->key = "btype";
    type_opt[1]->options = "area";
    type_opt[1]->answer = "area";
    
    field_opt[1] = G_define_standard_option(G_OPT_V_FIELD);
    field_opt[1]->key = "blayer";
    
    out_opt = G_define_standard_option(G_OPT_V_OUTPUT);

    operator_opt = G_define_option();
    operator_opt->key = "operator";
    operator_opt->type = TYPE_STRING;
    operator_opt->required = NO;
    operator_opt->multiple = NO;
    operator_opt->options = "and,or,not,xor";
    operator_opt->answer = "or";
    operator_opt->description = "Operator defines features written to output vector. "
	"Feature is written to output if the result of operation 'ainput operator binput' is true. "
	"Input feature is considered to be true, if category of given layer is defined.\n"
	"\t and : also known as 'intersection' in GIS\n"
	"\t or  : also known as 'union' in GIS (only for atype=area)\n"
	"\t not : features from ainput not overlayed by features from binput\n"
	"\t xor : features from either ainput or binput but not those from ainput overlayed by binput "
	"\t       (only for atype=area)";
    
    ofield_opt = G_define_standard_option(G_OPT_V_FIELD);
    ofield_opt->key = "olayer";
    ofield_opt->multiple = YES;
    ofield_opt->answer = "1,0,0";
    ofield_opt->description = "Output layer for new category, ainput and binput. If 0 or not given, "
	                      "the category is not written.";

    table_flag = G_define_flag ();
    table_flag->key             = 't';
    table_flag->description     = "Do not create attribute table.";

    if (G_parser (argc, argv)) exit(-1);

    for ( input = 0; input < 2; input++ ) {
        type[input] = Vect_option_to_types ( type_opt[input] );
	field[input] = atoi(field_opt[input]->answer);
    }

    ofield[0] = ofield[1] = ofield[2] = 0;
    i = 0;
    while ( ofield_opt->answers[i] && i < 3 ) {
	ofield[i] = atoi ( ofield_opt->answers[i] );
	i++;
    }

    if ( operator_opt->answer[0] == 'a' ) operator = OP_AND;
    else if ( operator_opt->answer[0] == 'o' ) operator = OP_OR;
    else if ( operator_opt->answer[0] == 'n' ) operator = OP_NOT;
    else if ( operator_opt->answer[0] == 'x' ) operator = OP_XOR;

    /* OP_OR, OP_XOR is not supported for lines, mostly because I'am not sure if they make enouhg sense */
    if ( type[0] == GV_LINE && (operator == OP_OR || operator == OP_XOR) )
	G_fatal_error ( "Operator '%s' is not supported for type line.",  operator_opt->answer );

    Vect_check_input_output_name ( in_opt[0]->answer, out_opt->answer, GV_FATAL_EXIT );
    Vect_check_input_output_name ( in_opt[1]->answer, out_opt->answer, GV_FATAL_EXIT );
    
    Points = Vect_new_line_struct();
    Cats = Vect_new_cats_struct();

    /* Open output */
    Vect_open_new (&Out, out_opt->answer, 0);
    Vect_set_map_name ( &Out, "Output from v.overlay");
    Vect_set_person ( &Out, G_whoami ());
    Vect_hist_command ( &Out );

    /* Create dblinks */
    if ( ofield[0] > 0 ) {
        Fi = Vect_default_field_info ( &Out, ofield[0], NULL, GV_1TABLE );
    }

    db_init_string ( &sql );
    db_init_string ( &value_string );
    db_init_string ( &col_defs );

    /* Open database */
    if ( ofield[0] > 0 && !(table_flag->answer) ) {
	fprintf (stderr, SEP );
	
	db_init_string (&stmt);
	driver = db_start_driver_open_database ( Fi->driver, Vect_subst_var(Fi->database, &Out) );
	if ( driver == NULL ) {
	    Vect_close (&Out);
	    G_fatal_error ( "Cannot open database %s by driver %s", Fi->database, Fi->driver );
	}
	db_begin_transaction ( driver );
    } else {
	driver = NULL;
    }
    
    /* Copy lines to output */
    for ( input = 0; input < 2; input++ ) {
	int ncats, index;
	
	fprintf (stderr, "Copying %sinput lines ... ", pre[input]);

	if ((mapset[input] = G_find_vector2 (in_opt[input]->answer, NULL)) == NULL) {
	    G_fatal_error ("Could not find vector '%s'\n", in_opt[input]->answer);
	}

	Vect_set_open_level (2);
	Vect_open_old (&(In[input]), in_opt[input]->answer, mapset[input] );

	nlines = Vect_get_num_lines ( &(In[input]) );

	for ( line = 1; line <= nlines; line++ ) {
	    int ltype;

	    G_percent ( line, nlines, 1 ); /* must be before any continue */
	    
	    ltype = Vect_read_line ( &(In[input]), Points, Cats, line);

	    if ( type[input] == GV_AREA ) {
		if ( !(ltype & GV_BOUNDARY) ) continue;
	    } else { /* GV_LINE */
		if ( !(ltype & type[input]) ) continue;
	    }

	    Vect_write_line ( &Out, ltype, Points, Cats );
	}

	/* Allocate attributes */
	attr[input].n = 0;
	attr[input].attr = (ATTR *) G_calloc ( Vect_cidx_get_type_count( &(In[input]), 
		            field[input], type[input]) , sizeof(ATTR) ); /* this may be more than necessary */

	index = Vect_cidx_get_field_index (  &(In[input]), field[input] );

	if ( index >= 0 ) {
	    ncats = Vect_cidx_get_num_cats_by_index ( &(In[input]), index );
	    for ( i = 0; i < ncats; i++ ) {
		int cat, ctype, id;

		Vect_cidx_get_cat_by_index ( &(In[input]), index, i, &cat, &ctype, &id );
		if ( !(ctype & type[input]) ) continue;
		      
		if ( attr[input].n == 0 || cat != attr[input].attr[attr[input].n-1].cat ) {
		    attr[input].attr[attr[input].n].cat = cat;
		    attr[input].n++;
		}
	    }
	}

	G_debug ( 3, "%d cats read from index", attr[input].n );

        fprintf (stderr, "Collecting input attributes ...\n");

	attr[input].null_values = NULL;
	attr[input].columns = NULL;
	
	/* Attributes */
	if ( driver ) {
	    int ncol, more;
	    struct field_info *inFi;
	    dbDriver *in_driver;
	    dbCursor cursor;
	    dbTable  *Table;
	    dbColumn *Column;
	    dbValue  *Value;
	    int sqltype, ctype;

	    inFi = Vect_get_field ( &(In[input]), field[input] );
	    if (!inFi ) {
		G_warning ( "Database connection not defined for layer %d", field[input]);
		continue;
	    }

	    in_driver = db_start_driver_open_database ( inFi->driver, inFi->database );
	    if ( in_driver == NULL ) {
		G_fatal_error ( "Cannot open database %s by driver %s", inFi->database, inFi->driver );
	    }

	    sprintf ( buf, "select * from %s", inFi->table );
	    db_set_string( &sql, buf );

	    if ( db_open_select_cursor( in_driver, &sql, &cursor, DB_SEQUENTIAL) != DB_OK )
		G_fatal_error ( "Cannot select attributes");

	    Table = db_get_cursor_table (&cursor);
	    ncol = db_get_table_number_of_columns(Table);
	    G_debug ( 3, "ncol = %d", ncol );
	       
	    db_set_string( &sql, "" );
	    db_set_string( &col_defs, "" );
	    for (i = 0; i < ncol; i++) {
	       db_append_string( &sql, ", null" );

	       Column = db_get_table_column (Table, i);
	       sqltype = db_get_column_sqltype (Column);
	       ctype = db_sqltype_to_Ctype(sqltype);
	       
	       if ( input == 0 )
	           db_append_string ( &col_defs, ", a_" );
	       else
	           db_append_string ( &col_defs, ", b_" );

	       db_append_string ( &col_defs, db_get_column_name (Column) );
	       db_append_string ( &col_defs, " " );
	       switch ( sqltype ) {
		    case DB_SQL_TYPE_CHARACTER:
			sprintf (buf, "varchar(%d)", db_get_column_length (Column) );
			db_append_string ( &col_defs, buf);
			break;
		    case DB_SQL_TYPE_SMALLINT:
		    case DB_SQL_TYPE_INTEGER:
			db_append_string ( &col_defs, "integer");
			break;
		    case DB_SQL_TYPE_REAL:
		    case DB_SQL_TYPE_DOUBLE_PRECISION:
		    case DB_SQL_TYPE_DECIMAL:
		    case DB_SQL_TYPE_NUMERIC:
		    case DB_SQL_TYPE_INTERVAL:
			db_append_string ( &col_defs, "double precision");
			break;
		    case DB_SQL_TYPE_DATE:
			db_append_string ( &col_defs, "date");
			break;
		    case DB_SQL_TYPE_TIME:
			db_append_string ( &col_defs, "time");
			break;
		    case DB_SQL_TYPE_TIMESTAMP:
			db_append_string ( &col_defs, "datetime");
			break;
		    default:
			G_warning ( "Unknown column type (%s)", db_get_column_name (Column));
			sprintf (buf, "varchar(250)" );
		}
	    }
	    attr[input].null_values = G_store ( db_get_string ( &sql ) );
	    attr[input].columns = G_store ( db_get_string ( &col_defs) );
		
  	    while (1) {
	       int cat ;
	       ATTR *at;

	       if(db_fetch (&cursor, DB_NEXT, &more) != DB_OK)
		   G_fatal_error ("Cannot fetch data");

	       if (!more) break;

	       db_set_string( &sql, "" );

	       for (i = 0; i < ncol; i++) {
		   
		   Column = db_get_table_column (Table, i);
		   sqltype = db_get_column_sqltype (Column);
		   ctype = db_sqltype_to_Ctype(sqltype);
		   Value  = db_get_column_value(Column);
	    

		   if ( G_strcasecmp ( db_get_column_name (Column), inFi->key)  == 0 ) {
		       cat = db_get_value_int ( Value );
		       G_debug ( 3, "cat = %d", cat );
		   }

		   db_append_string ( &sql, ", " ); 

		   db_convert_value_to_string( Value, sqltype, &value_string);
		   
		   G_debug ( 3, "%d: %s : %s", i, db_get_column_name (Column), db_get_string(&value_string));

		   switch ( ctype ) {
			case DB_C_TYPE_STRING:
			case DB_C_TYPE_DATETIME:
			    if ( db_test_value_isnull(Value) ) {
				db_append_string ( &sql, "null" );
			    } else {
				db_double_quote_string ( &value_string );
				sprintf (buf, "'%s'", db_get_string(&value_string) );
				db_append_string ( &sql, buf);
			    }
			    break;
			case DB_C_TYPE_INT:
			case DB_C_TYPE_DOUBLE:
			    if ( db_test_value_isnull(Value) ) {
				db_append_string ( &sql, "null" );
			    } else {
				db_append_string ( &sql, db_get_string(&value_string) );
			    }
			    break;
			default:
			    G_warning ( "Unknown column type (%s), values lost", db_get_column_name (Column) );
			    db_append_string ( &sql, "null" );
		   }
	       }

	       at = find_attr( &(attr[input]), cat );
	       if ( !at ) continue;
	       
	       /* if ( !at->used ) continue; */ /* We don't know yet */

	       at->values = G_store ( db_get_string ( &sql ) );
	       G_debug ( 3, "values: %s", at->values );
	    }

	    db_table_to_sql ( Table, &sql );
	
	    db_close_database_shutdown_driver ( in_driver );
	}
	
        fprintf (stderr, SEP );
    }

    if ( driver ) {
	sprintf ( buf, "create table %s (cat integer ", Fi->table );
	db_set_string ( &stmt, buf);
	
	if ( attr[0].columns )
	    db_append_string ( &stmt, attr[0].columns);
	else {
	    sprintf ( buf, ", a_cat integer" );
	    db_append_string ( &stmt, buf );
	}

	if ( attr[1].columns )
	    db_append_string ( &stmt, attr[1].columns);
	else {
	    sprintf ( buf, ", b_cat integer" );
	    db_append_string ( &stmt, buf );
	}

	db_append_string ( &stmt, " )" );
	
	G_debug ( 3, db_get_string ( &stmt ) );
	
	if (db_execute_immediate (driver, &stmt) != DB_OK ) { 
	    Vect_close (&Out);
	    db_close_database_shutdown_driver ( driver );
	    G_fatal_error ( "Cannot create table: %s", db_get_string (&stmt) );
	}

	if ( db_create_index2(driver, Fi->table, "cat" ) != DB_OK )
	    G_warning ( "Cannot create index" );

	if (db_grant_on_table (driver, Fi->table, DB_PRIV_SELECT, DB_GROUP|DB_PUBLIC ) != DB_OK )
	    G_fatal_error ( "Cannot grant privileges on table %s", Fi->table );

	/* Table created, now we can write dblink */
	Vect_map_add_dblink ( &Out, ofield[0], NULL, Fi->table, "cat", Fi->database, Fi->driver);
    }

    fprintf ( stderr, "Building partial topology ...\n" );
    /* do not print output, because befor cleaning it is nonsense */
    Vect_build_partial ( &Out, GV_BUILD_BASE, NULL ); 

    /* AREA x AREA */
    if ( type[0] == GV_AREA ) { 
	area_area ( In, field, &Out, Fi, driver, operator, ofield, attr );
    } else { /* LINE x AREA */
	line_area ( In, field, &Out, Fi, driver, operator, ofield, attr );
    }

    fprintf (stderr, SEP );
    fprintf ( stderr, "Rebuilding topology ...\n" );
    Vect_build_partial ( &Out, GV_BUILD_NONE, NULL );
    Vect_build (&Out, stderr); /* Build topology to show the final result and prepare for Vect_close() */

    if ( driver ) {
        /* Close table */
	db_commit_transaction ( driver );
	db_close_database_shutdown_driver ( driver );
    }
    
    Vect_close ( &(In[0]) );
    Vect_close ( &(In[1]) );
    Vect_close (&Out);

    G_done_msg("");
    exit (0);
}

