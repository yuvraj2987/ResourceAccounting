/* gpsd.h -- fundamental types and structures for the gpsd library
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */

#ifndef _GPSD_H_
#define _GPSD_H_

#include <stdbool.h>
#include <stdio.h>

#ifndef GPSD_CONFIG_H
/* Feature configuration switches begin here */

/* gpsd_config.h.  Generated by scons, do not hand-hack.  */

#define VERSION "3.0"

#define HAVE_DAEMON 1

/* #undef HAVE_STRLCPY */

/* #undef HAVE_STRLCAT */

#define HAVE_LIBUSB 1

#define HAVE_LIBRT 1

/* #undef HAVE_DBUS */

#define HAVE_BLUEZ 1

#define HAVE_SYS_TIMEPPS_H 1

/* AIVDM support */
#define AIVDM_ENABLE 1

/* Ashtech support */
#define ASHTECH_ENABLE 1

/* application binaries directory */
#define BINDIR "/bin"

/* BlueZ support for Bluetooth devices */
#define BLUEZ_ENABLE 1

/* float ops are cheap, compute error estimates */
#define CHEAPFLOATS_ENABLE 1

/* client debugging support */
#define CLIENTDEBUG_ENABLE 1

/* control socket for hotplug notifications */
#define CONTROL_SOCKET_ENABLE 1

/* allow gpsctl/gpsmon to change device settings */
#define CONTROLSEND_ENABLE 1

/* enable DBUS export support */
/* #undef DBUS_EXPORT_ENABLE */

/* include debug information in build */
/* #undef DEBUG_ENABLE */

/* documents directory */
#define DOCDIR "/share/doc"

/* DeLorme EarthMate Zodiac support */
#define EARTHMATE_ENABLE 1

/* EverMore binary support */
#define EVERMORE_ENABLE 1

/* fixed serial port speed */
/* #undef FIXED_PORT_SPEED */

/* fixed serial port stop bits */
/* #undef FIXED_STOP_BITS */

/* San Jose Navigation FV-18 support */
#define FV18_ENABLE 1

/* Garmin kernel driver support */
#define GARMIN_ENABLE 1

/* Garmin Simple Text support */
#define GARMINTXT_ENABLE 1

/* Geostar Protocol support */
#define GEOSTAR_ENABLE 1

/* GPSClock support */
#define GPSCLOCK_ENABLE 1

/* privilege revocation group */
/* #undef GPSD_GROUP */

/* privilege revocation user */
/* #undef GPSD_USER */

/* header file directory */
#define INCLUDEDIR "/include"

/* build IPv6 support */
#define IPV6_ENABLE 1

/* iTrax hardware support */
#define ITRAX_ENABLE 1

/* build QT bindings */
#define LIBQGPSMM_ENABLE 1

/* system libraries */
#define LIBDIR "/lib"

/* build C++ bindings */
#define LIBGPSMM_ENABLE 1

/* maximum allowed clients */
/* #undef LIMITED_MAX_CLIENTS */

/* maximum allowed devices */
/* #undef LIMITED_MAX_DEVICES */

/* manual pages directory */
#define MANDIR "/share/man"

/* MTK-3301 support */
#define MTK3301_ENABLE 1

/* Navcom support */
#define NAVCOM_ENABLE 1

/* build with ncurses */
#define NCURSES_ENABLE 1

/* build support for handling TCP/IP data sources */
#define NETFEED_ENABLE 1

/* NMEA support */
#define NMEA_ENABLE 1

/* NTP time hinting support */
#define NTPSHM_ENABLE 1

/* NTRIP support */
#define NTRIP_ENABLE 1

/* OceanServer support */
#define OCEANSERVER_ENABLE 1

/* oldstyle (pre-JSON) protocol support */
#define OLDSTYLE_ENABLE 1

/* Motorola OnCore chipset support */
#define ONCORE_ENABLE 1

/* build support for passing through JSON */
#define PASSTHROUGH_ENABLE 1

/* PPS time syncing support */
#define PPS_ENABLE 1

/* PPS pulse on CTS rather than DCD */
/* #undef PPS_ON_CTS_ENABLE */

/* installation directory prefix */
#define PREFIX "/usr/local"

/* build with profiling enabled */
/* #undef PROFILING_ENABLE */

/* allow gpsd to change device settings */
#define RECONFIGURE_ENABLE 1

/* rtcm104v2 support */
#define RTCM104V2_ENABLE 1

/* rtcm104v3 support */
#define RTCM104V3_ENABLE 1

/* system binaries directory */
#define SBINDIR "/sbin"

/* build shared libraries, not static */
#define SHARED_ENABLE 1

/* export via shared memory */
#define SHM_EXPORT_ENABLE 1

/* SiRF chipset support */
#define SIRF_ENABLE 1

/* data export over sockets */
#define SOCKET_EXPORT_ENABLE 1

/* squelch gpsd_report/gpsd_hexdump to save cpu */
/* #undef SQUELCH_ENABLE */

/* Novatel SuperStarII chipset support */
#define SUPERSTAR2_ENABLE 1

/* system configuration directory */
#define SYSCONFDIR "/etc"

/* systemd socket activation */
/* #undef SYSTEMD_ENABLE */

/* cross-development target */
/* #undef TARGET */

/* latency timing support */
#define TIMING_ENABLE 1

/* True North Technologies support */
#define TNT_ENABLE 1

/* DeLorme TripMate support */
#define TRIPMATE_ENABLE 1

/* Trimble TSIP support */
#define TSIP_ENABLE 1

/* UBX Protocol support */
#define UBX_ENABLE 1

/* libusb support for USB devices */
#define USB_ENABLE 1


/* will not handle pre-Intel Apples that can run big-endian */
#if defined __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#else
#undef WORDS_BIGENDIAN
#endif

/* Some libcs do not have strlcat/strlcpy. Local copies are provided */
#ifndef HAVE_STRLCAT
# ifdef __cplusplus
extern "C" {
# endif
size_t strlcat(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
# ifdef __cplusplus
}
# endif
#endif
#ifndef HAVE_STRLCPY
# ifdef __cplusplus
extern "C" {
# endif
size_t strlcpy(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
# ifdef __cplusplus
}
# endif
#endif

#define GPSD_CONFIG_H

/* Feature configuration switches end here
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */
#endif /* GPSD_CONFIG_H */

#include <termios.h>
#include <stdint.h>
#include "gps.h"

#ifdef _WIN32
typedef unsigned int speed_t;
#endif

/*
 * Constants for the VERSION response
 * 3.1: Base JSON version
 * 3.2: Added POLL command and response
 * 3.3: AIS app_id split into DAC and FID
 * 3.4: Timestamps change from seconds since Unix epoch to ISO8601.
 * 3.5: POLL subobject name changes: fixes -> tpv, skyview -> sky.
 *      DEVICE::activated becomes ISO8601 rather thab real.
 * 3.6  VERSION, WATCH, and DEVICES from slave gpsds get "remote" attribute.
 */
#define GPSD_PROTO_MAJOR_VERSION	3	/* bump on incompatible changes */
#define GPSD_PROTO_MINOR_VERSION	6	/* bump on compatible changes */

#define JSON_DATE_MAX	24	/* ISO8601 timestamp with 2 decimal places */

#ifndef DEFAULT_GPSD_SOCKET
#define DEFAULT_GPSD_SOCKET	"/var/run/gpsd.sock"
#endif 

/* Some internal capabilities depend on which drivers we're compiling. */
#if !defined(NMEA_ENABLE) && (defined(FV18_ENABLE) || defined(MTK3301_ENABLE) || defined(TNT_ENABLE) || defined(OCEANSERVER_ENABLE) || defined(GPSCLOCK_ENABLE))
#define NMEA_ENABLE
#endif
#ifdef EARTHMATE_ENABLE
#define ZODIAC_ENABLE
#endif
#if defined(ZODIAC_ENABLE) || defined(SIRF_ENABLE) || defined(GARMIN_ENABLE) || defined(TSIP_ENABLE) || defined(EVERMORE_ENABLE) || defined(ITRAX_ENABLE) || defined(UBX_ENABLE) || defined(SUPERSTAR2_ENABLE) || defined(ONCORE_ENABLE) || defined(GEOSTAR_ENABLE)
#define BINARY_ENABLE
#endif
#if defined(TRIPMATE_ENABLE) || defined(BINARY_ENABLE)
#define NON_NMEA_ENABLE
#endif
#if defined(TNT_ENABLE) || defined(OCEANSERVER_ENABLE)
#define COMPASS_ENABLE
#endif

/* First, declarations for the packet layer... */

/*
 * For NMEA-conforming receivers this is supposed to be 82, but
 * some receivers (TN-200, GSW 2.3.2) emit oversized sentences.
 * The current hog champion is the Trimble BX-960 receiver, which
 * emits a 91-character GGA message.
 */
#define NMEA_MAX	91		/* max length of NMEA sentence */
#define NMEA_BIG_BUF	(2*NMEA_MAX+1)	/* longer than longest NMEA sentence */

/* a few bits of ISGPS magic */
enum isgpsstat_t {
    ISGPS_NO_SYNC, ISGPS_SYNC, ISGPS_SKIP, ISGPS_MESSAGE,
};

#define RTCM_MAX	(RTCM2_WORDS_MAX * sizeof(isgps30bits_t))

/*
 * The packet buffers need to be as long than the longest packet we
 * expect to see in any protocol, because we have to be able to hold
 * an entire packet for checksumming...
 * First we thought it had to be big enough for a SiRF Measured Tracker
 * Data packet (188 bytes). Then it had to be big enough for a UBX SVINFO
 * packet (206 bytes). Now it turns out that a couple of ITALK messages are
 * over 512 bytes. I know we like verbose output, but this is ridiculous.
 */
#define MAX_PACKET_LENGTH	516	/* 7 + 506 + 3 */

/*
 * UTC of second 0 of week 0 of the first rollover period of GPS time.
 * Used to compute UTC from GPS time. Also, the threshold value
 * under which system clock times are considered unreliable. Often,
 * embedded systems come up thinking it's early 1970 and the system
 * clock will report small positive values until the clock is set.  By
 * choosing this as the cutoff, we'll never reject historical GPS logs
 * that are actually valid.
 */
#define GPS_EPOCH	315964800	/* 6 Jan 1981 00:00:00 UTC */

struct gps_packet_t {
    /* packet-getter internals */
    int	type;
#define BAD_PACKET      	-1
#define COMMENT_PACKET  	0
#define NMEA_PACKET     	1
#define AIVDM_PACKET    	2
#define GARMINTXT_PACKET	3
#define MAX_TEXTUAL_TYPE	3	/* increment this as necessary */
#define SIRF_PACKET     	4
#define ZODIAC_PACKET   	5
#define TSIP_PACKET     	6
#define EVERMORE_PACKET 	7
#define ITALK_PACKET    	8
#define GARMIN_PACKET   	9
#define NAVCOM_PACKET   	10
#define UBX_PACKET      	11
#define SUPERSTAR2_PACKET	12
#define ONCORE_PACKET   	13
#define GEOSTAR_PACKET   	14
#define MAX_GPSPACKET_TYPE	14	/* increment this as necessary */
#define RTCM2_PACKET    	15
#define RTCM3_PACKET    	16
#define JSON_PACKET    	    	17
#define TEXTUAL_PACKET_TYPE(n)	((((n)>=NMEA_PACKET) && ((n)<=MAX_TEXTUAL_TYPE)) || (n)==JSON_PACKET)
#define GPS_PACKET_TYPE(n)	(((n)>=NMEA_PACKET) && ((n)<=MAX_GPSPACKET_TYPE))
#define LOSSLESS_PACKET_TYPE(n)	(((n)>=RTCM2_PACKET) && ((n)<=RTCM3_PACKET))
#define PACKET_TYPEMASK(n)	(1 << (n))
#define GPS_TYPEMASK	(((2<<(MAX_GPSPACKET_TYPE+1))-1) &~ PACKET_TYPEMASK(COMMENT_PACKET))
    unsigned int state;
    size_t length;
    unsigned char inbuffer[MAX_PACKET_LENGTH*2+1];
    size_t inbuflen;
    unsigned /*@observer@*/char *inbufptr;
    /* outbuffer needs to be able to hold 4 GPGSV records at once */
    unsigned char outbuffer[MAX_PACKET_LENGTH*2+1];
    size_t outbuflen;
    unsigned long char_counter;		/* count characters processed */
    unsigned long retry_counter;	/* count sniff retries */
    unsigned counter;			/* packets since last driver switch */
    int debug;				/* lexer debug level */
    /*
     * ISGPS200 decoding context.
     *
     * This is not conditionalized on RTCM104_ENABLE because we need to
     * be able to build gpsdecode even when RTCM support is not
     * configured in the daemon.
     */
    struct {
	bool            locked;
	int             curr_offset;
	isgps30bits_t   curr_word;
	unsigned int    bufindex;
	/*
	 * Only these should be referenced from elsewhere, and only when
	 * RTCM_MESSAGE has just been returned.
	 */
	isgps30bits_t   buf[RTCM2_WORDS_MAX];   /* packet data */
	size_t          buflen;                 /* packet length in bytes */
    } isgps;
#ifdef PASSTHROUGH_ENABLE
    unsigned int json_depth;
    unsigned int json_after;
#endif /* PASSTHROUGH_ENABLE */
};

extern void packet_init(/*@out@*/struct gps_packet_t *);
extern void packet_reset(/*@out@*/struct gps_packet_t *);
extern void packet_pushback(struct gps_packet_t *);
extern void packet_parse(struct gps_packet_t *);
extern ssize_t packet_get(int, struct gps_packet_t *);
extern int packet_sniff(struct gps_packet_t *);
#define packet_buffered_input(lexer) ((lexer)->inbuffer + (lexer)->inbuflen - (lexer)->inbufptr)

extern void isgps_init(/*@out@*/struct gps_packet_t *);
enum isgpsstat_t isgps_decode(struct gps_packet_t *,
			      bool (*preamble_match)(isgps30bits_t *),
			      bool (*length_check)(struct gps_packet_t *),
			      size_t,
			      unsigned int);
extern unsigned int isgps_parity(isgps30bits_t);
extern void isgps_output_magnavox(const isgps30bits_t *, unsigned int, FILE *);

extern enum isgpsstat_t rtcm2_decode(struct gps_packet_t *, unsigned int);
extern void json_rtcm2_dump(const struct rtcm2_t *,
			    /*@null@*/const char *, /*@out@*/char[], size_t);
extern void rtcm2_unpack(/*@out@*/struct rtcm2_t *, char *);
extern void json_rtcm3_dump(const struct rtcm3_t *,
			    /*@null@*/const char *, /*@out@*/char[], size_t);
extern void rtcm3_unpack(/*@out@*/struct rtcm3_t *, char *);

/* Next, declarations for the core library... */

/* factors for converting among confidence interval units */
#define CEP50_SIGMA	1.18
#define DRMS_SIGMA	1.414
#define CEP95_SIGMA	2.45

/* this is where we choose the confidence level to use in reports */
#define GPSD_CONFIDENCE	CEP95_SIGMA

#define NTPSHMSEGS	4		/* number of NTP SHM segments */

#define AIVDM_CHANNELS	2		/* A, B */

struct gps_context_t {
    int valid;				/* member validity flags */
    int debug;				/* dehug verbosity level */
    bool readonly;			/* if true, never write to device */
#define LEAP_SECOND_VALID	0x01	/* we have or don't need correction */
#define GPS_TIME_VALID  	0x02	/* GPS week/tow is valid */
    /* DGPS status */
    int fixcnt;				/* count of good fixes seen */
    size_t rtcmbytes;			/* byte count of last RTCM104 report */
    char rtcmbuf[RTCM_MAX];		/* last RTCM104 report */
    timestamp_t rtcmtime;		/* timestamp of last RTCM104 report */
    /* timekeeping */
    time_t start_time;			/* local time of daemon startup */
    int leap_seconds;			/* Unix seconds to UTC */
    unsigned short gps_week;            /* GPS week, actually 10 bits */
    double gps_tow;                     /* GPS time of week, actually 19 bits */
    int century;			/* for NMEA-only devices without ZDA */
    int rollovers;			/* rollovers since start of run */
#ifdef NTPSHM_ENABLE
    bool enable_ntpshm;
    /* we need the volatile here to tell the C compiler not to
     * 'optimize' as 'dead code' the writes to SHM */
    /*@reldef@*/volatile struct shmTime *shmTime[NTPSHMSEGS];
    bool shmTimeInuse[NTPSHMSEGS];
# ifdef PPS_ENABLE
    bool shmTimePPS;
# endif /* PPS_ENABLE */
#endif /* NTPSHM_ENABLE */
#ifdef SHM_EXPORT_ENABLE
    /* we don't want the compiler to treat writes to shmexport as dead code,
     * and we don't want them reordered either */
    /*@reldef@*/volatile char *shmexport;
#endif
};

struct aivdm_context_t {
    /* hold context for decoding AIDVM packet sequences */
    int decoded_frags;		/* for tracking AIDVM parts in a multipart sequence */
    unsigned char bits[2048];
    size_t bitlen; /* how many valid bits */
    unsigned int mmsi24; /* type 24 specific */
    char shipname24[AIS_SHIPNAME_MAXLEN+1]; /* type 24 specific */
};

struct gps_device_t;

#define MODE_NMEA	0
#define MODE_BINARY	1

typedef enum {ANY, GPS, RTCM2, RTCM3, AIS} gnss_type;
typedef enum {
    event_wakeup,
    event_triggermatch,
    event_identified,
    event_configure,
    event_driver_switch,
    event_deactivate,
    event_reactivate,
} event_t;


#define INTERNAL_SET(n)	((gps_mask_t)(1llu<<(SET_HIGH_BIT+(n))))
#define RAW_IS  	INTERNAL_SET(1)	/* raw pseudorange data available */
#define USED_IS 	INTERNAL_SET(2)	/* sat-used count available */
#define DRIVER_IS	INTERNAL_SET(3)	/* driver type identified */
#define CLEAR_IS	INTERNAL_SET(4)	/* starts a reporting cycle */
#define REPORT_IS	INTERNAL_SET(5)	/* ends a reporting cycle */
#define NODATA_IS	INTERNAL_SET(6)	/* no data read from fd */
#define PPSTIME_IS	INTERNAL_SET(7)	/* precision time is available */
#define PERR_IS 	INTERNAL_SET(8)	/* PDOP set */
#define PASSTHROUGH_IS 	INTERNAL_SET(9)	/* passthrough mode */
#define DATA_IS	~(ONLINE_SET|PACKET_SET|CLEAR_IS|REPORT_IS)

typedef /*@unsignedintegraltype@*/ unsigned int driver_mask_t;
#define DRIVER_NOFLAGS	0x00000000u

struct gps_type_t {
/* GPS method table, describes how to talk to a particular GPS type */
    /*@observer@*/char *type_name;
    int packet_type;
    driver_mask_t flags;	/* reserved for expansion */
    /*@observer@*//*@null@*/char *trigger;
    int channels;
    /*@null@*/bool (*probe_detect)(struct gps_device_t *session);
    /*@null@*/ssize_t (*get_packet)(struct gps_device_t *session);
    /*@null@*/gps_mask_t (*parse_packet)(struct gps_device_t *session);
    /*@null@*/ssize_t (*rtcm_writer)(struct gps_device_t *session, const char *rtcmbuf, size_t rtcmbytes);
    /*@null@*/void (*event_hook)(struct gps_device_t *session, event_t event);
#ifdef RECONFIGURE_ENABLE
    /*@null@*/bool (*speed_switcher)(struct gps_device_t *session,
				     speed_t speed, char parity, int stopbits);
    /*@null@*/void (*mode_switcher)(struct gps_device_t *session, int mode);
    /*@null@*/bool (*rate_switcher)(struct gps_device_t *session, double rate);
    double min_cycle;
#endif /* RECONFIGURE_ENABLE */
#ifdef CONTROLSEND_ENABLE
    /*@null@*/ssize_t (*control_send)(struct gps_device_t *session, char *buf, size_t buflen);
#endif /* CONTROLSEND_ENABLE */
#ifdef NTPSHM_ENABLE
    /*@null@*/double (*ntp_offset)(struct gps_device_t *session);
#endif /* NTPSHM_ENABLE */
};

/*
 * Each input source has an associated type.  This is currently used in two
 * ways:
 *
 * (1) To determince if we require that gpsd be the only process opening a
 * device.  We make an exception for PTYs because the master side has to be
 * opened by test code,
 *
 * (2) To determine whether it's safe to send wakeup strings.  These are
 * required on some unusual RS-232 devices (such as the TNT compass and
 * Thales/Ashtech GPSes) but should not be shipped to unidentified USB
 * or Bluetooth devices as we don't even know in advance those are GPSEs;
 * they might not cope well.
 *
 * Where it says "case detected but not used" it means that we can identify
 * a source type but no behavior is yet contingent on it.  A "discoverable"
 * device is one for which there is discoverable metadata such as a
 * vendor/product ID.
 *
 * We should never see a block device; that would indicate a serious error
 * in command-line usage or the hotplug system.
 */
typedef enum {source_unknown,
	      source_blockdev,	/* block devices can't be GPS sources */
	      source_rs232,	/* potential GPS source, not discoverable */
	      source_usb,	/* potential GPS source, discoverable */
	      source_bluetooth,	/* potential GPS source, discoverable */
	      source_pty,	/* PTY: we don't require exclusive access */
	      source_tcp,	/* TCP/IP stream: case detected but not used */
	      source_udp,	/* UDP stream: case detected but not used */
	      source_gpsd,	/* Remote gpsd instance over TCP/IP */
} sourcetype_t;

/*
 * Each input source also has an associated service type.
 */
typedef enum {service_unknown,
	      service_sensor,
	      service_dgpsip,
	      service_ntrip
} servicetype_t;

/*
 * Private state information about an NTRIP stream.
 */
struct ntrip_stream_t
{
    char mountpoint[101];
    char credentials[128];
    char authStr[128];
    char url[256];
    char port[32]; /* in my /etc/services 16 was the longest */
    bool set; /* found and set */
    enum
    {
	fmt_rtcm2,
	fmt_rtcm2_0,
	fmt_rtcm2_1,
	fmt_rtcm2_2,
	fmt_rtcm2_3,
	fmt_rtcm3,
	fmt_unknown
    } format;
    int carrier;
    double latitude;
    double longitude;
    int nmea;
    enum
    { cmp_enc_none, cmp_enc_unknown } compr_encryp;
    enum
    { auth_none, auth_basic, auth_digest, auth_unknown } authentication;
    int fee;
    int bitrate;
};

struct gps_device_t {
/* session object, encapsulates all global state */
    struct gps_data_t gpsdata;
    /*@relnull@*/const struct gps_type_t *device_type;
    struct gps_context_t	*context;
    sourcetype_t sourcetype;
    servicetype_t servicetype;
    timestamp_t rtcmtime;	/* timestamp of last RTCM104 correction to GPS */
#ifndef _WIN32
    struct termios ttyset, ttyset_old;
#endif
#ifndef FIXED_PORT_SPEED
    unsigned int baudindex;
#endif /* FIXED_PORT_SPEED */
    int saved_baud;
    struct gps_packet_t packet;
    int getcount;
    int subframe_count;
    char subtype[64];			/* firmware version or subtype ID */
    timestamp_t opentime;
    timestamp_t releasetime;
    bool zerokill;
    timestamp_t reawake;
#ifdef NTPSHM_ENABLE
    int shmindex;
    timestamp_t last_fixtime;		/* so updates happen once */
    bool ship_to_ntpd;
# ifdef PPS_ENABLE
    int shmTimeP;
# endif /* PPS_ENABLE */
#endif /* NTPSHM_ENABLE */
    double mag_var;			/* magnetic variation in degrees */
    bool back_to_nmea;			/* back to NMEA on revert? */
    char msgbuf[MAX_PACKET_LENGTH*2+1];	/* command message buffer for sends */
    size_t msgbuflen;
    int observed;			/* which packet type`s have we seen? */
    bool cycle_end_reliable;		/* does driver signal REPORT_MASK */
    bool notify_clients;		/* ship DEVICE notification on poll? */
    int fixcnt;				/* count of fixes from this device */
    struct gps_fix_t newdata;		/* where drivers put their data */
    struct gps_fix_t oldfix;		/* previous fix for error modeling */
    /*
     * The rest of this structure is driver-specific private storage.
     * Because the Garmin driver uses a long buffer, you can have
     * up to 4096+12 bytes of private storage in your own union member
     * without making this structure larger or changing the API at all.
     */
    union {
#ifdef NMEA_ENABLE
	struct {
	    int part, await;		/* for tracking GSV parts */
	    struct tm date;		/* date part of last sentence time */
	    double subseconds;		/* subsec part of last sentence time */
	    char *field[NMEA_MAX];
	    unsigned char fieldcopy[NMEA_MAX+1];
	    /* detect receivers that ship GGA with non-advancing timestamp */
	    bool latch_mode;
	    char last_gga_timestamp[16];
	    /*
	     * State for the cycle-tracking machinery.
	     * The reason these timestamps are separate from the
	     * general sentence timestamps is that we can
	     * use the minutes and seconds part of a sentence
	     * with an incomplete timestamp (like GGA) for
	     * end-cycle recognition, even if we don't have a previous
	     * RMC or ZDA that lets us get full time from it.
	     */
	    timestamp_t this_frac_time, last_frac_time;
	    bool latch_frac_time;
	    unsigned int lasttag;
	    unsigned int cycle_enders;
#ifdef GPSCLOCK_ENABLE
	    bool ignore_trailing_edge;
#endif /* GPSCLOCK_ENABLE */
	} nmea;
#endif /* NMEA_ENABLE */
#ifdef GARMINTXT_ENABLE
	struct {
	    struct tm date;		/* date part of last sentence time */
	    double subseconds;		/* subsec part of last sentence time */
	} garmintxt;
#endif /* NMEA_ENABLE */
#ifdef BINARY_ENABLE
#ifdef GEOSTAR_ENABLE
	struct {
	    unsigned int physical_port;
	} geostar;
#endif /* GEOSTAR_ENABLE */
#ifdef SIRF_ENABLE
	struct {
	    unsigned int driverstate;	/* for private use */
#define SIRF_LT_231	0x01		/* SiRF at firmware rev < 231 */
#define SIRF_EQ_231     0x02            /* SiRF at firmware rev == 231 */
#define SIRF_GE_232     0x04            /* SiRF at firmware rev >= 232 */
#define UBLOX   	0x08		/* uBlox firmware with packet 0x62 */
	    unsigned long satcounter;
	    unsigned int time_seen;
#define TIME_SEEN_UTC_2	0x08	/* Seen UTC time variant 2? */
	    /* fields from Navigation Parameters message */
	    bool nav_parameters_seen;	/* have we seen one? */
	    unsigned char altitude_hold_mode;
	    unsigned char altitude_hold_source;
	    int16_t altitude_source_input;
	    unsigned char degraded_mode;
	    unsigned char degraded_timeout;
	    unsigned char dr_timeout;
	    unsigned char track_smooth_mode;
	} sirf;
#endif /* SIRF_ENABLE */
#ifdef SUPERSTAR2_ENABLE
	struct {
	    time_t last_iono;
	} superstar2;
#endif /* SUPERSTAR2_ENABLE */
#ifdef TSIP_ENABLE
	struct {
	    bool superpkt;		/* Super Packet mode requested */
	    time_t last_41;		/* Timestamps for packet requests */
	    time_t last_48;
	    time_t last_5c;
	    time_t last_6d;
	    time_t last_46;
	    time_t req_compact;
	    unsigned int stopbits; /* saved RS232 link parameter */
	    char parity;
	} tsip;
#endif /* TSIP_ENABLE */
#ifdef GARMIN_ENABLE	/* private housekeeping stuff for the Garmin driver */
	struct {
	    unsigned char Buffer[4096+12];	/* Garmin packet buffer */
	    size_t BufferLen;		/* current GarminBuffer Length */
	} garmin;
#endif /* GARMIN_ENABLE */
#ifdef ZODIAC_ENABLE	/* private housekeeping stuff for the Zodiac driver */
	struct {
	    unsigned short sn;		/* packet sequence number */
	    /*
	     * Zodiac chipset channel status from PRWIZCH. Keep it so
	     * raw-mode translation of Zodiac binary protocol can send
	     * it up to the client.
	     */
#define ZODIAC_CHANNELS	12
	    unsigned int Zs[ZODIAC_CHANNELS];	/* satellite PRNs */
	    unsigned int Zv[ZODIAC_CHANNELS];	/* signal values (0-7) */
	} zodiac;
#endif /* ZODIAC_ENABLE */
#ifdef UBX_ENABLE
	struct {
	    bool have_port_configuration;
	    unsigned char original_port_settings[20];
	    unsigned char sbas_in_use;
    	} ubx;
#endif /* UBX_ENABLE */
#ifdef NAVCOM_ENABLE
	struct {
	    uint8_t physical_port;
	    bool warned;
	} navcom;
#endif /* NAVCOM_ENABLE */
#ifdef ONCORE_ENABLE
	struct {
#define ONCORE_VISIBLE_CH 12
	    int visible;
	    int PRN[ONCORE_VISIBLE_CH];		/* PRNs of satellite */
	    int elevation[ONCORE_VISIBLE_CH];	/* elevation of satellite */
	    int azimuth[ONCORE_VISIBLE_CH];	/* azimuth */
	    int pps_offset_ns;
	} oncore;
#endif /* ONCORE_ENABLE */

	/*
	 * This is not conditionalized on RTCM104_ENABLE because we need to
	 * be able to build gpsdecode even when RTCM support is not
	 * configured in the daemon.  It doesn't take up extra space.
	 */
	struct {
	    /* ISGPS200 decoding */
	    bool            locked;
	    int             curr_offset;
	    isgps30bits_t   curr_word;
	    isgps30bits_t   buf[RTCM2_WORDS_MAX];
	    unsigned int    bufindex;
	} isgps;
#endif /* BINARY_ENABLE */
    } driver;
    /*
     * Auxiliary structures for parsing data that can be interleaved with
     * GPS sentences. Can't be in the driver union or it will get stepped on.
     * So far the only case of this is AIS reports, which in marine navigation
     * systems may come over the same wire with GPS NMEA sentences.
     */
#ifdef AIVDM_ENABLE
    struct aivdm_context_t aivdm[AIVDM_CHANNELS];
#endif /* AIVDM_ENABLE */

#ifdef TIMING_ENABLE
    /* profiling data for last sentence */
    char tag[MAXTAGLEN+1];	/* tag of last sentence processed */
    timestamp_t d_xmit_time;		/* beginning of sentence transmission */
    timestamp_t d_recv_time;		/* daemon receipt time (-> E1+T1) */
    timestamp_t d_decode_time;	/* daemon end-of-decode time (-> D1) */
    timestamp_t emit_time;		/* emission time (-> E2) */
#endif /* TIMING_ENABLE */

    /*
     * State of an NTRIP connection.  We don't want to zero this on every
     * activation, otherwise the connection state will get lost.  Information
     * in this substructure is only valid if servicetype is service_ntrip.
     */
    struct {
	/* state information about the stream */
	struct ntrip_stream_t stream;

	/* state information about our response parsing */
	enum {
	    ntrip_conn_init,
	    ntrip_conn_sent_probe,
	    ntrip_conn_sent_get,
	    ntrip_conn_established,
	    ntrip_conn_err
	} conn_state; 	/* connection state for multi stage connect */
	bool works;		/* marks a working connection, so we try to reconnect once */
	bool sourcetable_parse;	/* have we read the sourcetable header? */
    } ntrip;
    /* State of a DGPSIP connection */
    struct {
	bool reported;
    } dgpsip;
};

/* logging levels */
#define LOG_ERROR 	-1	/* errors, display always */
#define LOG_SHOUT	0	/* not an error but we should always see it */
#define LOG_WARN	1	/* not errors but may indicate a problem */
#define LOG_INF 	2	/* key informative messages */
#define LOG_DATA	3	/* log data management messages */
#define LOG_PROG	4	/* progress messages */
#define LOG_IO  	5	/* IO to and from devices */
#define LOG_SPIN	6	/* logging for catching spin bugs */
#define LOG_RAW 	7	/* raw low-level I/O */

#define ISGPS_ERRLEVEL_BASE	LOG_RAW

#define IS_HIGHEST_BIT(v,m)	(v & ~((m<<1)-1))==0

/* here are the available GPS drivers */
extern const struct gps_type_t **gpsd_drivers;

/* gpsd library internal prototypes */
extern gps_mask_t generic_parse_input(struct gps_device_t *);
extern ssize_t generic_get(struct gps_device_t *);

extern gps_mask_t nmea_parse(char *, struct gps_device_t *);
extern ssize_t nmea_write(struct gps_device_t *, char *, size_t);
extern ssize_t nmea_send(struct gps_device_t *, const char *, ... );
extern void nmea_add_checksum(char *);

extern gps_mask_t sirf_parse(struct gps_device_t *, unsigned char *, size_t);
extern gps_mask_t evermore_parse(struct gps_device_t *, unsigned char *, size_t);
extern gps_mask_t navcom_parse(struct gps_device_t *, unsigned char *, size_t);
extern gps_mask_t garmin_ser_parse(struct gps_device_t *);
extern gps_mask_t garmintxt_parse(struct gps_device_t *);
extern gps_mask_t aivdm_parse(struct gps_device_t *);

extern bool netgnss_uri_check(char *);
extern int netgnss_uri_open(struct gps_device_t *, char *);
extern void netgnss_report(struct gps_context_t *,
			 struct gps_device_t *,
			 struct gps_device_t *);
extern void netgnss_autoconnect(struct gps_context_t *, double, double);

extern int dgpsip_open(struct gps_device_t *, const char *);
extern void dgpsip_report(struct gps_context_t *,
			 struct gps_device_t *,
			 struct gps_device_t *);
extern void dgpsip_autoconnect(struct gps_context_t *,
			       double, double, const char *);
extern int ntrip_open(struct gps_device_t *, char *);
extern void ntrip_report(struct gps_context_t *,
			 struct gps_device_t *,
			 struct gps_device_t *);

extern void gpsd_tty_init(struct gps_device_t *);
extern int gpsd_serial_open(struct gps_device_t *);
extern bool gpsd_set_raw(struct gps_device_t *);
extern ssize_t gpsd_write(struct gps_device_t *, const char *, size_t);
extern bool gpsd_next_hunt_setting(struct gps_device_t *);
extern int gpsd_switch_driver(struct gps_device_t *, char *);
extern void gpsd_set_speed(struct gps_device_t *, speed_t, char, unsigned int);
extern speed_t gpsd_get_speed(const struct termios *);
extern void gpsd_assert_sync(struct gps_device_t *);
extern void gpsd_close(struct gps_device_t *);

extern void gpsd_time_init(struct gps_context_t *, time_t);
extern void gpsd_set_century(struct gps_device_t *);
extern timestamp_t gpsd_gpstime_resolve(/*@in@ */ struct gps_device_t *,
			      const unsigned short, const double);
extern timestamp_t gpsd_utc_resolve(/*@in@*/struct gps_device_t *);

extern void gpsd_zero_satellites(/*@out@*/struct gps_data_t *sp)/*@modifies sp@*/;
extern gps_mask_t gpsd_interpret_subframe(struct gps_device_t *, unsigned int,
				uint32_t[]);
extern gps_mask_t gpsd_interpret_subframe_raw(struct gps_device_t *,
				unsigned int, uint32_t[]);
extern /*@ observer @*/ char *gpsd_hexdump(/*@null@*/char *, size_t);
extern int gpsd_hexpack(/*@in@*/const char *, /*@out@*/char *, size_t);
extern int hex2bin(const char *);
extern ssize_t hex_escapes(/*@out@*/char *, const char *);
extern void gpsd_position_fix_dump(struct gps_device_t *,
				   /*@out@*/char[], size_t);
extern void gpsd_clear_data(struct gps_device_t *);
extern socket_t netlib_connectsock(int, const char *, const char *, const char *);
extern socket_t netlib_localsocket(const char *, int);
extern char /*@observer@*/ *netlib_errstr(const int);
extern char /*@observer@*/ *netlib_sock2ip(int);

extern void nmea_tpv_dump(struct gps_device_t *, /*@out@*/char[], size_t);
extern void nmea_sky_dump(struct gps_device_t *, /*@out@*/char[], size_t);
extern void nmea_subframe_dump(struct gps_device_t *, /*@out@*/char[], size_t);

extern void ntpshm_init(struct gps_context_t *, bool);
extern int ntpshm_put(struct gps_device_t *, double, double);
extern void ntpd_link_deactivate(struct gps_device_t *);
extern void ntpd_link_activate(struct gps_device_t *);

extern void ecef_to_wgs84fix(/*@out@*/struct gps_fix_t *,
			     /*@out@*/double *,
			     double, double, double,
			     double, double, double);
extern void clear_dop(/*@out@*/struct dop_t *);

/* shmexport.c */
#define GPSD_KEY	0x47505344	/* "GPSD" */
struct shmexport_t
{
    int bookend1;
    struct gps_data_t gpsdata;
    int bookend2;
};
extern bool shm_acquire(struct gps_context_t *);
extern void shm_release(struct gps_context_t *);
extern void shm_update(struct gps_context_t *, struct gps_data_t *);


/* dbusexport.c */
#if defined(DBUS_EXPORT_ENABLE) && !defined(S_SPLINT_S)
#include <dbus/dbus.h>

int initialize_dbus_connection (void);
void send_dbus_fix (struct gps_device_t* channel);

#endif /* defined(DBUS_EXPORT_ENABLE) && !defined(S_SPLINT_S) */

/* srecord.c */
extern void hexdump(size_t, unsigned char *, unsigned char *);
extern unsigned char sr_sum(unsigned int, unsigned int, unsigned char *);
extern int bin2srec(unsigned int, unsigned int, unsigned int, unsigned char *, unsigned char *);
extern int srec_hdr(unsigned int, unsigned char *, unsigned char *);
extern int srec_fin(unsigned int, unsigned char *);
extern unsigned char hc(unsigned char);

/* application interface */
extern void gps_context_init(struct gps_context_t *context);
extern void gpsd_init(struct gps_device_t *,
		      struct gps_context_t *,
		      /*@null@*/const char *);
extern void gpsd_clear(struct gps_device_t *);
extern int gpsd_open(struct gps_device_t *);
extern int gpsd_activate(struct gps_device_t *);
extern void gpsd_deactivate(struct gps_device_t *);
extern gps_mask_t gpsd_poll(struct gps_device_t *);
extern void gpsd_wrap(struct gps_device_t *);
extern /*@observer@*/const char *gpsd_maskdump(gps_mask_t);

/* exceptional driver methods */
extern bool ubx_write(struct gps_device_t *, unsigned int, unsigned int,
		      /*@null@*/unsigned char *, unsigned short);
extern bool aivdm_decode(const char *, size_t,
			 struct aivdm_context_t [],
			 struct ais_t *, int);

/* debugging apparatus for the client library */
#ifdef CLIENTDEBUG_ENABLE
#define LIBGPS_DEBUG
#endif /* CLIENTDEBUG_ENABLE */
#ifdef LIBGPS_DEBUG
#define DEBUG_CALLS	1	/* shallowest debug level */
#define DEBUG_JSON	5	/* minimum level for verbose JSON debugging */
# define libgps_debug_trace(args) (void) libgps_trace args
extern int libgps_debuglevel;
extern void libgps_dump_state(struct gps_data_t *);
#else
# define libgps_debug_trace(args) /*@i1@*/do { } while (0)
#endif /* LIBGPS_DEBUG */

/* caller should supply this */
# if __GNUC__ >= 3 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
__attribute__((__format__(__printf__, 2, 3))) void gpsd_report(int, const char *, ...);
# else /* not a new enough GCC, use the unprotected prototype */
void gpsd_report(int, const char *, ...);
#endif

#ifdef S_SPLINT_S
extern struct protoent *getprotobyname(const char *);
extern /*@observer@*/char *strptime(const char *,const char *tp,/*@out@*/struct tm *)/*@modifies tp@*/;
extern struct tm *gmtime_r(const time_t *,/*@out@*/struct tm *tp)/*@modifies tp@*/;
extern struct tm *localtime_r(const time_t *,/*@out@*/struct tm *tp)/*@modifies tp@*/;
#endif /* S_SPLINT_S */

/*
 * How to mix together epx and epy to get a horizontal circular error
 * eph when reporting requires it. Most devices don't report these;
 * NMEA 3.x devices reporting $GPGBS are the exception.
 */
#define EMIX(x, y)	(((x) > (y)) ? (x) : (y))

#define NITEMS(x) (int)(sizeof(x)/sizeof(x[0]))

/* Cygwin, in addition to NAN, doesn't have cfmakeraw */
#if defined(__CYGWIN__)
void cfmakeraw(struct termios *);
#endif /* defined(__CYGWIN__) */

#define DEVICEHOOKPATH SYSCONFDIR"/gpsd/device-hook"

/* Needed because 4.x versions of GCC are really annoying */
#define ignore_return(funcall)	assert(funcall != -23)

/* memory barriers */
static /*@unused@*/ inline void barrier(void) {
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#ifndef S_SPLINT_S
	asm volatile("mfence");
#endif /* S_SPLINT_S */
#endif /* defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)) */
}

#endif /* _GPSD_H_ */
// Local variables:
// mode: c
// end:
