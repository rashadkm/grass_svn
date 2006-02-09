#ifndef __MINGW32__

#include <grass/config.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <grass/gis.h>
#include <grass/glocale.h>
#include <grass/raster.h>
#include <grass/graphics.h>

#include "open.h"

#define BUFFERSIZ   2048

int _rfd, _wfd;
int _quiet;

extern int unlock_driver(int);

int sync_driver(char *);

static unsigned char outbuf[BUFFERSIZ];
static int cursiz;
static volatile int no_mon;

static RETSIGTYPE dead(int);

static int _get(char *buf, int n)
{
    int x;
    while (n > 0)
    {
        x = read(_rfd, buf, n);
        if (x <= 0)
        {
            fprintf(stderr, _("ERROR %s from graphics driver.\n"),
		    x ? "reading" : "eof");
            exit(1);
        }
        n -= x;
        buf += x;
    }

    return 0;
}

static int flushout(void)
{
    if (cursiz)
    {
        write(_wfd, outbuf, (size_t) cursiz);
        cursiz = 0 ;
    }

    return 0;
}

int _send_ident(int anint)
{
    unsigned char achar ;
    achar = anint;

    if ((cursiz+2) >= BUFFERSIZ)
        flushout() ;
    outbuf[cursiz++] = COMMAND_ESC ;
    outbuf[cursiz++] = achar ;

    return 0;
}

int _send_char(const unsigned char *achar)
{
    if ((cursiz+2) >= BUFFERSIZ)
        flushout() ;
    outbuf[cursiz++] = *achar ;
    if (*achar == COMMAND_ESC)
        outbuf[cursiz++] = 0 ;

    return 0;
}

int _send_char_array(int num, const unsigned char *achar)
{
    while (num-- > 0)
        _send_char(achar++);

    return 0;
}

int _send_int_array(int num, const int *anint)
{
    return _send_char_array(num * sizeof(int), (const unsigned char *) anint);
}

int _send_float_array(int num, const float *afloat)
{
    return _send_char_array(num * sizeof(float), (const unsigned char *) afloat);
}

int _send_int(const int *anint)
{
    return _send_char_array(sizeof(int), (const unsigned char *) anint);
}

int _send_float(const float *afloat)
{
    return _send_char_array(sizeof(float), (const unsigned char *) afloat);
}

int _send_text(const char *text)
{
    return _send_char_array(1 + strlen(text), (const unsigned char *) text);
}

int _get_char(char *achar)
{
    flushout() ;
    _get(achar, 1);

    return 0;
}

int _get_int(int *anint)
{
    flushout() ;
    _get((char *)anint, sizeof(int));

    return 0;
}

int _get_float(float *afloat)
{
    flushout() ;
    _get((char *)afloat, sizeof(float));

    return 0;
}

int _get_text(char *buf)
{
    char *b;

    b = buf;
    do
        _get_char(b);
    while (*b++ != 0);

    return 0;
}

char *_get_text_2(void)
{
    static char *buf;
    static int len;
    int i;

    for (i = 0; ; i++)
    {
	if (i >= len)
	{
	    len += 1000;
	    buf = G_realloc(buf, len);
	    if (!buf)
	    {
		fprintf(stderr, _("Unable to allocate memory\n"));
		exit(1);
	    }
	}
        _get_char(&buf[i]);
	if (!buf[i])
	    break;
    }

    return buf;
}

int R__open_quiet(void)
{
    _quiet = 1;

    return 0;
}

int sync_driver(char *name)
{
    RETSIGTYPE (*sigalarm)(int);
    int try;
    int count;
    unsigned char c;

    _send_ident(BEGIN);
    flushout();

/*
 * look for at least BEGIN_SYNC_COUNT zero bytes
 * then look for COMMAND_ESC
 *
 * try twice. first timeout is warning, second is fatal
 */
    count = 0;
    sigalarm = signal(SIGALRM, dead);
    for (try = 0; try < 2; try++)
    {
        no_mon = 0;
        alarm(try?10:5);
        while(no_mon == 0)
        {
            if (read(_rfd, &c, (size_t) 1) != 1)
            {
                if (no_mon)
                    break; /* from while */
                fprintf(stderr, _("ERROR - eof from graphics monitor.\n"));
                exit(-1);
            }
            if (c == 0)
                count++;
            else if (c == COMMAND_ESC && count >= BEGIN_SYNC_COUNT)
                break;
            else
                count = 0;  /* start over */
        }
        alarm(0);
        signal(SIGALRM, sigalarm);
        if (no_mon == 0)
            return 1;   /* ok! */
        if (try)
            break;

        fprintf(stderr, _("Warning - no response from graphics monitor <%s>.\n"),
            name);
        fprintf(stderr, _("Check to see if the mouse is still active.\n"));
        signal(SIGALRM, dead);
    }
    fprintf(stderr, _("ERROR - no response from graphics monitor <%s>.\n"),
        name);
    exit(-1);
}

static RETSIGTYPE dead(int sig)
{
    no_mon = 1 ;
}

int _hold_signals(int hold)
{
    static RETSIGTYPE (*sigint)(int);
    static RETSIGTYPE (*sigquit)(int);

    if (hold)
    {
        sigint  = signal(SIGINT, SIG_IGN);
        sigquit = signal(SIGQUIT, SIG_IGN);
    }
    else
    {
        signal(SIGINT, sigint);
        signal(SIGQUIT, sigquit);
    }

    return 0;
}

/*!
 * \brief synchronize graphics
 *
 * Send all pending graphics commands to the graphics driver and cause 
 * all pending graphics to be drawn (provided the driver is written to 
 * comply).  This routine does more than <i>R_flush()</i> and in many 
 * instances is the more appropriate routine of the two to use.
 *
 *  \param void
 *  \return int
 */

int R_stabilize(void)
{
    char c;

    flushout();
    _send_ident(RESPOND);
    _get_char(&c);

    return 0;
}

int R_kill_driver(void)
{
    _send_ident(GRAPH_CLOSE);
    flushout();
    R_release_driver();

    return 0;
}

/*!
 * \brief terminate graphics
 *
 * This routine breaks the connection with the graphics driver opened by R_open_driver().
 *
 *  \param void
 *  \return int
 */

int R_close_driver(void)
{
    R_stabilize();

    close(_rfd);
    close(_wfd);
    _wfd = _rfd = -1;
    unlock_driver(0);

    return 0;
}

int R_release_driver(void)
{
    close(_rfd);
    close(_wfd);
    _wfd = _rfd = -1;
    unlock_driver(1);

    return 0;
}

#else /* __MINGW32__ */


#endif /* __MINGW32__ */
