/*****************************************************************************

All of gpsd's assumptions about time and GPS time reporting live in this file.

This is a work in progress.  Currently GPSD requires that the host system 
clock be accurate to within one second.  We are attempting to relax this
to "accurate within one GPS rollover period" for receivers reporting
GPS week+TOW.

Date and time in GPS is represented as number of weeks from the start
of zero second of 6 January 1980, plus number of seconds into the
week.  GPS time is not leap-second corrected, though satellites also
broadcast a current leap-second correction which is updated on
six-month boundaries according to rotational bulletins issued by the
International Earth Rotation and Reference Systems Service (IERS).

The leap-second correction is only included in the satellite subframre
broadcast, roughly once ever 20 minutes.  While the satellites do
notify GPSes of upcoming leap-seconds, this notification is not
necessarily processed correctly on consumer-grade devices, and will
not be available at all when a GPS receiver has just
cold-booted. Thus, UTC time reported from NMEA devices may be slightly
inaccurate between a cold boot or leap second and the following
subframe broadcast.

GPS date and time are subject to a rollover problem in the 10-bit week
number counter, which will re-zero every 1024 weeks (roughly every 20
years). The last rollover (and the first since GPS went live in 1980)
was 0000 22 August 1999; the next would fall in 2019, but plans are
afoot to upgrade the satellite counters to 13 bits; this will delay
the next rollover until 2173.

For accurate time reporting, therefore, a GPS requires a supplemental
time references sufficient to identify the current rollover period,
e.g. accurate to within 512 weeks.  Many NMEA GPSes have a wired-in
assumption about the UTC time of the last rollover and will thus report
incorrect times outside the rollover period they were designed in.

These conditions leave gpsd in a serious hole.  Actually there are several
interrelated problems:

1) Every NMEA device has some assumption about base epoch (date of
last rollover) that we don't have access to.  Thus, there's no way to
check whether a rollover the device wasn't prepared for has occurred
before gpsd startup time (making the reported UTC date invalid)
without some other time source.  (Some NMEA devices may keep a
rollover count in RAM and avoid the problem; we can't tell when that's
happening, either.)

2) Many NMEA devices - in fact, all that don't report ZDA - never tell
us what century they think it is. Those that do report century are
still subject to rollover problems. We need an external time reference
for this, too.

3) Supposing we're looking at a binary protocol that returns week/tow,
we can't know which rollover period we're in without an external time
source.

4) Only one external time source, the host system clock, is reliably
available.

5) Another source *may* be available - the GPS leap second count, if we can
get the device to report it. The latter is not a given; SiRFs before 
firmware rev 2.3.2 don't report it unless special subframe data reporting
is enabled, which requires 38400bps. Evermore GPSes can't be made to
report it at all.  

Conclusion: if the system clock isn't accurate enough that we can deduce 
what rollover period we're in, we're utterly hosed. Furthermore, if it's
not accurate to within a second and only NMEA devices are reporting, 
we don't know what century it is! 

Therefore, we must assume the system clock is reliable.

This file is Copyright (c) 2010 by the GPSD project
BSD terms apply: see the file COPYING in the distribution root for details.

*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "gpsd.h"
#include "timebase.h"

#define SECS_PER_WEEK	(60*60*24*7)	/* seconds per week */
#define GPS_ROLLOVER	(1024*SECS_PER_WEEK)	/* rollover period */

void gpsd_time_init(struct gps_context_t *context, time_t starttime)
/* initialize the GPS context's time fields */
{
    /*
     * Provides a start time for getting the century.  Do this, just
     * in case one of our embedded deployments is still in place in
     * the year 2.1K.  Still likely to fail if we bring up the daemon
     * just before a century mark, but that case is probably doomed
     * anyhow because of 2-digit years.
     */
    context->leap_seconds = LEAPSECOND_NOW;
    context->century = CENTURY_BASE;
    context->start_time = starttime;

    context->rollovers = (int)((context->start_time-GPS_EPOCH) / GPS_ROLLOVER);

    if (context->start_time < GPS_EPOCH)
	gpsd_report(LOG_ERROR, "system time looks bogus, dates may not be reliable.\n");
    else {
	struct tm *now = localtime(&context->start_time);
	char scr[128];
	/*
	 * This is going to break our regression-test suite once a century.
	 * I think we can live with that consequence.
	 */
	now->tm_year += 1900;
	context->century = now->tm_year - (now->tm_year % 100);
	(void)unix_to_iso8601((timestamp_t)context->start_time, scr, sizeof(scr));
	gpsd_report(LOG_INF, "startup at %s (%d)\n", 
		    scr, (int)context->start_time);
    }
}

void gpsd_set_century(struct gps_device_t *session)
/*
 * Interpret "Date: yyyy-mm-dd", setting the session context
 * century from the year.  We do this so the behavior of the
 * regression tests won't depend on what century the daemon
 * started up in.
 */
{
    unsigned char *cp; 
    char *end;
    int year;
    if (strstr((char *)session->packet.outbuffer, "Date:") != NULL) {
	cp = session->packet.outbuffer + 5;
	while (isspace(*cp))
	    --cp;
	year = (int)strtol((char *)cp, &end, 10);
	session->context->century = year - (year % 100);
    }
}

#ifdef NMEA_ENABLE
timestamp_t gpsd_utc_resolve(/*@in@*/struct gps_device_t *session)
/* resolve a UTC date, checking for rollovers */
{
    /*
     * We'd like to *correct* for rollover the way we do for GPS week.
     * In theory, comparing extracted UTC against present time should 
     * allow us to compute the device's epoch assumption.  In practice,
     * this will be hairy and risky.
     */
    timestamp_t t;

    t = (timestamp_t)mkgmtime(&session->driver.nmea.date) +
	session->driver.nmea.subseconds;
    session->context->valid &=~ GPS_TIME_VALID;

    /*
     * If the system clock is zero or has a small-integer value,
     * no further sanity-checking is possible.
     */
    if (session->context->start_time < GPS_EPOCH)
	return t;

    /*
     * If the GPS is reporting a time from before the daemon started, we've
     * had a rollover event while the daemon was running.
     *
     * The reason for the 12-hour slop is that our recorded start time is local,
     * but GPSes deliver time as though in UTC.  This test could be exact if we
     * counted on knowing our timezone at startup, but since we can't count on
     * knowing location...
     */
    if (session->newdata.time + (12*60*60) < (timestamp_t)session->context->start_time) {
	char scr[128];
	(void)unix_to_iso8601(session->newdata.time, scr, sizeof(scr));
	gpsd_report(LOG_WARN, "GPS week rollover makes time %s (%f) invalid\n", 
		    scr, session->newdata.time);
    }

    return t;
}
#endif /* NMEA_ENABLE */

timestamp_t gpsd_gpstime_resolve(/*@in@*/struct gps_device_t *session,
			 unsigned short week, double tow)
{
    timestamp_t t;

    /*
     * This code detects and compensates for week counter rollovers that
     * happen while gpsd is running. It will not save you if there was a 
     * rollover that confused the receiver before gpsd booted up.  It *will*
     * work even when Block IIF satellites increase the week counter width
     * to 13 bits,
     */
    if ((int)week < (session->context->gps_week & 0x3ff)) {
	gpsd_report(LOG_INF, "GPS week 10-bit rollover detected.\n");
	++session->context->rollovers;
    }

    /*
     * This guard copes with both conventional GPS weeks and the "extended"
     * 15-or-16-bit version with no wraparound that appears in Zodiac
     * chips and is supposed to appear in the Geodetic Navigation
     * Information (0x29) packet of SiRF chips.  Some SiRF firmware versions
     * (notably 231) actually ship the wrapped 10-bit week, despite what
     * the protocol reference claims.
     */
    if (week < 1024)
	week += session->context->rollovers * 1024;

    t = GPS_EPOCH + (week * SECS_PER_WEEK) + tow;
    t -= session->context->leap_seconds;

    session->context->gps_week = week;
    session->context->gps_tow = tow;
    session->context->valid |= GPS_TIME_VALID;

    return t;
}

/* end */
