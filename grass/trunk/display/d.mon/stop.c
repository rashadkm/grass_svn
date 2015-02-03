#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

#include <grass/gis.h>
#include <grass/glocale.h>

#include "proto.h"

static int stop_wx(const char *);
static int stop(const char *);

int stop_mon(const char *name)
{
    if (!check_mon(name)) {
	G_fatal_error(_("Monitor <%s> is not running"), name);
    }
    
    if (strncmp(name, "wx", 2) == 0)
	return stop_wx(name);

    return stop(name);
}

int stop(const char *name)
{
    char *mon_path, file_path[GPATH_MAX];
    struct dirent *dp;
    DIR *dirp;

    mon_path = get_path(name, TRUE);
    dirp = opendir(mon_path);

    while ((dp = readdir(dirp)) != NULL) {
        if (!dp->d_name || dp->d_name[0] == '.')
            continue;
        sprintf(file_path, "%s/%s", mon_path, dp->d_name);
        if (unlink(file_path) == -1)
            G_warning(_("Unable to delete file '%s'"), file_path);
    }
    closedir(dirp);
    
    if (rmdir(mon_path) == -1)
        G_warning(_("Unable to delete directory '%s'"), mon_path);

    G_free(mon_path);

    G_unsetenv("MONITOR");

    return 0;
}

int stop_wx(const char *name)
{
    char *env_name;
    const char *pid;

    env_name = NULL;
    G_asprintf(&env_name, "MONITOR_%s_PID", G_store_upper(name));
    
    pid = G_getenv_nofatal(env_name);
    if (!pid) {
	G_fatal_error(_("PID file not found"));
    }
    
#ifdef __MINGW32__
    /* TODO */
#else
    if (kill((pid_t) atoi(pid), SIGTERM) != 0) {
	/* G_fatal_error(_("Unable to stop monitor <%s>"), name); */
    }
#endif
    
    return 0;
}
