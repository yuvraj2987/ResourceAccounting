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
//#define IPV6_ENABLE 1 //FARSHAD: commented

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
