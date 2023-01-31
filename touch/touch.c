/*
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>

__FBSDID("$FreeBSD: src/usr.bin/touch/touch.c,v 1.25 2010/03/28 13:16:08 ed Exp $");

#ifndef lint
__used static const char copyright[] =
"@(#) Copyright (c) 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#ifndef lint
__used static const char sccsid[] = "@(#)touch.c	8.1 (Berkeley) 6/6/93";
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/attr.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct {
	struct timespec mtime;
	struct timespec atime;
} set_ts;

int	rw(char *, struct stat *, int);
void	stime_arg1(char *, set_ts *);
void	stime_arg2(char *, int, set_ts *);
void	stime_file(char *, set_ts *);
int	timeoffset(char *);
void	usage(char *);

int
main(int argc, char *argv[])
{
	struct stat sb;
	int (*stat_f)(const char *, struct stat *);
	int (*utimes_f)(const char *, const struct timeval *);
	int Aflag, aflag, cflag, fflag, mflag, ch, fd, len, rval, timeset;
	char *p;
	char *myname;
	struct attrlist ts_req = {
		.bitmapcount = ATTR_BIT_MAP_COUNT,
		.commonattr = ATTR_CMN_MODTIME | ATTR_CMN_ACCTIME,
	};
	set_ts ts_struct = {};

	myname = basename(argv[0]);
	Aflag = aflag = cflag = fflag = mflag = timeset = 0;
	stat_f = stat;
	utimes_f = utimes;
	if (clock_gettime(CLOCK_REALTIME, &ts_struct.mtime))
		err(1, "clock_gettime");
	ts_struct.atime = ts_struct.mtime;

	while ((ch = getopt(argc, argv, "A:acfhmr:t:")) != -1)
		switch(ch) {
		case 'A':
			Aflag = timeoffset(optarg);
			break;
		case 'a':
			aflag = 1;
			break;
		case 'c':
			cflag = 1;
			break;
		case 'f':
			fflag = 1;
			break;
		case 'h':
			cflag = 1;
			stat_f = lstat;
			utimes_f = lutimes;
			break;
		case 'm':
			mflag = 1;
			break;
		case 'r':
			timeset = 1;
			stime_file(optarg, &ts_struct);
			break;
		case 't':
			timeset = 1;
			stime_arg1(optarg, &ts_struct);
			break;
		case '?':
		default:
			usage(myname);
		}
	argc -= optind;
	argv += optind;

	if (aflag == 0 && mflag == 0)
		aflag = mflag = 1;

	if (timeset) {
		if (Aflag) {
			/*
			 * We're setting the time to an offset from a specified
			 * time.  God knows why, but it means that we can set
			 * that time once and for all here.
			 */
			if (aflag)
				ts_struct.atime.tv_sec += Aflag;
			if (mflag)
				ts_struct.mtime.tv_sec += Aflag;
			Aflag = 0;		/* done our job */
		}
	} else {
		/*
		 * If no -r or -t flag, at least two operands, the first of
		 * which is an 8 or 10 digit number, use the obsolete time
		 * specification, otherwise use the current time.
		 */
		if (argc > 1) {
			strtol(argv[0], &p, 10);
			len = p - argv[0];
			if (*p == '\0' && (len == 8 || len == 10)) {
				timeset = 1;
				stime_arg2(*argv++, len == 10, &ts_struct);
			}
		}
	}

	if (*argv == NULL)
		usage(myname);

	if (Aflag)
		cflag = 1;

	for (rval = 0; *argv; ++argv) {
		/* See if the file exists. */
		if (stat_f(*argv, &sb) != 0) {
			if (errno != ENOENT) {
				rval = 1;
				warn("%s", *argv);
				continue;
			}
			if (!cflag) {
				/* Create the file. */
				fd = open(*argv,
				    O_WRONLY | O_CREAT, DEFFILEMODE);
				if (fd == -1 || fstat(fd, &sb) || close(fd)) {
					rval = 1;
					warn("%s", *argv);
					continue;
				}

				/* If using the current time, we're done. */
				if (!timeset)
					continue;
			} else
				continue;
		}

		if (!aflag)
			ts_struct.atime = sb.st_atimespec;
		if (!mflag)
			ts_struct.mtime = sb.st_mtimespec;

		/*
		 * We're adjusting the times based on the file times, not a
		 * specified time (that gets handled above).
		 */
		if (Aflag) {
			if (aflag) {
				ts_struct.atime = sb.st_atimespec;
				ts_struct.atime.tv_sec += Aflag;
			}
			if (mflag) {
				ts_struct.mtime = sb.st_mtimespec;
				ts_struct.mtime.tv_sec += Aflag;
			}
		}

		/* Try setattrlist(2). */
		if (!setattrlist(*argv, &ts_req, &ts_struct, sizeof(ts_struct), 0))
			continue;

		/* If the user specified a time, nothing else we can do. */
		if (timeset || Aflag) {
			rval = 1;
			warn("%s", *argv);
			continue;
		}

		/*
		 * System V and POSIX 1003.1 require that a NULL argument
		 * set the access/modification times to the current time.
		 * The permission checks are different, too, in that the
		 * ability to write the file is sufficient.  Take a shot.
		 */
		 if (!utimes_f(*argv, NULL))
			continue;

		/* Try reading/writing. */
		if (!S_ISLNK(sb.st_mode) && !S_ISDIR(sb.st_mode)) {
			if (rw(*argv, &sb, fflag))
				rval = 1;
		} else {
			rval = 1;
			warn("%s", *argv);
		}
	}
	exit(rval);
}

#define	ATOI2(ar)	((ar)[0] - '0') * 10 + ((ar)[1] - '0'); (ar) += 2;

void
stime_arg1(char *arg, set_ts *tsp)
{
	time_t now;
	struct tm *t;
	int yearset;
	char *p;
					/* Start with the current time. */
	now = tsp->atime.tv_sec;
	if ((t = localtime(&now)) == NULL)
		err(1, "localtime");
					/* [[CC]YY]MMDDhhmm[.SS] */
	if ((p = strchr(arg, '.')) == NULL)
		t->tm_sec = 0;		/* Seconds defaults to 0. */
	else {
		if (strlen(p + 1) != 2)
			goto terr;
		*p++ = '\0';
		t->tm_sec = ATOI2(p);
	}

	yearset = 0;
	switch(strlen(arg)) {
	case 12:			/* CCYYMMDDhhmm */
		t->tm_year = ATOI2(arg);
		t->tm_year *= 100;
		yearset = 1;
		/* FALLTHROUGH */
	case 10:			/* YYMMDDhhmm */
		if (yearset) {
			yearset = ATOI2(arg);
			t->tm_year += yearset;
		} else {
			yearset = ATOI2(arg);
			if (yearset < 69)
				t->tm_year = yearset + 2000;
			else
				t->tm_year = yearset + 1900;
		}
		t->tm_year -= 1900;	/* Convert to UNIX time. */
		/* FALLTHROUGH */
	case 8:				/* MMDDhhmm */
		t->tm_mon = ATOI2(arg);
		--t->tm_mon;		/* Convert from 01-12 to 00-11 */
		t->tm_mday = ATOI2(arg);
		t->tm_hour = ATOI2(arg);
		t->tm_min = ATOI2(arg);
		break;
	default:
		goto terr;
	}

	t->tm_isdst = -1;		/* Figure out DST. */
	tsp->atime.tv_sec = tsp->mtime.tv_sec = mktime(t);
	if (tsp->atime.tv_sec == -1)
terr:		errx(1,
	"out of range or illegal time specification: [[CC]YY]MMDDhhmm[.SS]");

	tsp->atime.tv_nsec = tsp->mtime.tv_nsec = 0;
}

void
stime_arg2(char *arg, int year, set_ts *tsp)
{
	time_t now;
	struct tm *t;
					/* Start with the current time. */
	now = tsp->atime.tv_sec;
	if ((t = localtime(&now)) == NULL)
		err(1, "localtime");

	t->tm_mon = ATOI2(arg);		/* MMDDhhmm[yy] */
	--t->tm_mon;			/* Convert from 01-12 to 00-11 */
	t->tm_mday = ATOI2(arg);
	t->tm_hour = ATOI2(arg);
	t->tm_min = ATOI2(arg);
	if (year) {
		t->tm_year = ATOI2(arg);
		if (t->tm_year < 39)	/* support 2000-2038 not 1902-1969 */
			t->tm_year += 100;
	}

	t->tm_isdst = -1;		/* Figure out DST. */
	tsp->atime.tv_sec = tsp->mtime.tv_sec = mktime(t);
	if (tsp->atime.tv_sec == -1)
		errx(1,
	"out of range or illegal time specification: MMDDhhmm[yy]");

	tsp->atime.tv_nsec = tsp->mtime.tv_nsec = 0;
}

/* Calculate a time offset in seconds, given an arg of the format [-]HHMMSS. */
int
timeoffset(char *arg)
{
	int offset;
	int isneg;

	offset = 0;
	isneg = *arg == '-';
	if (isneg)
		arg++;
	switch (strlen(arg)) {
	default:				/* invalid */
		errx(1, "Invalid offset spec, must be [-][[HH]MM]SS");

	case 6:					/* HHMMSS */
		offset = ATOI2(arg);
		/* FALLTHROUGH */
	case 4:					/* MMSS */
		offset = offset * 60 + ATOI2(arg);
		/* FALLTHROUGH */
	case 2:					/* SS */
		offset = offset * 60 + ATOI2(arg);
	}
	if (isneg)
		return (-offset);
	else
		return (offset);
}

void
stime_file(char *fname, set_ts *ts_struct)
{
	struct stat sb;

	if (stat(fname, &sb))
		err(1, "%s", fname);
	ts_struct->atime = sb.st_atimespec;
	ts_struct->mtime = sb.st_mtimespec;
}

int
rw(char *fname, struct stat *sbp, int force)
{
	int fd, needed_chmod, rval;
	u_char byte;

	/* Try regular files. */
	if (!S_ISREG(sbp->st_mode)) {
		warnx("%s: %s", fname, strerror(EFTYPE));
		return (1);
	}

	needed_chmod = rval = 0;
	if ((fd = open(fname, O_RDWR, 0)) == -1) {
		if (!force || chmod(fname, DEFFILEMODE))
			goto err;
		if ((fd = open(fname, O_RDWR, 0)) == -1)
			goto err;
		needed_chmod = 1;
	}

	if (sbp->st_size != 0) {
		if (read(fd, &byte, sizeof(byte)) != sizeof(byte))
			goto err;
		if (lseek(fd, (off_t)0, SEEK_SET) == -1)
			goto err;
		if (write(fd, &byte, sizeof(byte)) != sizeof(byte))
			goto err;
	} else {
		if (write(fd, &byte, sizeof(byte)) != sizeof(byte)) {
err:			rval = 1;
			warn("%s", fname);
		} else if (ftruncate(fd, (off_t)0)) {
			rval = 1;
			warn("%s: file modified", fname);
		}
	}

	if (close(fd) && rval != 1) {
		rval = 1;
		warn("%s", fname);
	}
	if (needed_chmod && chmod(fname, sbp->st_mode) && rval != 1) {
		rval = 1;
		warn("%s: permissions modified", fname);
	}
	return (rval);
}

void
usage(char *myname)
{
	fprintf(stderr, "usage:\n" "%s [-A [-][[hh]mm]SS] [-acfhm] [-r file] "
		"[-t [[CC]YY]MMDDhhmm[.SS]] file ...\n", myname);
	exit(1);
}
