/***************************************************************************
 * Routines used to assist in command line parsing.  
 ***************************************************************************
 * G_define_flag()
 *
 * Returns a pointer to a flag structure.
 * Flags are always represented by single letters.  A user "turns them on"
 * at the command line using a minus sign followed by the character
 * representing the flag.
 *
 ***************************************************************************
 * G_define_option()
 *
 * Returns a pointer to a flag structure.
 * Options are provided by user on command line using the standard
 * format:  key=value
 * Options identified as REQUIRED must be specified by user on command line.
 * The option string can either specify a range of values (e.g. "10-100") or
 * a list of acceptable values (e.g. "red,orange,yellow").  Unless the option
 * string is NULL, user provided input will be evaluated agaist this string.
 *
 ***************************************************************************
 *
 * G_disable_interactive()
 *
 * Disables the ability of the parser to operate interactively.
 *
 ***************************************************************************
 *
 * G_parser(argc, argv)
 *    int argc ;
 *    char **argv ;
 *
 * Parses the command line provided through argc and argv.  Example:
 * Assume the previous calls:
 *
 *  opt1 = G_define_option() ;
 *  opt1->key        = "map",
 *  opt1->type       = TYPE_STRING,
 *  opt1->required   = YES,
 *  opt1->checker    = sub,
 *  opt1->description= "Name of an existing raster map" ;
 *
 *  opt2 = G_define_option() ;
 *  opt2->key        = "color",
 *  opt2->type       = TYPE_STRING,
 *  opt2->required   = NO,
 *  opt2->answer     = "white",
 *  opt2->options    = "red,orange,blue,white,black",
 *  opt2->description= "Color used to display the map" ;
 *
 *  opt3 = G_define_option() ;
 *  opt3->key        = "number",
 *  opt3->type       = TYPE_DOUBLE,
 *  opt3->required   = NO,
 *  opt3->answer     = "12345.67",
 *  opt3->options    = "0-99999",
 *  opt3->description= "Number to test parser" ;
 *
 * parser() will respond to the following command lines as described:
 *
 * command      (No command line arguments)
 *    Parser enters interactive mode.
 *
 * command map=map.name
 *    Parser will accept this line.  Map will be set to "map.name", the
 *    'a' and 'b' flags will remain off and the num option will be set
 *    to the default of 5.
 *
 * command -ab map=map.name num=9
 * command -a -b map=map.name num=9
 * command -ab map.name num=9
 * command map.name num=9 -ab
 * command num=9 -a map=map.name -b
 *    These are all treated as acceptable and identical. Both flags are
 *    set to on, the map option is "map.name" and the num option is "9".
 *    Note that the "map=" may be omitted from the command line if it
 *    is part of the first option (flags do not count).
 *
 * command num=12
 *    This command line is in error in two ways.  The user will be told 
 *    that the "map" option is required and also that the number 12 is
 *    out of range.  The acceptable range (or list) will be printed.
 *
 * On error, G_parser() prints call G_usage() and returns -1.
 * Otherwise returns 0
 *
 ***************************************************************************
 *
 * G_recreate_command()
 *
 * Creates a command-line that runs the current command completely
 * non-interactive
 *
 ***************************************************************************
*/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include "gis.h"

#define BAD_SYNTAX  1
#define OUT_OF_RANGE    2
#define MISSING_VALUE   3


static int interactive_ok = 1 ;
static int n_opts = 0 ;
static int n_flags = 0 ;

static struct Flag first_flag;    /* First flag in a linked list      */
static struct Flag *current_flag; /* Pointer for traversing list      */

static struct Option first_option ;
static struct Option *current_option ;

static struct GModule module_info; /* general information on the
									  corresponding module */

static char *pgm_name = NULL;

struct Item
{
	struct Option *option ;
	struct Flag *flag ;
	struct Item *next_item ;
} ;

static struct Item first_item ;
static struct Item *current_item ;
static int n_items = 0 ;
static int show_options(int ,char *);
static int show(char *,int);
static int set_flag (int);
static int contains (char *,int);
static int set_option( char *);
static int check_opts();
static int check_an_opt( char *, int , char *,char *);
static int check_int(char *, char *);
static int check_double( char *, char *);
static int check_string( char *, char *);
static int check_required();
static int split_opts();
static int check_multiple_opts();
static int interactive( char *);
static int interactive_flag( struct Flag *);
static int interactive_option( struct Option *);
static int gis_prompt( struct Option *, char *);

int 
G_disable_interactive (void)
{
	interactive_ok = 0 ;

        return 0;
}

struct Flag *
G_define_flag (void)
{
	struct Flag *flag ;
	struct Item *item ;

	/* Allocate memory if not the first flag */

	if (n_flags)
	{
		flag = (struct Flag *)G_malloc(sizeof(struct Flag)) ;
		current_flag->next_flag = flag ;
	}
	else
		flag = &first_flag ;

	/* Zero structure */

	G_zero ((char *) flag, sizeof(struct Flag));

	current_flag = flag ;
	n_flags++ ;

	if (n_items)
	{
		item = (struct Item *)G_malloc(sizeof(struct Item)) ;
		current_item->next_item = item ;
	}
	else
		item = &first_item ;

	G_zero ((char *) item, sizeof(struct Item));
	
	item->flag = flag ;
	item->option = NULL ;

	current_item = item ;
	n_items++ ;

	return(flag) ;
}

struct Option *
G_define_option (void)
{
	struct Option *opt ;
	struct Item *item ;

	/* Allocate memory if not the first option */

	if (n_opts)
	{
		opt = (struct Option *)G_malloc(sizeof(struct Option)) ;
		current_option->next_opt = opt ;
	}
	else
		opt = &first_option ;

	/* Zero structure */
	G_zero ((char *) opt, sizeof(struct Option));

	opt->required  = NO ;
	opt->multiple  = NO ;
	opt->answer    = NULL ;
	opt->answers   = NULL ;
	opt->def       = NULL ;
	opt->checker   = NULL ;
	opt->options   = NULL ;
	opt->key_desc  = NULL ;
	opt->gisprompt = NULL ;

	current_option = opt ;
	n_opts++ ;

	if (n_items)
	{
		item = (struct Item *)G_malloc(sizeof(struct Item)) ;
		current_item->next_item = item ;
	}
	else
		item = &first_item ;

	G_zero ((char *) item, sizeof(struct Item));
	
	item->option = opt ;
	item->flag = NULL ;

	current_item = item ;
	n_items++ ;

	return(opt) ;
}

struct GModule *
G_define_module (void)
{
	struct GModule *module ;

	/* Allocate memory */

	module = &module_info;

	/* Zero structure */

	G_zero ((char *) module, sizeof(struct GModule));

	return(module) ;
}

/* The main parsing routine */

/*
**  Returns  0 on success
**          -1 on error
**          
*/
int G_parser (int argc, char **argv)
{
	int need_first_opt ;
	int opt_checked = 0;
	int error ;
	char *ptr ;
	int i;
	struct Option *opt ;

	error = 0 ;
	need_first_opt = 1 ;
	i = strlen(pgm_name = argv[0]) ;
	while (--i >= 0)
	{
		if (pgm_name[i] == '/')
		{
			pgm_name += i+1;
			break;
		}
	}

	/* Stash default answers */

	opt= &first_option;
	while(opt != NULL)
	{
		if(opt->multiple && opt->answers && opt->answers[0])
		{
			opt->answer = (char *)G_malloc(strlen(opt->answers[0])+1);
			strcpy(opt->answer, opt->answers[0]);
			for(i=1; opt->answers[i]; i++)
			{
				opt->answer = (char *)G_realloc (opt->answer,
						strlen(opt->answer)+
						strlen(opt->answers[i])+2);
				strcat(opt->answer, ",");
				strcat(opt->answer, opt->answers[i]);
			}
		}
		opt->def = opt->answer ;
		opt = opt->next_opt ;
	}
	
	/* If there are NO arguments, go interactive */

	if (argc < 2 && interactive_ok && isatty(0) )
	{
	    if (getenv("GRASS_UI_TERM")) {
		interactive(argv[0]) ;
	        opt_checked = 1; 
		/* all options have been already checked interactively */
	    } else {
		G_gui();
		return -1;
	    }
	}
	else if (argc < 2 && isatty(0))
		{
			G_usage();
			return -1;
		}
	else if (argc >= 2)
	{

		/* If first arg is "help" give a usage/syntax message */
		if (strcmp(argv[1],"help") == 0 ||
			strcmp(argv[1], "-help") == 0 ||
			strcmp(argv[1], "--help") == 0)
		{
			G_usage();
			return -1;
		}

		/* If first arg is "--interface-description" then print out
		 * a xml description of the task */
		if (strcmp(argv[1],"--interface-description") == 0)
		{
			G_usage_xml();
			return -1;
		}

		/* Loop thru all command line arguments */

		while(--argc)
		{
			ptr = *(++argv) ;

			/* If we see a flag */
			if(*ptr == '-')
			{
				while(*(++ptr))
					error += set_flag(*ptr) ;

			}
			/* If we see standard option format (option=val) */
			else if (contains(ptr, '='))
			{
				error += set_option(ptr) ;
				need_first_opt = 0 ;
			}

			/* If we see the first option with no equal sign */
			else if (need_first_opt && n_opts)
			{
				first_option.answer = G_store(ptr) ;
				need_first_opt = 0 ;
			}

  	        /* If we see the non valid argument (no "=", just argument) */
			else if (contains(ptr, '=') == 0)
			{
				fprintf(stderr, "Sorry <%s> is not a valid option\n", ptr);
				error = 1;
			}

		}
	}

	/* Split options where multiple answers are OK */
	split_opts() ;

	/* Check multiple options */
	error += check_multiple_opts() ;

	/* Check answers against options and check subroutines */
	if(!opt_checked)
	   error += check_opts() ;

	/* Make sure all required options are set */
	error += check_required() ;

	if(error)
	{
		G_usage();
		return -1;
	}
	return(0) ;
}

int G_usage (void)
{
	struct Option *opt ;
	struct Flag *flag ;
	char item[256];
	char *key_desc;
	int maxlen;
	int len, n;
	
	if (!pgm_name)		/* v.dave && r.michael */
	    pgm_name = G_program_name ();
	if (!pgm_name)
	    pgm_name = "??";

	if (module_info.description) {
		fprintf (stderr, "\nDescription:\n");
		fprintf (stderr, " %s\n", module_info.description);
	}

	fprintf (stderr, "\nUsage:\n ");

	len = show(pgm_name,1);

	/* Print flags */

	if(n_flags)
	{
		item[0] = ' ';
		item[1] = '[';
		item[2] = '-';
		flag= &first_flag;
		for(n = 3; flag != NULL; n++, flag = flag->next_flag)
			item[n] = flag->key;
		item[n++] = ']';
		item[n] = 0;
		len=show(item,len);
	}

	maxlen = 0;
	if(n_opts)
	{
		opt= &first_option;
		while(opt != NULL)
		{
			if (opt->key_desc != NULL)
				key_desc = opt->key_desc;
			else if (opt->type == TYPE_STRING)
				key_desc = "name";
			else
				key_desc = "value";

			n = strlen (opt->key);
			if (n > maxlen) maxlen = n;

			strcpy(item," ");
			if(!opt->required )
				strcat (item, "[");
			strcat (item, opt->key);
			strcat (item, "=");
			strcat (item, key_desc);
			if (opt->multiple)
			{
				strcat(item,"[,");
				strcat(item,key_desc);
				strcat(item,",...]");
			}
			if(!opt->required )
				strcat(item,"]") ;

			len = show(item,len);

			opt = opt->next_opt ;
		}
	}
	fprintf (stderr, "\n");

	/* Print help info for flags */

	if(n_flags)
	{
		fprintf (stderr, "\nFlags:\n");
		flag= &first_flag;
		while(flag != NULL)
		{
			fprintf(stderr,"  -%c   %s\n",
			    flag->key, flag->description) ;
			flag = flag->next_flag ;
		}
	}

	/* Print help info for options */

	if(n_opts)
	{
		fprintf (stderr, "\nParameters:\n");
		opt= &first_option;
		while(opt != NULL)
		{
			fprintf (stderr, "  %*s   %s\n", maxlen, opt->key,
			    opt->description);
			if(opt->options)
				show_options(maxlen, opt->options) ;
				/*
				fprintf (stderr, "  %*s   options: %s\n", maxlen, " ",
					opt->options) ;
				*/
			if(opt->def)
				fprintf (stderr, "  %*s   default: %s\n", maxlen, " ",
					opt->def) ;
			opt = opt->next_opt ;
		}
	}

        return 0;
}

void print_escaped_for_xml (FILE * fp, char * str) {
	for (;*str;str++) {
		switch (*str) {
			case '&':
				fputs("&amp;", fp);
				break;
			case '<':
				fputs("&lt;", fp);
				break;
			case '>':
				fputs("&gt;", fp);
				break;
			default:
				fputc(*str, fp);
		}
	}
}

int G_usage_xml (void)
{
	struct Option *opt ;
	struct Flag *flag ;
	char *type;
	char *s, *top;
	int i;
	
	if (!pgm_name)		/* v.dave && r.michael */
	    pgm_name = G_program_name ();
	if (!pgm_name)
	    pgm_name = "??";

	fprintf(stdout, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(stdout, "<!DOCTYPE task SYSTEM \"grass-interface.dtd\">\n");

	fprintf(stdout, "<task name=\"%s\">\n", pgm_name);  

	if (module_info.description) {
		fprintf(stdout, "\t<description>\n\t\t");
		print_escaped_for_xml (stdout, module_info.description);
		fprintf(stdout, "\n\t</description>\n");
	}

	/***** Don't use parameter-groups for now.  We'll reimplement this later 
	 ***** when we have a concept of several mutually exclusive option
	 ***** groups
	if (n_opts || n_flags)
		fprintf(stdout, "\t<parameter-group>\n");
	 *****
	 *****
	 *****/
	
	if(n_opts)
	{
		opt= &first_option;
		while(opt != NULL)
		{
			/* TODO: make this a enumeration type? */
			switch (opt->type) {
				case TYPE_INTEGER:
					type = "integer";
					break ;
				case TYPE_DOUBLE:
					type = "float";
					break ;
				case TYPE_STRING:
					type = "string";
					break ;
				default:
					type = "string";
					break;
			}
			fprintf (stdout, "\t<parameter "
				"name=\"%s\" "
				"type=\"%s\" "
				"required=\"%s\" "
				"multiple=\"%s\">\n",
				opt->key,
				type,
				opt->required == YES ? "yes" : "no",
				opt->multiple == YES ? "yes" : "no");

			if (opt->description) {
				fprintf(stdout, "\t\t<description>\n\t\t\t");
				print_escaped_for_xml(stdout, opt->description);
				fprintf(stdout, "\n\t\t</description>\n");
			}

			if (opt->key_desc)
			{
				fprintf (stdout, "\t\t<keydesc>\n");
				top = G_calloc (strlen (opt->key_desc) + 1, 1);
				strcpy (top, opt->key_desc);
				s = strtok (top, ",");
				for (i = 1; s != NULL; i++)
				{
					fprintf (stdout, "\t\t\t<item order=\"%d\">", i);
					print_escaped_for_xml (stdout, s);
					fprintf (stdout, "</item>\n");
					s = strtok (NULL, ",");
				}
				fprintf (stdout, "\t\t</keydesc>\n");
				G_free (top);
			}
			
			if (opt->gisprompt)
			{
				const char *atts[] = {"age", "element", "prompt", NULL};
				top = G_calloc (strlen (opt->gisprompt) + 1, 1);
				strcpy (top, opt->gisprompt);
				s = strtok (top, ",");
				fprintf (stdout, "\t\t<gisprompt ");
				for (i = 0; s != NULL && atts[i] != NULL; i++)
				{
					fprintf (stdout, "%s=\"%s\" ", atts[i], s);
					s = strtok (NULL, ",");
				}
				fprintf (stdout, "/>\n");
				G_free (top);
			}

			if(opt->def) {
				fprintf(stdout, "\t\t\t<default>\n\t\t\t");
				print_escaped_for_xml(stdout, opt->def);
				fprintf(stdout, "\n\t\t\t</default>\n");
			}

			if(opt->options) {
				fprintf(stdout, "\t\t<values>\n");
				top = G_calloc(strlen(opt->options) + 1,1);
				strcpy(top, opt->options);
				s = strtok(top, ",");
				while (s) {
					fprintf(stdout, "\t\t\t<value>");
					print_escaped_for_xml(stdout, s);
					fprintf(stdout, "</value>\n");
					s = strtok(NULL, ",");
				}
				fprintf(stdout, "\t\t</values>\n");
				G_free (top);
			}

			/* TODO:
			 * add something like
			 * 	 <range min="xxx" max="xxx"/>
			 * to <values>
			 * - key_desc?
			 * - there surely are some more. which ones?
			 */

			opt = opt->next_opt ;
			fprintf (stdout, "\t</parameter>\n");
		}
	}

	
	if(n_flags)
	{
		flag= &first_flag;
		while(flag != NULL)
		{
			fprintf (stdout, "\t<flag name=\"%c\">\n", flag->key);

			if (flag->description) {
				fprintf(stdout, "\t\t<description>\n\t\t\t");
				print_escaped_for_xml(stdout, flag->description);
				fprintf(stdout, "\n\t\t</description>\n");
			}
			flag = flag->next_flag ;
			fprintf (stdout, "\t</flag>\n");
		}
	}

	/***** Don't use parameter-groups for now.  We'll reimplement this later 
	 ***** when we have a concept of several mutually exclusive option
	 ***** groups
	if (n_opts || n_flags)
		fprintf(stdout, "\t</parameter-group>\n");
	 *****
	 *****
	 *****/

	fprintf(stdout, "</task>\n");
    return 0;
}

/* Add to cmd */
int append (char *cmd, char *frm, ...)
{
    char buffer[5000];
    va_list ap;
	     

    va_start(ap,frm);
    vsprintf(buffer,frm,ap);
    va_end(ap);
    
    strcat (cmd, buffer);

    return 1;
}

/* Build gui */
int G_gui (void)
{
    char *cmd, *cmd2;
    char buf[1000];
    char wish[200], def[200];
    struct Option *opt ;
    struct Flag *flag ;
    char *type;
    char *s, *top, *p1, *p2;
    int i, optn, n_options;
    int cmdl = 100000;
    
    cmd = G_calloc( cmdl, 1);
    cmd2 = G_calloc( cmdl, 1);
		
    if (!pgm_name)
	pgm_name = G_program_name ();
    if (!pgm_name)
	pgm_name = "??";
    
    strcpy (wish, getenv("GRASS_WISH"));
    
    cmd[0] = '\0';
    append(cmd, "lappend auto_path %s/bwidget\n", G_gisbase());
    append(cmd, "package require BWidget\n");
    append(cmd, "wm title . \"%s\"\n", pgm_name);

    /* Selection window */
    append(cmd,"proc list_select_item { item } {
		    global list_select_item
		    set list_select_item $item
		    .list.sw.lb selection set $item
		    destroy .list
		}\n" );
    append(cmd,"proc list_select { list } {
		    global list_select_item
		    set list_select_item \"\"
		    toplevel .list
		    wm title .list \"Select item\"
		    regexp -- {(.+)x(.+)([+-].+)([+-].+)} [wm geometry .] g w h x y 
		    set w [expr int($w/3)]
		    wm geometry .list ${w}x$h$x$y 
		    set sw [ScrolledWindow .list.sw]
		    set lb [ListBox $sw.lb -width 10 -padx 0]
		    $sw setwidget $lb
		    $lb bindText <ButtonPress-1> list_select_item
		    pack $sw -fill both -expand yes
		    frame .list.buttons
		    pack .list.buttons -side bottom -fill x
		    button .list.buttons.cancel -text Cancel -command {
			set list_select_item \"\"
			destroy .list
		    }
		    pack .list.buttons.cancel -side left -expand yes
		    foreach i $list { 
			$lb insert end [lindex $i 0] -text $i
		    }
		    tkwait window .list
		    return $list_select_item
		}\n" );
    append(cmd,"proc element_list { element } {
		    global env
		    set pwd [pwd]
		    set inpath 1
		    set list \"\"
		    cd $env(GISDBASE)/$env(LOCATION_NAME)
		    foreach dir [exec g.mapsets -p] {
			if {[string compare $dir .] == 0} {
			    set inpath 0
			    continue
			}  
			if [info exists dirstat($dir)] continue
			set dirstat($dir) 1
			if {[catch {eval eval cd $env(GISDBASE)/$env(LOCATION_NAME)/$dir/$element}]} {
			    if {0 && $dir == $env(MAPSET)} {
				tk_messageBox -message \"$typ directory\n'[subst [subst $element]]'\nnon-existent or unusable\" \
				-type ok
			    } 
			} elseif {[catch {glob *} names]} {
			} else {
			    if {$dir == $env(MAPSET)} {
				eval lappend list [lsort $names]
			    } else {
				foreach name [lsort $names] {
				    lappend list \"$name@$dir\"
				}
			    }    
			}
		    }
		    cd $pwd
		    return $list
		}\n" );

    append(cmd, "set pw [PanedWindow .pw -side left ]\n");
    append(cmd, "set optpane [$pw add -minsize 50]\n");
    append(cmd, "set outpane [$pw add -minsize 30]\n");
    
    append(cmd, "set optwin [ScrolledWindow $optpane.optwin -relief sunken -borderwidth 2]\n");
    append(cmd, "set optfra [ScrollableFrame $optwin.fra -height 200 ]\n");
    append(cmd, "$optwin setwidget $optfra\n");
    append(cmd, "set suf [$optfra getframe]\n");
    append(cmd, "pack $optwin -fill both -expand yes\n");
    append(cmd, "pack $optpane $outpane -fill both -expand yes\n");
    append(cmd, "pack $pw -fill both -expand yes\n");
    
    /* Output text frame */ 
    append(cmd, "set outwin [ScrolledWindow $outpane.win -relief sunken -borderwidth 2]\n");
    append(cmd, "set outtext [text $outwin.text -height 5 -width 30] \n" );
    append(cmd, "$outwin setwidget $outtext\n");
    append(cmd, "pack $outwin $outtext -expand yes -fill both\n");

    if(n_opts)
    {
	optn = 1;
	opt= &first_option;
	while(opt != NULL)
	{
	    switch (opt->type) {
		    case TYPE_INTEGER:
			    type = "integer";
			    break ;
		    case TYPE_DOUBLE:
			    type = "float";
			    break ;
		    case TYPE_STRING:
			    type = "string";
			    break ;
		    default:
			    type = "string";
			    break;
	    }
	    
	    /* Set key name and type */
	    append(cmd, "set optname(%d) \"%s\" \n", optn, opt->key);
	    if(opt->multiple && opt->options)
	        append(cmd, "set opttype(%d) \"multi\" \n");
	    else
	        append(cmd, "set opttype(%d) \"opt\" \n");

	    /* Option label */ 
	    append(cmd, "label $suf.lab%d -text \"%s (%s, %s):\" -anchor w -justify left\n", 
		          optn, opt->description, 
			  type, opt->required == YES ? "required" : "optional");
	    append(cmd, "pack $suf.lab%d -side top -fill x\n", optn );

	    
	    /* Option value */
	    append(cmd, "frame $suf.val%d \n", optn );
	    if(opt->options) {
	        if(!opt->multiple) {
		    append(cmd, "ComboBox $suf.val%d.val -underline 0 -labelwidth 0 -width 25 -textvariable optval(%d) -values { ", optn, optn );
		}
		    
		top = G_calloc(strlen(opt->options) + 1,1);
		strcpy(top, opt->options);
		s = strtok(top, ",");
		p1 = s;
		i = 1;
		while (s) {
		    if(opt->multiple) {
			append(cmd, "checkbutton $suf.val%d.val%d -text \"%s\" -variable optval(%d,%d) -onvalue 1 -offvalue 0\n", optn, i, s, optn, i );    
			append(cmd, "pack $suf.val%d.val%d -side left\n", optn, i);
			append(cmd, "set optvalname(%d,%d) \"%s\" \n", optn, i, s);    
		    } else {
			append(cmd, " \"%s\" ", s );
		    }
		    s = strtok(NULL, ",");
		    i++;
		}
	        if(!opt->multiple) {
		    append(cmd, " } \n");
		    append(cmd, "pack $suf.val%d.val -side left\n", optn);
		    append(cmd, " set optval(%d) \"%s\" \n", optn, p1 );
		}
		append(cmd, "set nmulti(%d) %d \n", optn, i - 1 );
		G_free (top);
	    } else {
		if ( opt->gisprompt ) {
		    if ( !strncmp(opt->gisprompt, "old", 3) ) {
		        strcpy(buf, opt->gisprompt);
		        s = strtok(buf, ",");
		        s = strtok(NULL, ",");
			append(cmd, "button $suf.val%d.sel -text \">\" -command {
				   set lst [element_list \"%s\" ]
				   set val [list_select $lst]
				   if { [string length $val] > 0 } {
                                       set optval(%d) $val
				   } 
			       }\n", optn, s, optn );
	                append(cmd, "pack $suf.val%d.sel -side left -fill x\n", optn);
		    }
		}
	        append(cmd, "Entry $suf.val%d.val -textvariable optval(%d)\n", optn, optn );
	        append(cmd, "pack $suf.val%d.val -side left -fill x -expand yes\n", optn);
		if(opt->def) {
		    append(cmd, " set optval(%d) \"%s\" \n", optn, opt->def );
		} 
	    }
	    append(cmd, "pack $suf.val%d -side top -fill x\n", optn );

	    opt = opt->next_opt ;
	    optn++;
	}
    }

    if(n_flags)
    {
	flag= &first_flag;
	while(flag != NULL)
	{
	    append(cmd, "set opttype(%d) \"flag\" \n", optn);
	    
	    append(cmd, "frame $suf.val%d \n", optn );
	    append(cmd, "checkbutton $suf.val%d.chk -text \"%s\" -variable optval(%d) -onvalue 1 -offvalue 0 -anchor w\n", optn, flag->description, optn );    
	    append(cmd, "pack $suf.val%d.chk -side left\n", optn);
	    append(cmd, "set optname(%d) \"%c\" \n", optn, flag->key);    
	    append(cmd, "pack $suf.val%d -side top -fill x\n", optn );
	    flag = flag->next_flag ;
	    optn++;
	}
    }
   
    n_options = optn - 1; 
    append(cmd, "set nopt %d\n", n_options);
    
    /* Command construction */
    append(cmd, "proc mkcmd { } {
	             global optname optval opttype nmulti optvalname
	             set cmd \"%s\"
		     for {set i 1} {$i <= %d } {incr i} {
			 if { $opttype($i) == \"multi\" } {
			     set domulti 0
		             for {set j 1} {$j <= $nmulti($i) } {incr j} {
			         if { $optval($i,$j) == 1 } {
				     set domulti 1
				 }
			     }
			     if { $domulti == 1 } {
                                 append cmd \" $optname($i)=\"
				 set first 1
				 for {set j 1} {$j <= $nmulti($i) } {incr j} {
				     if { $optval($i,$j) == 1 } {
				         if { $first == 1 } {
					     set first 0 
					 } else {
					     append cmd \",\" 
					 }
					 append cmd \"$optvalname($i,$j)\"
				     }
				 }
			     }
			 } 
			 if { $opttype($i) == \"opt\" } {
			     if {[string length $optval($i)] > 0} { 
                                 append cmd \" $optname($i)=$optval($i)\"
			     }
			 }
			 if { $opttype($i) == \"flag\" } {
			     if { $optval($i) == 1 } {
                                 append cmd \" -$optname($i)\"
			     }
		         }
		     }
		     return $cmd
	         }\n", pgm_name, n_options );
	    
    /* Run button */
    append(cmd, "proc prnout { fh } {
	             global outtext
		     if [eof $fh] {
			 close $fh
		     } else {
			 set str [ read $fh ]
			 $outtext insert end $str
			 $outtext yview end
		     }
	         }\n");
    append(cmd, "button .run -text \"Run\" -command {
	       global outtext pipe
	       set cmd [ mkcmd ]
               $outtext insert end  \"\\n$cmd\\n\"
               $outtext yview end
	       set cmd \"| $cmd 2>@ stdout\"
               catch {open $cmd r} msg
               fconfigure $msg -blocking 0 
               fileevent $msg readable [ list prnout $msg  ]
               update idletasks
	   }\n");
    append(cmd, "pack .run -side top -padx 20 -pady 5\n");

    G_debug (1, "cmd:\n%s", cmd);  
    
    /* Add '\' before " and $ */
    p1 = cmd; p2 = cmd2;
    while ( *p1 != 0 ) {
	if ( (*p1 == '"') || (*p1 == '$')) { *p2 = '\\' ; p2++; }
	*p2 = *p1; p2++; p1++;
    }
    *p2 = '\0';
/*	
    fprintf(stdout, "cmd:\n%s", cmd);  
    fprintf(stdout, "cmd2:\n%s", cmd2);  
    sprintf(buf, "echo \"%s\"", cmd2);
    system ( buf );
*/
    sprintf(cmd, "echo \"%s\" | %s &", cmd2, wish);
    system (cmd);

    return 0;
}
/**************************************************************************
 *
 * The remaining routines are all local (static) routines used to support
 * the parsing process.
 *
 **************************************************************************/

static int show_options(int maxlen,char *str)
{
	char buff[1024] ;
	char *p1, *p2 ;
	int totlen, len ;

	strcpy(buff, str) ;
	fprintf (stderr, "  %*s   options: ", maxlen, " ") ;
	totlen = maxlen + 13 ;
	p1 = buff ;
	while(p2 = G_index(p1, ','))
	{
		*p2 = '\0' ;
		len = strlen(p1) + 1 ;
		if ((len + totlen) > 76)
		{
			totlen = maxlen + 13 ;
			fprintf(stderr, "\n %*s", maxlen + 13, " ") ;
		}
		fprintf (stderr, "%s,",  p1) ;
		totlen += len ;
		p1 = p2 + 1 ;
	}
	len = strlen(p1) ;
	if ((len + totlen) > 76 )
		fprintf(stderr, "\n %*s", maxlen + 13, " ") ;
	fprintf (stderr, "%s\n",  p1) ;

        return 0;
}

static int show (char *item, int len)
{
	int n;

	n = strlen (item)+(len>0);
	if (n + len > 76)
	{
		if (len)
			fprintf (stderr, "\n  ");
		len = 0;
	}
	fprintf (stderr, "%s", item);
	return n+len;
}

static int set_flag (int f)
{
	struct Flag *flag ;

	/* Flag is not valid if there are no flags to set */

	if(!n_flags)
	{
		fprintf(stderr,"Sorry, <%c> is not a valid flag\n", f) ;
		return(1) ;
	}

	/* Find flag with corrrect keyword */

	flag= &first_flag;
	while(flag != NULL)
	{
		if( flag->key == f)
		{
			flag->answer = 1 ;
			return(0) ;
		}
		flag = flag->next_flag ;
	}

	fprintf(stderr,"Sorry, <%c> is not a valid flag\n", f) ;
	return(1) ;
}

/* contents() is used to find things strings with characters like commas and
 * dashes.
 */
static int contains (char *s, int c)
{
	while(*s)
	{
		if(*s == c)
			return(1) ;
		s++ ;
	}
	return(0) ;
}

static int set_option (char *string)
{
	struct Option *at_opt ;
	struct Option *opt ;
	int got_one ;
	int key_len ;
	char the_key[64] ;
	char *ptr ;

	for(ptr=the_key; *string!='='; ptr++, string++)
		*ptr = *string ;
	*ptr = '\0' ;
	string++ ;

	/* Find option with best keyword match */
	got_one = 0 ;
	key_len = strlen(the_key) ;
	for(at_opt= &first_option; at_opt != NULL; at_opt=at_opt->next_opt)
	{
		if (strncmp(the_key,at_opt->key,key_len))
			continue ;

		got_one++;
		opt = at_opt ;

		/* changed 1/15/91 -dpg   old code is in parser.old */
		/* overide ambiguous check, if we get an exact match */
		if (strlen (at_opt->key) == key_len) 	
		{
		    opt = at_opt;
		    got_one = 1;
		    break;
		}
	}

	if (got_one > 1)
	{
		fprintf(stderr,"Sorry, <%s=> is ambiguous\n", the_key) ;
		return(1) ;
	}

	/* If there is no match, complain */
	if(got_one == 0)
	{
		fprintf(stderr,"Sorry, <%s> is not a valid parameter\n",
			the_key) ;
		return(1) ;
	}
		
	/* Allocate memory where answer is stored */
	if (opt->count++)
	{
		opt->answer = (char *)G_realloc (opt->answer,
			strlen (opt->answer)+strlen(string)+2);
		strcat (opt->answer, ",");
		strcat (opt->answer, string);
	}
	else
		opt->answer = G_store(string) ;
	return(0) ;
}

static int check_opts (void)
{
	struct Option *opt ;
	int error ;
	int ans ;

	error = 0 ;

	if(! n_opts)
		return(0) ;

	opt= &first_option;
	while(opt != NULL)
	{
		/* Check answer against options if any */

		if(opt->options && opt->answer)
		{
			if(opt->multiple == 0)
				error += check_an_opt(opt->key, opt->type,
				    opt->options, opt->answer) ;
			else
			{
				for(ans=0; opt->answers[ans] != '\0'; ans++)
					error += check_an_opt(opt->key, opt->type,
					    opt->options, opt->answers[ans]) ;
			}
		}

		/* Check answer against user's check subroutine if any */

		if(opt->checker)
			error += opt->checker(opt->answer) ;

		opt = opt->next_opt ;
	}
	return(error) ;
}

static int check_an_opt (char *key, int type, char *options, char *answer)
{
	int error ;

	error = 0 ;

	switch(type)
	{
	case TYPE_INTEGER:
		error = check_int(answer,options) ;
		break ;
	case TYPE_DOUBLE:
		error = check_double(answer,options) ;
		break ;
	case TYPE_STRING:
		error = check_string(answer,options) ;
		break ;
/*
	case TYPE_COORDINATE:
		error = check_coor(answer,options) ;
		break ;
*/
	}
	switch(error)
	{
	case 0:
		break ;
	case BAD_SYNTAX:
		fprintf(stderr,"\nError: illegal range syntax for parameter <%s>\n",
		    key) ;
		fprintf(stderr,"       Presented as: %s\n", options) ;
		break ;
	case OUT_OF_RANGE:
		fprintf(stderr,"\nError: value <%s> out of range for parameter <%s>\n",
		    answer, key) ;
		fprintf(stderr,"       Legal range: %s\n", options) ;
		break ;
	case MISSING_VALUE:
		fprintf(stderr,"\nError: Missing value for parameter <%s>\n",
		    key) ;
	}
	return(error) ;
}

static int check_int (char *ans, char *opts)
{
	int d, lo, hi;

	if (1 != sscanf(ans,"%d", &d))
		return(MISSING_VALUE) ;

	if (contains(opts, '-'))
	{
		if (2 != sscanf(opts,"%d-%d",&lo, &hi))
			return(BAD_SYNTAX) ;
		if (d < lo || d > hi)
			return(OUT_OF_RANGE) ;
		else
			return(0) ;
	}
	else if (contains(opts, ','))
	{
		for(;;)
		{
			if (1 != sscanf(opts,"%d",&lo))
				return(BAD_SYNTAX) ;
			if (d == lo)
				return(0) ;
			while(*opts != '\0' && *opts != ',')
				opts++ ;
			if (*opts == '\0')
				return(OUT_OF_RANGE) ;
			if (*(++opts) == '\0')
				return(OUT_OF_RANGE) ;
		}
	}
	else
	{
		if (1 != sscanf(opts,"%d",&lo))
			return(BAD_SYNTAX) ;
		if (d == lo)
			return(0) ;
		return(OUT_OF_RANGE) ;
	}
}

/*
static int
check_coor(ans, opts)
char *ans ;
char *opts ;
{
	double xd, xlo, xhi;
	double yd, ylo, yhi;

	if (1 != sscanf(ans,"%lf,%lf", &xd, &yd))
		return(MISSING_VALUE) ;

	if (contains(opts, '-'))
	{
		if (2 != sscanf(opts,"%lf-%lf,%lf-%lf",&xlo, &xhi, &ylo, &yhi))
			return(BAD_SYNTAX) ;
		if (xd < xlo || xd > xhi)
			return(OUT_OF_RANGE) ;
		if (yd < ylo || yd > yhi)
			return(OUT_OF_RANGE) ;
		return(0) ;
	}
	return(BAD_SYNTAX) ;
}
*/

static int check_double (char *ans, char *opts)
{
	double d, lo, hi;

	if (1 != sscanf(ans,"%lf", &d))
		return(MISSING_VALUE) ;

	if (contains(opts, '-'))
	{
		if (2 != sscanf(opts,"%lf-%lf",&lo, &hi))
			return(BAD_SYNTAX) ;
		if (d < lo || d > hi)
			return(OUT_OF_RANGE) ;
		else
			return(0) ;
	}
	else if (contains(opts, ','))
	{
		for(;;)
		{
			if (1 != sscanf(opts,"%lf",&lo))
				return(BAD_SYNTAX) ;
			if (d == lo)
				return(0) ;
			while(*opts != '\0' && *opts != ',')
				opts++ ;
			if (*opts == '\0')
				return(OUT_OF_RANGE) ;
			if (*(++opts) == '\0')
				return(OUT_OF_RANGE) ;
		}
	}
	else
	{
		if (1 != sscanf(opts,"%lf",&lo))
			return(BAD_SYNTAX) ;
		if (d == lo)
			return(0) ;
		return(OUT_OF_RANGE) ;
	}
}

static int check_string (char *ans, char *opts)
{
	if (*opts == '\0')
		return(0) ;

	if (contains(opts, ','))
	{
		for(;;)
		{
			if ((! strncmp(ans, opts, strlen(ans)))
			    && ( *(opts+strlen(ans)) == ','
			       ||  *(opts+strlen(ans)) == '\0'))
				return(0) ;
			while(*opts != '\0' && *opts != ',')
				opts++ ;
			if (*opts == '\0')
				return(OUT_OF_RANGE) ;
			if (*(++opts) == '\0')
				return(OUT_OF_RANGE) ;
		}
	}
	else
	{
		if (! strcmp(ans, opts))
			return(0) ;
		return(OUT_OF_RANGE) ;
	}
}

static int check_required (void)
{
	struct Option *opt ;
	int err ;

	err = 0 ;

	if(! n_opts)
		return(0) ;

	opt= &first_option;
	while(opt != NULL)
	{
		if(opt->required && opt->answer == NULL)
		{
			fprintf(stderr,"\nERROR: Required parameter <%s> not set:\n    (%s).\n",
			    opt->key, opt->description) ;
			err++ ;
		}
		opt = opt->next_opt ;
	}

	return(err) ;
}

static int split_opts (void)
{
	struct Option *opt ;
	char *ptr1 ;
	char *ptr2 ;
	int allocated ;
	int ans_num ;
	int len ;


	if(! n_opts)
		return 0;

	opt= &first_option;
	while(opt != NULL)
	{
		if (/*opt->multiple && */(opt->answer != NULL))
		{
			/* Allocate some memory to store array of pointers */
			allocated = 10 ;
			opt->answers = (char **)G_malloc(allocated * sizeof(char *)) ;

			ans_num = 0 ;
			ptr1 = opt->answer ;
			opt->answers[ans_num] = NULL ;

			for(;;)
			{
				for(len=0, ptr2=ptr1; *ptr2 != '\0' && *ptr2 != ','; ptr2++, len++)
					;

				if (len > 0)        /* skip ,, */
				{
					opt->answers[ans_num]=(char *)G_malloc(len+1) ;
					G_copy(opt->answers[ans_num], ptr1, len) ;
					opt->answers[ans_num][len] = 0;

					ans_num++ ;

					if(ans_num >= allocated)
					{
						allocated += 10 ;
						opt->answers =
						    (char **)G_realloc((char *)opt->answers,
						    allocated * sizeof(char *)) ;
					}

					opt->answers[ans_num] = NULL ;
				}

				if(*ptr2 == '\0')
					break ;

				ptr1 = ptr2+1 ;

				if(*ptr1 == '\0')
					break ;
			}
		}
		opt = opt->next_opt ;
	}

	return 0;
}

static int check_multiple_opts (void)
{
	struct Option *opt ;
	char *ptr ;
	int n_commas ;
	int n ;
	int error ;

	if(! n_opts)
		return (0) ;

	error = 0 ;
	opt= &first_option;
	while(opt != NULL)
	{
		if ((opt->answer != NULL) && (opt->key_desc != NULL))
		{
			/* count commas */
			n_commas = 1 ;
			for(ptr=opt->key_desc; *ptr!='\0'; ptr++)
				if (*ptr == ',')
					n_commas++ ;
			/* count items */
			for(n=0;opt->answers[n] != '\0';n++)
				;
			/* if not correct multiple of items */
			if(n % n_commas)
			{
				fprintf(stderr,"\nError: option <%s> must be provided in multiples of %d\n",
					opt->key, n_commas) ;
				fprintf(stderr,"       You provided %d items:\n", n) ;
				fprintf(stderr,"       %s\n", opt->answer) ;
				error++ ;
			}
		}
		opt = opt->next_opt ;
	}
	return(error) ;
}

static int interactive( char *command)
{
	struct Item *item ;

	/* Query for flags */

	if(!n_items)
	{
		fprintf(stderr,"Programmer error: no flags or options\n") ;
		exit(-1) ;
	}

	for (item= &first_item ;;)
	{
		if (item->flag)
			interactive_flag(item->flag) ;
		else if (item->option)
			interactive_option(item->option) ;
		else
			break ;

		item=item->next_item ;

		if (item == NULL)
			break ;
	}

	return 0;
}

static int interactive_flag( struct Flag *flag )
{
	char buff[1024] ;
	fprintf(stderr, "\nFLAG: Set the following flag?\n") ;
	sprintf(buff,"    %s?", flag->description) ;
	flag->answer = G_yes(buff, 0) ;

	return 0;
}

static int interactive_option(struct Option *opt )
{
	char buff[1024],*bptr ;
	char buff2[1024] ;
	int set_one ;

	fprintf(stderr,"\nOPTION:   %s\n", opt->description) ;
	fprintf(stderr,"     key: %s\n", opt->key) ;
	if (opt->key_desc)
	fprintf(stderr,"  format: %s\n", opt->key_desc) ;
	if (opt->def)
	fprintf(stderr," default: %s\n", opt->def) ;
	fprintf(stderr,"required: %s\n", opt->required ? "YES" : "NO") ;
	if (opt->multiple)
	fprintf(stderr,"multiple: %s\n", opt->multiple ? "YES" : "NO") ;
	if (opt->options)
	fprintf(stderr," options: %s\n", opt->options) ;
	/*
	show_options(0, opt->options) ;
	*/

	set_one = 0 ;
	for(;;)
	{
	   *buff='\0' ;
	   if(opt->gisprompt)
		gis_prompt(opt, buff) ;
	   else
	   {
		fprintf(stderr,"enter option > ") ;
		if(fgets(buff,1024,stdin) == 0) exit(1); ;
                bptr = buff;  /* strip newline  */
                while(*bptr) {if(*bptr=='\n') *bptr='\0'; bptr++;}

	   }

	   if(strlen(buff) != 0)
	   {
	        if(opt->options)
		/* then check option */
	        {
		    if (check_an_opt(opt->key, opt->type, opt->options, buff))
	            {
		        if (G_yes("   Try again? ", 1))
		    		continue ;
	    	        else
				exit(-1) ;
		    }
		}
		if (opt->checker)
	 	    if (opt->checker(buff))
		    {
		    	    fprintf(stderr,"Sorry, %s is not accepted.\n", buff) ;
			    *buff = '\0' ;
			    if (G_yes("   Try again? ", 1))
			    	continue ;
			    else
				exit(-1) ;
		    }

		sprintf(buff2,"%s=%s", opt->key, buff) ;
		if(! opt->gisprompt)
		{
			fprintf(stderr,"\nYou have chosen:\n  %s\n", buff2) ;
			if (G_yes("Is this correct? ", 1))
			{
				set_option(buff2) ;
				set_one++ ;
			}
		}
		else 
		{
			set_option(buff2) ;
			set_one++ ;
		}
	   } /* if strlen(buf ) !=0 */

	   if ((strlen(buff) == 0) && opt->required && (set_one == 0))
		exit(-1) ;
	   if ((strlen(buff) == 0) && (set_one > 0) && opt->multiple )
		break ;
	   if ((strlen(buff) == 0) && !opt->required)
		break ;
	   if ((set_one == 1) && !opt->multiple)
		break ;
	}
	return(0) ;
}

static int gis_prompt (struct Option *opt, char *buff)
{
	char age[64] ;
	char element[64] ;
	char desc[64] ;
	char *ptr1, *ptr2 ;

	for(ptr1=opt->gisprompt,ptr2=age; *ptr1!='\0'; ptr1++, ptr2++)
	{
		if (*ptr1 == ',')
			break ;
		*ptr2 = *ptr1 ;
	}
	*ptr2 = '\0' ;

	for(ptr1++, ptr2=element; *ptr1!='\0'; ptr1++, ptr2++)
	{
		if (*ptr1 == ',')
			break ;
		*ptr2 = *ptr1 ;
	}
	*ptr2 = '\0' ;

	for(ptr1++, ptr2=desc; *ptr1!='\0'; ptr1++, ptr2++)
	{
		if (*ptr1 == ',')
			break ;
		*ptr2 = *ptr1 ;
	}
	*ptr2 = '\0' ;
	/*********ptr1 points to current mapset description***********/

	if (opt->answer)
		G_set_ask_return_msg ("to accept the default");
	if (! strcmp("old",age))
	{
		ptr1 = G_ask_old("", buff, element, desc) ;
		if (ptr1)
		{
		    strcpy (buff, G_fully_qualified_name(buff,ptr1));
		}
	}
	else if (! strcmp("new",age))
		ptr1 = G_ask_new("", buff, element, desc) ;
	else if (! strcmp("mapset",age))
		ptr1 = G_ask_in_mapset("", buff, element, desc) ;
	else if (! strcmp("any",age))
		ptr1 = G_ask_any("", buff, element, desc, 1) ;
	else
	{
		fprintf(stderr,"\nPROGRAMMER ERROR: first item in gisprompt is <%s>\n", age) ;
		fprintf(stderr,"        Must be either new, old, mapset, or any\n") ;
		return -1;
	}
	if (ptr1 == '\0')
		*buff = '\0';

	return 0;
}

char *G_recreate_command (void)
{
	char flg[4] ;
	static char *buff, *cur, *tmp;
	struct Flag *flag ;
	struct Option *opt ;
	int n , len, slen;
	int nalloced = 0;

	/* Flag is not valid if there are no flags to set */
	
	buff = G_calloc (1024, sizeof(char));
	nalloced += 1024;
	tmp = G_program_name();
	len = strlen (tmp);
	if (len >= nalloced)
	{
		nalloced += (1024 > len) ? 1024 : len + 1;
		buff = G_realloc (buff, nalloced);
	}
	cur = buff;
	strcpy (cur, tmp);
	cur += len;

	if(n_flags)
	{
		flag= &first_flag;
		while(flag != '\0')
		{
			if( flag->answer == 1 )
			{
				flg[0] = ' '; flg[1] = '-'; flg[2] = flag->key; flg[3] = '\0';
				slen = strlen (flg);
				if (len + slen >= nalloced)
				{
					nalloced += (nalloced + 1024 > len + slen) ? 1024 : slen + 1;
					buff = G_realloc (buff, nalloced);
					cur = buff + len;
				}
				strcpy (cur, flg);
				cur += slen;
				len += slen;
			}
			flag = flag->next_flag ;
		}
	}

	opt= &first_option;
	while(opt != '\0')
	{
		if (opt->answer != '\0')
		{
			slen = strlen (opt->key) + strlen (opt->answers[0]) + 2;
			if (len + slen >= nalloced)
			{
				nalloced += (nalloced + 1024 > len + slen) ? 1024 : slen + 1;
				buff = G_realloc (buff, nalloced);
				cur = buff + len;
			}
			strcpy (cur, " ");
			cur++;
			strcpy (cur, opt->key);
			cur = strchr (cur, '\0');
			strcpy (cur, "=");
			cur++;
			strcpy (cur, opt->answers[0]);
			cur = strchr (cur, '\0');
			len = cur - buff;
			for(n=1;opt->answers[n] != '\0';n++)
			{
				slen = strlen (opt->answers[n]) + 1;
				if (len + slen >= nalloced)
				{
					nalloced += (nalloced + 1024 > len + slen) ? 1024 : slen + 1;
					buff = G_realloc (buff, nalloced);
					cur = buff + len;
				}
				strcpy (cur, ",");
				cur++;
				strcpy (cur, opt->answers[n]);
				cur = strchr(cur, '\0');
				len = cur - buff;
			}
		}
		opt = opt->next_opt ;
	}

	return(buff) ;
}
