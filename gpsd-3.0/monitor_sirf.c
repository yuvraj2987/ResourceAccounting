/*
 * SiRF object for the GPS packet monitor.
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 *
 */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

#include "gpsd.h"
#include "bits.h"
#include "gpsmon.h"

#if defined(SIRF_ENABLE) && defined(BINARY_ENABLE)
extern const struct gps_type_t sirf_binary;

static WINDOW *mid2win, *mid4win, *mid6win, *mid7win, *mid9win, *mid13win;
static WINDOW *mid19win, *mid27win;
static bool dispmode = false, subframe_enabled = false, ppstime_enabled = false;
static int nfix, fix[20];
static int leapseconds;

/*@ -nullassign @*/
static char *verbpat[] = {
    "#Time:",
    "@R Time:",
    "CSTD: New almanac for",
    "NOTICE: DOP Q Boost",
    "RTC not set",
    "numOfSVs = 0",
    "rtcaj tow ",
    NULL
};

/*@ +nullassign @*/

static char *dgpsvec[] = {
    "None",
    "SBAS",
    "Serial",
    "Beacon",
    "Software",
};

/* check range of an unsigned quantity */
#define CHECK_RANGE(vec, i) ((i) < sizeof(vec)/sizeof(vec[0]))

/*****************************************************************************
 *
 * SiRF packet-decoding routines
 *
 *****************************************************************************/

#define display	(void)mvwprintw

#define MAXSATS 	12	/* the most satellites we can dump data on */

static bool sirf_initialize(void)
{
    unsigned int i;

    /*@ -onlytrans @*/
    mid2win = subwin(devicewin, 6, 80, 1, 0);
    mid4win = subwin(devicewin, MAXSATS + 3, 30, 7, 0);
    mid6win = subwin(devicewin, 3, 50, 7, 30);
    mid7win = subwin(devicewin, 4, 50, 13, 30);
    mid9win = subwin(devicewin, 3, 50, 10, 30);
    mid13win = subwin(devicewin, 3, 50, 17, 30);
    mid19win = newwin(16, 50, 7, 30);
    mid27win = subwin(devicewin, 3, 50, 20, 30);
    if (mid2win == NULL || mid4win == NULL || mid6win == NULL
	|| mid9win == NULL || mid13win == NULL || mid19win == NULL
	|| mid27win == NULL)
	return false;

    (void)syncok(mid2win, true);
    (void)syncok(mid4win, true);
    (void)syncok(mid6win, true);
    (void)syncok(mid7win, true);
    (void)syncok(mid9win, true);
    (void)syncok(mid13win, true);
    (void)syncok(mid27win, true);

    /*@ -nullpass @*/
    (void)wborder(mid2win, 0, 0, 0, 0, 0, 0, 0, 0),
	(void)wattrset(mid2win, A_BOLD);
    (void)wmove(mid2win, 0, 1);
    display(mid2win, 0, 12, " X ");
    display(mid2win, 0, 21, " Y ");
    display(mid2win, 0, 30, " Z ");
    display(mid2win, 0, 43, " North ");
    display(mid2win, 0, 54, " East ");
    display(mid2win, 0, 65, " Alt ");

    (void)wmove(mid2win, 1, 1);
    (void)wprintw(mid2win,
		  "Pos:                            m                                    m");
    (void)wmove(mid2win, 2, 1);
    (void)wprintw(mid2win,
		  "Vel:                            m/s                                  climb m/s");
    (void)wmove(mid2win, 3, 1);
    (void)wprintw(mid2win,
		  "Time:                          Leap:        Heading:                 speed m/s");
    (void)wmove(mid2win, 4, 1);
    (void)wprintw(mid2win,
		  "Fix:                                                 HDOP:      M1:     M2:   ");
    display(mid2win, 5, 24, " Packet type 2 (0x02) ");
    (void)wattrset(mid2win, A_NORMAL);

    (void)wborder(mid4win, 0, 0, 0, 0, 0, 0, 0, 0),
	(void)wattrset(mid4win, A_BOLD);
    display(mid4win, 1, 1, "Ch PRN  Az El Stat  C/N ? A");
    for (i = 0; i < MAXSATS; i++) {
	display(mid4win, (int)(i + 2), 1, "%2d", i);
    }
    display(mid4win, 0, 1, " Measured Tracker ");
    display(mid4win, 14, 4, " Packet Type 4 (0x04) ");
    (void)wattrset(mid4win, A_NORMAL);

    (void)wborder(mid19win, 0, 0, 0, 0, 0, 0, 0, 0),
	(void)wattrset(mid19win, A_BOLD);
    display(mid19win, 1, 1, "Alt. hold mode:");
    display(mid19win, 2, 1, "Alt. hold source:");
    display(mid19win, 3, 1, "Alt. source input:");
    display(mid19win, 4, 1, "Degraded timeout:");
    display(mid19win, 5, 1, "DR timeout:");
    display(mid19win, 6, 1, "Track smooth mode:");
    display(mid19win, 7, 1, "Static Navigation:");
    display(mid19win, 8, 1, "3SV Least Squares:");
    display(mid19win, 9, 1, "DOP Mask mode:");
    display(mid19win, 10, 1, "Nav. Elev. mask:");
    display(mid19win, 11, 1, "Nav. Power mask:");
    display(mid19win, 12, 1, "DGPS Source:");
    display(mid19win, 13, 1, "DGPS Mode:");
    display(mid19win, 14, 1, "DGPS Timeout:");
    display(mid19win, 1, 26, "LP Push-to-Fix:");
    display(mid19win, 2, 26, "LP On Time:");
    display(mid19win, 3, 26, "LP Interval:");
    display(mid19win, 4, 26, "U. Tasks Enab.:");
    display(mid19win, 5, 26, "U. Task Inter.:");
    display(mid19win, 6, 26, "LP Pwr Cyc En:");
    display(mid19win, 7, 26, "LP Max Acq Srch:");
    display(mid19win, 8, 26, "LP Max Off Time:");
    display(mid19win, 9, 26, "APM enabled:");
    display(mid19win, 10, 26, "# of Fixes:");
    display(mid19win, 11, 26, "Time btw Fixes:");
    display(mid19win, 12, 26, "H/V Error Max:");
    display(mid19win, 13, 26, "Rsp Time Max:");
    display(mid19win, 14, 26, "Time/Accu:");

    display(mid19win, 15, 8, " Packet type 19 (0x13) ");
    (void)wattrset(mid19win, A_NORMAL);

    (void)wborder(mid6win, 0, 0, 0, 0, 0, 0, 0, 0),
	(void)wattrset(mid6win, A_BOLD);
    display(mid6win, 0, 1, " Firmware Version ");
    display(mid6win, 2, 8, " Packet Type 6 (0x06) ");
    (void)wattrset(mid6win, A_NORMAL);

    (void)wborder(mid7win, 0, 0, 0, 0, 0, 0, 0, 0),
	(void)wattrset(mid7win, A_BOLD);
    display(mid7win, 0, 1, " Clock Status ");
    display(mid7win, 1, 1, "SVs: ");
    display(mid7win, 1, 9, "Drift: ");
    display(mid7win, 1, 23, "Bias: ");
    display(mid7win, 2, 1, "Estimated GPS Time: ");
    display(mid7win, 3, 8, " Packet type 7 (0x07) ");
    (void)wattrset(mid7win, A_NORMAL);

    (void)wborder(mid9win, 0, 0, 0, 0, 0, 0, 0, 0),
	(void)wattrset(mid9win, A_BOLD);
    display(mid9win, 0, 1, " CPU Throughput ");
    display(mid9win, 1, 1, "Max: ");
    display(mid9win, 1, 13, "Lat: ");
    display(mid9win, 1, 25, "Time: ");
    display(mid9win, 1, 39, "MS: ");
    display(mid9win, 2, 8, " Packet type 9 (0x09) ");
    (void)wattrset(mid9win, A_NORMAL);

    (void)wborder(mid13win, 0, 0, 0, 0, 0, 0, 0, 0),
	(void)wattrset(mid13win, A_BOLD);
    display(mid13win, 0, 1, " Visible List ");
    display(mid13win, 2, 8, " Packet type 13 (0x0D) ");
    (void)wattrset(mid13win, A_NORMAL);

    (void)wborder(mid27win, 0, 0, 0, 0, 0, 0, 0, 0),
	(void)wattrset(mid27win, A_BOLD);
    display(mid27win, 0, 1, " DGPS Status ");
    display(mid27win, 2, 8, " Packet type 27 (0x1B) ");
    (void)wattrset(mid27win, A_NORMAL);
    /*@ +nullpass @*/
    /*@ -onlytrans @*/

#ifdef CONTROLSEND_ENABLE
    /* probe for version */
    /*@ -compdef @*/
    (void)monitor_control_send((unsigned char *)"\x84\x00", 2);
    /*@ +compdef @*/
#endif /* CONTROLSEND_ENABLE */

    /* initialize the GPS context's time fields */
    gpsd_time_init(session.context, time(NULL));

    return true;
}

static void decode_ecef(double x, double y, double z,
			double vx, double vy, double vz)
{
    const double a = WGS84A;
    const double b = WGS84B;
    const double e2 = (a * a - b * b) / (a * a);
    const double e_2 = (a * a - b * b) / (b * b);
    double lambda, p, theta, phi, n, h, vnorth, veast, vup, speed, heading;

    lambda = atan2(y, x);
    /*@ -evalorder @*/
    p = sqrt(pow(x, 2) + pow(y, 2));
    theta = atan2(z * a, p * b);
    phi =
	atan2(z + e_2 * b * pow(sin(theta), 3),
	      p - e2 * a * pow(cos(theta), 3));
    n = a / sqrt(1.0 - e2 * pow(sin(phi), 2));
    h = p / cos(phi) - n;
    h -= wgs84_separation((double)(RAD_2_DEG * phi),
			  (double)(RAD_2_DEG * lambda));
    vnorth =
	-vx * sin(phi) * cos(lambda) - vy * sin(phi) * sin(lambda) +
	vz * cos(phi);
    veast = -vx * sin(lambda) + vy * cos(lambda);
    vup =
	vx * cos(phi) * cos(lambda) + vy * cos(phi) * sin(lambda) +
	vz * sin(phi);
    speed = sqrt(pow(vnorth, 2) + pow(veast, 2));
    heading = atan2(veast, vnorth);
    /*@ +evalorder @*/
    if (heading < 0)
	heading += 2 * GPS_PI;

    /* North and East position fields */
    (void)wattrset(mid2win, A_UNDERLINE);
    (void)wmove(mid2win, 1, 40);
    (void)wprintw(mid2win, "%9.5f %9.5f %9d",
			(double)(RAD_2_DEG * phi),
			(double)(RAD_2_DEG * lambda),
			(int)h);
    (void)mvwaddch(mid2win, 1, 49, ACS_DEGREE);
    (void)mvwaddch(mid2win, 1, 59, ACS_DEGREE);

    /* North and East velocity fields */
    (void)wmove(mid2win, 2, 40);
    (void)wprintw(mid2win, "%9.1f %9.1f %9.1f", vnorth, veast, vup);

    /* heading and speed fields */
    (void)wmove(mid2win, 3, 54);
    (void)wprintw(mid2win, "%5.1f %9.1f", (double)(RAD_2_DEG * heading), speed);
    (void)mvwaddch(mid2win, 3, 59, ACS_DEGREE);
    (void)wattrset(mid2win, A_NORMAL);
}

/*@ -globstate */
static void sirf_update(void)
{
    int i, j, ch, off, cn;
    unsigned char *buf;
    size_t len;
    uint8_t dgps;
    char tbuf[JSON_DATE_MAX+1];

    assert(mid27win != NULL);
    buf = session.packet.outbuffer + 4;
    len = session.packet.outbuflen - 8;
    switch (buf[0]) {
    case 0x02:			/* Measured Navigation Data */
	(void)wmove(mid2win, 1, 6);	/* ECEF position */
	(void)wprintw(mid2win, "%8d %8d %8d", getbes32(buf, 1),
		      getbes32(buf, 5), getbes32(buf, 9));
	(void)wmove(mid2win, 2, 6);	/* ECEF velocity */
	(void)wprintw(mid2win, "%8.1f %8.1f %8.1f",
		      (double)getbes16(buf, 13) / 8, (double)getbes16(buf,
								    15) / 8,
		      (double)getbes16(buf, 17) / 8);
	decode_ecef((double)getbes32(buf, 1), (double)getbes32(buf, 5),
		    (double)getbes32(buf, 9), (double)getbes16(buf, 13) / 8,
		    (double)getbes16(buf, 15) / 8, (double)getbes16(buf,
								  17) / 8);
	/* line 3 */
	(void)wmove(mid2win, 3, 7);
	(void)wprintw(mid2win, "%-24s",
			unix_to_iso8601(session.gpsdata.fix.time, tbuf, sizeof(tbuf))
			);
	(void)wmove(mid2win, 3, 38);
	(void)wattrset(mid2win, A_UNDERLINE);
	if (ppstime_enabled)
	   (void)wprintw(mid2win, "%02d", leapseconds);
	else
	   (void)wprintw(mid2win, "??");
	(void)wattrset(mid2win, A_NORMAL);
	/* line 4 */
	(void)wmove(mid2win, 4, 59);
	(void)wprintw(mid2win, "%4.1f", (double)getub(buf, 20) / 5);	/* HDOP */
	(void)wmove(mid2win, 4, 69);
	(void)wprintw(mid2win, "%02x", getub(buf, 19));	/* Mode 1 */
	(void)wmove(mid2win, 4, 77);
	(void)wprintw(mid2win, "%02x", getub(buf, 21));	/* Mode 2 */
	(void)wmove(mid2win, 4, 7);
	nfix = (int)getub(buf, 28);
	(void)wprintw(mid2win, "%d = ", nfix);	/* SVs in fix */
	for (i = 0; i < MAXSATS; i++) {	/* SV list */
	    if (i < nfix)
		(void)wprintw(mid2win, "%3d", fix[i] =
			      (int)getub(buf, 29 + i));
	    else
		(void)wprintw(mid2win, "   ");
	}
	monitor_log("MND 0x02=");
	break;

    case 0x04:			/* Measured Tracking Data */
	ch = (int)getub(buf, 7);
	for (i = 0; i < ch; i++) {
	    int sv, st;

	    off = 8 + 15 * i;
	    (void)wmove(mid4win, i + 2, 3);
	    sv = (int)getub(buf, off);
	    (void)wprintw(mid4win, " %3d", sv);

	    (void)wprintw(mid4win, " %3d%3d %04x",
			  ((int)getub(buf, off + 1) * 3) / 2, (int)getub(buf,
									 off +
									 2) /
			  2, (int)getbes16(buf, off + 3));

	    st = ' ';
	    if ((int)getbeu16(buf, off + 3) == 0xbf)
		st = 'T';
	    for (j = 0; j < nfix; j++)
		if (sv == fix[j]) {
		    st = 'N';
		    break;
		}

	    cn = 0;

	    for (j = 0; j < 10; j++)
		cn += (int)getub(buf, off + 5 + j);

	    (void)wprintw(mid4win, "%5.1f %c", (double)cn / 10, st);

	    if (sv == 0)	/* not tracking? */
		(void)wprintw(mid4win, "   ");	/* clear other info */
	}
	monitor_log("MTD 0x04=");
	break;

#ifdef __UNUSED__
    case 0x05:			/* raw track data */
	for (off = 1; off < len; off += 51) {
	    ch = getbeu32(buf, off);
	    (void)wmove(mid4win, ch + 2, 19);
	    cn = 0;

	    for (j = 0; j < 10; j++)
		cn += getub(buf, off + 34 + j);

	    printw("%5.1f", (double)cn / 10);

	    printw("%9d%3d%5d", getbeu32(buf, off + 8),
		   (int)getbeu16(buf, off + 12), (int)getbeu16(buf, off + 14));
	    printw("%8.5f %10.5f", (double)getbeu32(buf, off + 16) / 65536,
		   (double)getbeu32(buf, off + 20) / 1024);
	}
	monitor_log("RTD 0x05=");
	break;
#endif /* __UNUSED */

    case 0x06:			/* firmware version */
	display(mid6win, 1, 1, "%s", buf + 1);
	monitor_log("FV  0x06=");
	break;

    case 0x07:			/* Response - Clock Status Data */
	display(mid7win, 1, 5, "%2d", getub(buf, 7));	/* SVs */
	display(mid7win, 1, 16, "%lu", getbeu32(buf, 8));	/* Clock drift */
	display(mid7win, 1, 29, "%lu", getbeu32(buf, 12));	/* Clock Bias */
	display(mid7win, 2, 21, "%lu", getbeu32(buf, 16));	/* Estimated Time */
	monitor_log("CSD 0x07=");
	break;

    case 0x08:			/* 50 BPS data */
	ch = (int)getub(buf, 1);
	display(mid4win, ch + 2, 27, "Y");
	subframe_enabled = true;
	monitor_log("50B 0x08=");
	break;

    case 0x09:			/* Throughput */
	display(mid9win, 1, 6, "%.3f", (double)getbeu16(buf, 1) / 186);	/*SegStatMax */
	display(mid9win, 1, 18, "%.3f", (double)getbeu16(buf, 3) / 186);	/*SegStatLat */
	display(mid9win, 1, 31, "%.3f", (double)getbeu16(buf, 5) / 186);	/*SegStatTime */
	display(mid9win, 1, 42, "%3d", (int)getbeu16(buf, 7));	/* Last Millisecond */
	monitor_log("THR 0x09=");
	break;

    case 0x0b:			/* Command Acknowledgement */
	monitor_log("ACK 0x0b=");
	break;

    case 0x0c:			/* Command NAcknowledgement */
	monitor_log("NAK 0x0c=");
	break;

    case 0x0d:			/* Visible List */
	display(mid13win, 1, 1, "%02d =                                            ",
				 getub(buf, 1));
	(void)wmove(mid13win, 1, 5);
	for (i = 0; i < MAXSATS; i++)
	    (void)wprintw(mid13win, " %2d", getub(buf, 2 + 5 * i));
	if (MAXSATS < (int)getub(buf, 1))
	    (void)wprintw(mid13win, " ...");
	monitor_log("VL  0x0d=");
	break;

    case 0x13:
#define YESNO(n)	(((int)getub(buf, n) != 0)?'Y':'N')
	display(mid19win, 1, 20, "%d", getub(buf, 5));	/* Alt. hold mode */
	display(mid19win, 2, 20, "%d", getub(buf, 6));	/* Alt. hold source */
	display(mid19win, 3, 20, "%dm", (int)getbeu16(buf, 7));	/* Alt. source input */
	if (getub(buf, 9) != (uint8_t) '\0')
	    display(mid19win, 4, 20, "%dsec", getub(buf, 10));	/* Degraded timeout */
	else
	    display(mid19win, 4, 20, "N/A   ");
	display(mid19win, 5, 20, "%dsec", getub(buf, 11));	/* DR timeout */
	display(mid19win, 6, 20, "%c", YESNO(12));	/* Track smooth mode */
	display(mid19win, 7, 20, "%c", YESNO(13));	/* Static Nav. */
	display(mid19win, 8, 20, "0x%x", getub(buf, 14));	/* 3SV Least Squares */
	display(mid19win, 9, 20, "0x%x", getub(buf, 19));	/* DOP Mask mode */
	display(mid19win, 10, 20, "0x%x", (int)getbeu16(buf, 20));	/* Nav. Elev. mask */
	display(mid19win, 11, 20, "0x%x", getub(buf, 22));	/* Nav. Power mask */
	display(mid19win, 12, 20, "0x%x", getub(buf, 27));	/* DGPS Source */
	display(mid19win, 13, 20, "0x%x", getub(buf, 28));	/* DGPS Mode */
	display(mid19win, 14, 20, "%dsec", getub(buf, 29));	/* DGPS Timeout */
	display(mid19win, 1, 42, "%c", YESNO(34));	/* LP Push-to-Fix */
	display(mid19win, 2, 42, "%dms", getbeu32(buf, 35));	/* LP On Time */
	display(mid19win, 3, 42, "%d", getbeu32(buf, 39));	/* LP Interval */
	display(mid19win, 4, 42, "%c", YESNO(43));	/* User Tasks enabled */
	display(mid19win, 5, 42, "%d", getbeu32(buf, 44));	/* User Task Interval */
	display(mid19win, 6, 42, "%c", YESNO(48));	/* LP Power Cycling Enabled */
	display(mid19win, 7, 42, "%d", getbeu32(buf, 49));	/* LP Max Acq Search Time */
	display(mid19win, 8, 42, "%d", getbeu32(buf, 53));	/* LP Max Off Time */
	display(mid19win, 9, 42, "%c", YESNO(57));	/* APM Enabled */
	display(mid19win, 10, 42, "%d", (int)getbeu16(buf, 58));	/* # of fixes */
	display(mid19win, 11, 42, "%d", (int)getbeu16(buf, 60));	/* Time Between fixes */
	display(mid19win, 12, 42, "%d", getub(buf, 62));	/* H/V Error Max */
	display(mid19win, 13, 42, "%d", getub(buf, 63));	/* Response Time Max */
	display(mid19win, 14, 42, "%d", getub(buf, 64));	/* Time/Accu & Duty Cycle Priority */
#undef YESNO
	monitor_log("NP  0x13=");
	break;

    case 0x1b:
	/******************************************************************
	 Not actually documented in any published materials.
	 Here is what Chris Kuethe got from the SiRF folks,
	 (plus some corrections from the GpsPaSsion forums):

	Start of message
	----------------
	Message ID          1 byte    27
	Correction Source   1 byte    0=None, 1=SBAS, 2=Serial, 3=Beacon,
	4=Software

	total:              2 bytes

	Middle part of message varies if using beacon or other:
	-------------------------------------------------------
	If Beacon:
	Receiver Freq Hz    4 bytes
	Bit rate BPS        1 byte
	Status bit map      1 byte    01=Signal Valid,
				      02=Auto frequency detect
				      04=Auto bit rate detect
	Signal Magnitude    4 bytes   Note: in internal units
	Signal Strength dB  2 bytes   derived from Signal Magnitude
	SNR  dB             2 bytes

	total:             14 bytes

	If Not Beacon:
	Correction Age[12]  1 byte x 12  Age in seconds in same order as follows
	Reserved            2 bytes

	total:             14 bytes

	End of Message
	--------------
	Repeated 12 times (pad with 0 if less than 12 SV corrections):
	SVID                1 byte
	Correction (cm)     2 bytes (signed short)

	total               3 x 12 = 36 bytes
	******************************************************************/
	dgps = getub(buf, 1);
	/*@ -type @*/
	display(mid27win, 1, 1, "%8s =                                      ",
		(CHECK_RANGE(dgpsvec, dgps) ? dgpsvec[dgps] : "???"));
	(void)wmove(mid27win, 1, 12);
	for (i = 0; i < MAXSATS; i++)
	    if (getub(buf, 16 + 3 * i) != '\0')
		(void)wprintw(mid27win, " %2d", getub(buf, 16+3*i));
	/*@ +type @*/
	monitor_log("DST 0x1b=");
	break;

    case 0x1c:			/* NL Measurement Data */
    case 0x1d:			/* NL DGPS Data */
    case 0x1e:			/* NL SV State Data */
    case 0x1f:			/* NL Initialized Data */
	subframe_enabled = true;
	monitor_log("NL  0x%02x=", buf[0]);
	break;

    case 0x29:			/* Geodetic Navigation Message */
	monitor_log("GNM 0x29=");
	break;

    case 0x32:			/* SBAS Parameters */
	monitor_log("SBP 0x32=");
	break;

    case 0x34:			/* PPS Time */
	ppstime_enabled = true;
	leapseconds = (int)getbeu16(buf, 8);
	monitor_log("PPS 0x34=");
	break;

#ifdef __UNUSED__
    case 0x62:
	attrset(A_BOLD);
	move(2, 40);
	printw("%9.5f %9.5f", (double)(RAD_2_DEG * 1e8 * getbes32(buf, 1)),
	       (double)(RAD_2_DEG * 1e8 * getbes32(buf, 5)));
	move(2, 63);
	printw("%8d", getbes32(buf, 9) / 1000);

	move(3, 63);

	printw("%8.1f", (double)getbes32(buf, 17) / 1000);

	move(4, 54);
	if (getbeu32(buf, 13) > 50) {
	    double heading = RAD_2_DEG * 1e8 * getbes32(buf, 21);
	    if (heading < 0)
		heading += 360;
	    printw("%5.1f", heading);
	} else
	    printw("  0.0");

	move(4, 63);
	printw("%8.1f", (double)getbes32(buf, 13) / 1000);
	attrset(A_NORMAL);

	move(5, 13);
	printw("%04d-%02d-%02d %02d:%02d:%02d.%02d",
	       (int)getbeu16(buf, 26), getub(buf, 28), getub(buf, 29),
	       getub(buf, 30), getub(buf, 31), (unsigned short)getbeu16(buf,
								       32) /
	       1000, ((unsigned short)getbeu16(buf, 32) % 1000) / 10);
	{
	    struct timeval clk, gps;
	    struct tm tm;

	    gettimeofday(&clk, NULL);

	    memset(&tm, 0, sizeof(tm));
	    tm.tm_sec = (unsigned short)getbeu16(buf, 32) / 1000;
	    tm.tm_min = (int)getub(buf, 31);
	    tm.tm_hour = (int)getub(buf, 30);
	    tm.tm_mday = (int)getub(buf, 29);
	    tm.tm_mon = (int)getub(buf, 28) - 1;
	    tm.tm_year = (int)getbeu16(buf, 26) - 1900;

	    gps.tv_sec = mkgmtime(&tm);
	    gps.tv_usec =
		(((unsigned short)getbeu16(buf, 32) % 1000) / 10) * 10000;

	    move(5, 2);
	    printw("           ");
	    move(5, 2);
#if 1
	    printw("%ld", (gps.tv_usec - clk.tv_usec) +
		   ((gps.tv_sec - clk.tv_sec) % 3600) * 1000000);
#else
	    printw("%ld %ld %ld %ld", gps.tv_sec % 3600, gps.tv_usec,
		   clk.tv_sec % 3600, clk.tv_usec);
#endif
	}
	monitor_log("??? 0x62=");
	break;
#endif /* __UNUSED__ */

    case 0xff:			/* Development Data */
	/*@ +ignoresigns @*/
	while (len > 0 && buf[len - 1] == '\n')
	    len--;
	while (len > 0 && buf[len - 1] == ' ')
	    len--;
	/*@ -ignoresigns @*/
	buf[len] = '\0';
	j = 1;
	for (i = 0; verbpat[i] != NULL; i++)
	    if (strncmp((char *)(buf + 1), verbpat[i], strlen(verbpat[i])) ==
		0) {
		j = 0;
		break;
	    }
	if (j != 0)
	    monitor_log("%s\n", buf + 1);
	monitor_log("DD  0xff=");
	break;

    default:
	monitor_log("UNK 0x%02x=", buf[0]);
	break;
    }

#ifdef CONTROLSEND_ENABLE
    /* elicit navigation parameters */
    if (dispmode && (time(NULL) % 10 == 0)) {
	(void)monitor_control_send((unsigned char *)"\x98\x00", 2);
    }
#endif /* CONTROLSEND_ENABLE */

    /*@ -nullpass -nullderef @*/
    if (dispmode) {
	(void)touchwin(mid19win);
	(void)wnoutrefresh(mid19win);
    }
    /*@ +nullpass -nullderef @*/
}

/*@ +globstate */

#ifdef CONTROLSEND_ENABLE
static int sirf_command(char line[])
{
    unsigned char buf[BUFSIZ];
    int v;

    switch (line[0]) {
    case 'A':			/* toggle 50bps subframe data */
	(void)memset(buf, '\0', sizeof(buf));
	putbyte(buf, 0, 0x80);
	putbyte(buf, 23, 0x0c);
	putbyte(buf, 24, subframe_enabled ? 0x00 : 0x10);
	(void)monitor_control_send(buf, 25);
	/*
	 * The subframe_enabled off because we're counting on receipt of
	 * a subframe packet to turn it on if the command succeeds.
	 */
	subframe_enabled = false;
	return COMMAND_MATCH;

    case 'M':			/* static navigation */
	putbyte(buf, 0, 0x8f);	/* id */
	putbyte(buf, 1, atoi(line + 1));
	(void)monitor_control_send(buf, 2);
	return COMMAND_MATCH;

    case 'D':			/* MID 4 rate change (undocumented) */
	v = atoi(line + 1);
	if (v > 30)
	    return COMMAND_MATCH;
	putbyte(buf, 0, 0xa6);
	putbyte(buf, 1, 0);
	putbyte(buf, 2, 4);	/* satellite picture */
	putbyte(buf, 3, v);
	putbyte(buf, 4, 0);
	putbyte(buf, 5, 0);
	putbyte(buf, 6, 0);
	putbyte(buf, 7, 0);
	(void)monitor_control_send(buf, 8);
	return COMMAND_MATCH;

    case 'P':			/* poll navigation params */
	dispmode = !dispmode;
	if (dispmode) {
	    (void)syncok(mid6win, false);
	    (void)syncok(mid7win, false);
	    (void)syncok(mid9win, false);
	    (void)syncok(mid13win, false);
	    (void)syncok(mid27win, false);
	} else {
	    (void)syncok(mid6win, true);
	    (void)wsyncup(mid6win);
	    (void)syncok(mid7win, true);
	    (void)wsyncup(mid7win);
	    (void)syncok(mid9win, true);
	    (void)wsyncup(mid9win);
	    (void)syncok(mid13win, true);
	    (void)wsyncup(mid13win);
	    (void)syncok(mid27win, true);
	    (void)wsyncup(mid27win);
	}
	return COMMAND_MATCH;
    }

    return COMMAND_UNKNOWN;	/* no match */
}
#endif /* CONTROLSEND_ENABLE */

static void sirf_wrap(void)
{
    (void)delwin(mid2win);
    (void)delwin(mid4win);
    (void)delwin(mid6win);
    (void)delwin(mid7win);
    (void)delwin(mid9win);
    (void)delwin(mid13win);
    (void)delwin(mid19win);
    (void)delwin(mid27win);
}

const struct monitor_object_t sirf_mmt = {
    .initialize = sirf_initialize,
    .update = sirf_update,
#ifdef CONTROLSEND_ENABLE
    .command = sirf_command,
#else
    .command = NULL,
#endif /* CONTROLSEND_ENABLE */
    .wrap = sirf_wrap,
    .min_y = 22,.min_x = 80,
    .driver = &sirf_binary,
};
#endif /* defined(SIRF_ENABLE) && defined(BINARY_ENABLE) */

/* sirfmon.c ends here */
