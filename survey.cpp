
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



/*!
  - survey.c

  - purpose:  read in the iss60 survey plan file.

  - written:  7 aug 2000, dave fabre, neptune sciences, inc.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "nvutility.h"
#include "survey.hpp"		/* structs & prototypes */

#define MBS	512		/* my buffer size */


/* prototype functions used only in this file */

static char *strtolower(char * string);
static int32_t get_ids(char * buff, LINE * line);
static int32_t get_point(char * buff, int32_t * id, double * lon,
    double * lat);
static int32_t get_points(FILE * infile, LINE * line);
static int32_t get_lsurvey(FILE * infile, SURVEY * srv);
static int32_t get_psurvey(FILE * infile, SURVEY * srv);



/*****************************************************************************/

static char *strtolower(char * string)
{
    int32_t         i = (-1);

    while (string[++i] != '\0')
    {
	string[i] = (char) tolower(string[i]);
    }

    return string;
}				/* strtolower */



/*****************************************************************************/

/* determine the way-point id numbers for this line */

static int32_t get_ids(char * buff, LINE * line)
{
    int32_t         stat = 0;
    char           *ptr;


    /* initialize to -99 */

    line->i = line->p1.i = line->p2.i = -99;

    if ((ptr = strstr(buff, "-")))
	stat += sscanf(ptr + 1, "%d", &line->i);
    if ((ptr = strstr(ptr + 1, "-")))
	stat += sscanf(ptr + 1, "%d", &line->p1.i);
    if ((ptr = strstr(ptr + 1, "-")))
	stat += sscanf(ptr + 1, "%d", &line->p2.i);
    return stat == 3;
}				/* get_ids */



/*****************************************************************************/

static int32_t get_point(char * buff, int32_t * id, double * lon, 
double * lat)
{
    char           *ptr = NULL, h;
    double          d, m, s;
    int32_t         s1, s2, stat = 0;

    *id = -98;
    *lon = -181;
    *lat = -91;
    if ((ptr = strstr(buff, "-")))
    {
	stat = sscanf(ptr + 1, "%d", id);
    }

    if (stat)
    {
	if ((ptr = strstr(buff, ";")))
	{
	    /* read in latitude */

	    stat = sscanf(ptr + 1, "%lf %lf %2d.%5d", &d, &m, &s1, &s2);
	    s = s1 + (double) s2 / 1e5;
	    ptr = strstr(ptr + 1, ";");
	    stat += sscanf(ptr - 1, "%c", &h);
	    *lat = d + m / 60 + s / 3600;
	    if (stat == 5 && h == 's')
		*lat *= -1;


	    /* read in longitude */

	    stat = sscanf(ptr + 1, "%lf %lf %2d.%5d", &d, &m, &s1, &s2);
	    s = s1 + (double) s2 / 1e5;
	    ptr = strstr(ptr + 1, ";");
	    stat += sscanf(ptr - 1, "%c", &h);
	    *lon = d + m / 60 + s / 3600;
	    if (stat == 5 && h == 'w')
		*lon *= -1;
	}
    }
    return stat == 5;

}				/* get_point */



/*****************************************************************************/

static int32_t get_points(FILE * infile, LINE * line)
{
    char            buff[MBS];
    double          lat, lon;
    int32_t         i, stat, id, gotone = 0, gottwo = 0;

    for (i = 0; i < MBS; i++)
	buff[0] = 0;		/* init buff */

    line->p1.x = line->p1.y = line->p2.x = line->p2.y = -999;
    rewind(infile);
    while (fgets(buff, MBS, infile) != NULL && !(gotone && gottwo))
    {
	strtolower(buff);
	if (strncmp(buff, "point", 5) == 0 &&
	    !strstr(buff, "corner") && !strstr(buff, "center"))
	{
	    stat = get_point(buff, &id, &lon, &lat);

	    if (stat && id == line->p1.i)
	    {
		line->p1.x = lon;
		line->p1.y = lat;
		gotone = 1;
	    }
	    else if (stat && id == line->p2.i)
	    {
		line->p2.x = lon;
		line->p2.y = lat;
		gottwo = 1;
	    }

	}			/* if this is a point */
    }				/* while reading & haven't got both points */

    return (gotone && gottwo);

}				/* get_points */



/*****************************************************************************/

static int32_t get_lsurvey(FILE * infile, SURVEY * srv)
{
    char            buff[MBS];
    int32_t         i = (-1), n = 0;


    /* determine the id numbers of the lines & points */

    rewind(infile);
    n = 0;
    while (fgets(buff, MBS, infile) != NULL)
    {
	strtolower(buff);
	if (strncmp(buff, "line", 4) == 0)
	{
	    n += get_ids(buff, &srv->line[++i]);
	}
    }
    if (n != srv->n)
    {
	printf("get_survey:  warning got %d (!= %d) line ids\n", n, srv->n);
    }


    /* now fill in the points for each line */

    n = 0;
    for (i = 0; i < srv->n; i++)
    {
	n += get_points(infile, &srv->line[i]);
    }

    if (n != srv->n)
    {
	printf("get_dsurvey:  warning got %d (!= %d) lines\n", n, srv->n);
    }

    return n;

}				/* get_lsurvey */



/*****************************************************************************/

static int32_t get_psurvey(FILE * infile, SURVEY * srv)
{
    char            buff[MBS];
    int32_t         i = (-1), n = 0, id0 = 0, id;
    double          lon0 = 0.0, lat0 = 0.0, lon, lat;
    static int32_t first = 1;


    rewind(infile);
    n = 0;
    while (fgets(buff, MBS, infile) != NULL)
    {
	strtolower(buff);
	if (strncmp(buff, "point", 5) == 0 &&
	    !strstr(buff, "corner") && !strstr(buff, "center"))
	{
	    n += get_point(buff, &id, &lon, &lat);

	    if (first)
	    {
		id0 = id;
		lon0 = lon;
		lat0 = lat;
		first = 0;
	    }
	    else
	    {

		if (++i < srv->n)
		{
		    srv->line[i].i = i;
		    srv->line[i].p1.i = id0;
		    srv->line[i].p1.x = lon0;
		    srv->line[i].p1.y = lat0;
		    srv->line[i].p2.i = id;
		    srv->line[i].p2.x = lon;
		    srv->line[i].p2.y = lat;
		}
		id0 = id;
		lon0 = lon;
		lat0 = lat;

	    }			/* else !first */
	}
    }


    return i + 1;

}				/* get_psurvey */



/*****************************************************************************/

int32_t get_dsurvey (FILE *infile, SURVEY *srv)
{
    char            buff[MBS];
    int32_t         n = 0, j = 0, lines = 0, stat;


    /* determine the number of lines */

    while (fgets(buff, MBS, infile) != NULL)
    {
	strtolower(buff);
	if (strncmp(buff, "line", 4) == 0)
	    ++n;
	if (strncmp(buff, "point", 5) == 0 &&
	    !strstr(buff, "corner") && !strstr(buff, "center"))
	    ++j;
    }

    if (n == 0)			/* no lines defined */
    {
	lines = 0;
	n = j - 1;		/* match adj points */
    }
    else if (j > 1)		/* if some points */
    {
	lines = 1;
    }
    srv->n = n;


    /* malloc memory for the n lines */

    srv->line = (LINE *) malloc(sizeof(LINE) * n);
    if (srv->line == NULL)
    {
	printf("get_dsurvey:  malloc of %d lines failed\n", n);
	return 0;
    }

    if (lines)
	stat = get_lsurvey(infile, srv);
    else
	stat = get_psurvey(infile, srv);

    fclose(infile);

    return stat == n;

}				/* get_survey */



/*****************************************************************************/

int32_t show_dsurvey(SURVEY * srv)
{
    int32_t         i;

    for (i = 0; i < srv->n; i++)
    {
	printf("%d %d %.9f %.9f %d %.9f %.9f\n",
	    srv->line[i].i,
	    srv->line[i].p1.i,
	    srv->line[i].p1.x,
	    srv->line[i].p1.y,
	    srv->line[i].p2.i, srv->line[i].p2.x, srv->line[i].p2.y);
    }
    return i;
}				/* show_dsurvey */


/*****************************************************************************/

void free_dsurvey(SURVEY * srv)
{
    free(srv->line);
}				/* free_dsurvey */


#if defined( _TEST_SURVEY )



/*****************************************************************************/

int32_t main(int32_t argc, char ** argv)
{
    FILE           *infile;
    SURVEY          srv;
    int32_t         stat;

    if (argc != 2)
    {
	printf("usage:  %s infilename\n", argv[0]);
	exit(-1);
    }

    infile = fopen(argv[1], "r");

    if (infile == NULL)
    {
	printf("main:  couldn't open survey file <%s>\n", argv[1]);
	return 0;
    }


    stat = get_dsurvey(infile, &srv);
    if (stat)
	show_dsurvey(&srv);
    if (stat)
	free_dsurvey(&srv);

    return 0;

}				/* main */

#endif
