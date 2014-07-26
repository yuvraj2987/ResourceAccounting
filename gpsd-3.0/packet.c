/****************************************************************************

NAME:
   packet.c -- a packet-sniffing engine for reading from GPS devices

DESCRIPTION:

Initial conditions of the problem:

1. We have a file descriptor open for (possibly non-blocking) read. The device
   on the other end is sending packets at us.

2. It may require more than one read to gather a packet.  Reads may span packet
   boundaries.

3. There may be leading garbage before the first packet.  After the first
   start-of-packet, the input should be well-formed.

The problem: how do we recognize which kind of packet we're getting?

No need to handle Garmin USB binary, we know that type by the fact we're
connected to the Garmin kernel driver.  But we need to be able to tell the
others apart and distinguish them from baud barf.

PERMISSIONS
   This file is Copyright (c) 2010 by the GPSD project
   BSD terms apply: see the file COPYING in the distribution root for details.

***************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#ifndef S_SPLINT_S
#include <netinet/in.h>
#include <arpa/inet.h>		/* for htons() */
#include <unistd.h>
#endif /* S_SPLINT_S */

#include "bits.h"
#include "gpsd.h"
#include "crc24q.h"

/*
 * The packet-recognition state machine.  This takes an incoming byte stream
 * and tries to segment it into packets.  There are three types of packets:
 *
 * 1) Comments. These begin with # and end with \r\n.
 *
 * 2) NMEA lines.  These begin with $, and with \r\n, and have a checksum.
 *
 * 3) Binary packets.  These begin with some fixed leader character(s),
 *    have a length embedded in them, and end with a checksum (and possibly)
 *    some fixed trailing bytes.
 *
 * 4) ISGPS packets. The input may be a bitstream containing IS-GPS-200
 *    packets.  Each includes a fixed leader byte, a length, and check bits.
 *    In this case, it is not guaranted that packet starts begin on byte
 *    bounaries; the recognizer has to run a separate state machine against
 *    each byte just to achieve synchronization lock with the bitstream.
 *
 * Adding support for a new GPS protocol typically reqires adding state
 * transitions to support whatever binary packet structure it has.  The
 * goal is for the lexer to be able to cope with arbitrarily mixed packet
 * types on the input stream.  This is a requirement because (1) sometimes
 * gpsd wants to switch a device that supports both NMEA and a binary
 * packet protocol to the latter for more detailed reporting, and (b) in
 * the presence of device hotplugging, the type of GPS report coming
 * in is subject to change at any time.
 *
 * Caller should consume a packet when it sees one of the *_RECOGNIZED
 * states.  It's good practice to follow the _RECOGNIZED transition
 * with one that recognizes a leader of the same packet type rather
 * than dropping back to ground state -- this for example will prevent
 * the state machine from hopping between recognizing TSIP and
 * EverMore packets that both start with a DLE.
 *
 * Error handling is brutally simple; any time we see an unexpected
 * character, go to GROUND_STATE and reset the machine (except that a
 * $ in an NMEA payload only resets back to NMEA_DOLLAR state).  Because
 * another good packet will usually be along in less than a second
 * repeating the same data, Boyer-Moore-like attempts to do parallel
 * recognition beyond the headers would make no sense in this
 * application, they'd just add complexity.
 *
 * The NMEA portion of the state machine allows the following talker IDs:
 *      GP -- Global Positioning System.
 *      GL -- GLONASS, according to IEIC 61162-1
 *      GN -- Mixed GPS and GLONASS data, according to IEIC 61162-1
 *      II -- Integrated Instrumentation (Raytheon's SeaTalk system).
 *	IN -- Integrated Navigation (Garmin uses this).
 *
 */

enum
{
#include "packet_states.h"
};

#define SOH	(unsigned char)0x01
#define DLE	(unsigned char)0x10
#define STX	(unsigned char)0x02
#define ETX	(unsigned char)0x03

#ifdef ONCORE_ENABLE
static size_t oncore_payload_cksum_length(unsigned char id1, unsigned char id2)
{
    size_t l;

    /* For the packet sniffer to not terminate the message due to
     * payload data looking like a trailer, the known payload lengths
     * including the checksum are given.  Return -1 for unknown IDs.
     */

#define ONCTYPE(id2,id3) ((((unsigned int)id2)<<8)|(id3))

    /* *INDENT-OFF* */
    switch (ONCTYPE(id1,id2)) {
    case ONCTYPE('A','b'): l = 10; break; /* GMT offset */
    case ONCTYPE('A','w'): l =  8; break; /* time mode */
    case ONCTYPE('A','c'): l = 11; break; /* date */
    case ONCTYPE('A','a'): l = 10; break; /* time of day */
    case ONCTYPE('A','d'): l = 11; break; /* latitude */
    case ONCTYPE('A','e'): l = 11; break; /* longitude */
    case ONCTYPE('A','f'): l = 15; break; /* height */
    case ONCTYPE('E','a'): l = 76; break; /* position/status/data */
    case ONCTYPE('A','g'): l =  8; break; /* satellite mask angle */
    case ONCTYPE('B','b'): l = 92; break; /* visible satellites status */
    case ONCTYPE('B','j'): l =  8; break; /* leap seconds pending */
    case ONCTYPE('A','q'): l =  8; break; /* atmospheric correction mode */
    case ONCTYPE('A','p'): l = 25; break; /* set user datum / select datum */
    /* Command "Ao" gives "Ap" response   (select datum) */
    case ONCTYPE('C','h'): l =  9; break; /* almanac input ("Cb" response) */
    case ONCTYPE('C','b'): l = 33; break; /* almanac output ("Be" response) */
    case ONCTYPE('S','z'): l =  8; break; /* system power-on failure */
    case ONCTYPE('C','j'): l = 294; break; /* receiver ID */
    case ONCTYPE('F','a'): l =  9; break; /* self-test */
    case ONCTYPE('C','f'): l =  7; break; /* set-to-defaults */
    case ONCTYPE('E','q'): l = 96; break; /* ASCII position */
    case ONCTYPE('A','u'): l = 12; break; /* altitide hold height */
    case ONCTYPE('A','v'): l =  8; break; /* altitude hold mode */
    case ONCTYPE('A','N'): l =  8; break; /* velocity filter */
    case ONCTYPE('A','O'): l =  8; break; /* RTCM report mode */
    case ONCTYPE('C','c'): l = 80; break; /* ephemeris data input ("Bf") */
    case ONCTYPE('C','k'): l =  7; break; /* pseudorng correction inp. ("Ce")*/
    /* Command "Ci" (switch to NMEA, GT versions only) has no response */
    case ONCTYPE('B','o'): l =  8; break; /* UTC offset status */
    case ONCTYPE('A','z'): l = 11; break; /* 1PPS cable delay */
    case ONCTYPE('A','y'): l = 11; break; /* 1PPS offset */
    case ONCTYPE('A','P'): l =  8; break; /* pulse mode */
    case ONCTYPE('A','s'): l = 20; break; /* position-hold position */
    case ONCTYPE('A','t'): l =  8; break; /* position-hold mode */
    case ONCTYPE('E','n'): l = 69; break; /* time RAIM setup and status */
    default:
	return 0;
    }
    /* *INDENT-ON* */

    return l - 6;		/* Subtract header and trailer. */
}
#endif /* ONCORE_ENABLE */

static void character_pushback(struct gps_packet_t *lexer)
/* push back the last character grabbed */
{
    /*@-modobserver@*//* looks like a splint bug */
    --lexer->inbufptr;
    /*@+modobserver@*/
    --lexer->char_counter;
    gpsd_report(LOG_RAW + 2, "%08ld: character pushed back\n",
		lexer->char_counter);
}

static void nextstate(struct gps_packet_t *lexer, unsigned char c)
{
    static int n = 0;
#ifdef RTCM104V2_ENABLE
    enum isgpsstat_t isgpsstat;
#endif /* RTCM104V2_ENABLE */
#ifdef SUPERSTAR2_ENABLE
    static unsigned char ctmp;
#endif /* SUPERSTAR2_ENABLE */
/*@ +charint -casebreak @*/
    n++;
    switch (lexer->state) {
    case GROUND_STATE:
	n = 0;
	if (c == '#') {
	    lexer->state = COMMENT_BODY;
	    break;
	}
#ifdef NMEA_ENABLE
	if (c == '$') {
	    lexer->state = NMEA_DOLLAR;
	    break;
	}
	if (c == '!') {
	    lexer->state = NMEA_BANG;
	    break;
	}
#endif /* NMEA_ENABLE */
#if defined(TNT_ENABLE) || defined(GARMINTXT_ENABLE) || defined(ONCORE_ENABLE)
	if (c == '@') {
	    lexer->state = AT1_LEADER;
	    break;
	}
#endif
#ifdef SIRF_ENABLE
	if (c == 0xa0) {
	    lexer->state = SIRF_LEADER_1;
	    break;
	}
#endif /* SIRF_ENABLE */
#ifdef SUPERSTAR2_ENABLE
	if (c == SOH) {
	    lexer->state = SUPERSTAR2_LEADER;
	    break;
	}
#endif /* SUPERSTAR2_ENABLE */
#if defined(TSIP_ENABLE) || defined(EVERMORE_ENABLE) || defined(GARMIN_ENABLE)
	if (c == DLE) {
	    lexer->state = DLE_LEADER;
	    break;
	}
#endif /* TSIP_ENABLE || EVERMORE_ENABLE || GARMIN_ENABLE */
#ifdef TRIPMATE_ENABLE
	if (c == 'A') {
#ifdef RTCM104V2_ENABLE
	    if (rtcm2_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = ASTRAL_1;
	    break;
	}
#endif /* TRIPMATE_ENABLE */
#ifdef EARTHMATE_ENABLE
	if (c == 'E') {
#ifdef RTCM104V2_ENABLE
	    if (rtcm2_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = EARTHA_1;
	    break;
	}
#endif /* EARTHMATE_ENABLE */
#ifdef ZODIAC_ENABLE
	if (c == 0xff) {
	    lexer->state = ZODIAC_LEADER_1;
	    break;
	}
#endif /* ZODIAC_ENABLE */
#ifdef UBX_ENABLE
	if (c == 0xb5) {
	    lexer->state = UBX_LEADER_1;
	    break;
	}
#endif /* UBX_ENABLE */
#ifdef ITRAX_ENABLE
	if (c == '<') {
	    lexer->state = ITALK_LEADER_1;
	    break;
	}
#endif /* ITRAX_ENABLE */
#ifdef NAVCOM_ENABLE
	if (c == 0x02) {
	    lexer->state = NAVCOM_LEADER_1;
	    break;
	}
#endif /* NAVCOM_ENABLE */
#ifdef GEOSTAR_ENABLE
	if (c == 'P') {
	    lexer->state = GEOSTAR_LEADER_1;
	    break;
	}
#endif /* GEOSTAR_ENABLE */
#ifdef RTCM104V2_ENABLE
	if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_SYNC) {
	    lexer->state = RTCM2_SYNC_STATE;
	    break;
	} else if (isgpsstat == ISGPS_MESSAGE) {
	    lexer->state = RTCM2_RECOGNIZED;
	    break;
	}
#endif /* RTCM104V2_ENABLE */
#ifdef RTCM104V3_ENABLE
	if (c == 0xD3) {
	    lexer->state = RTCM3_LEADER_1;
	    break;
	}
#endif /* RTCM104V3_ENABLE */
#ifdef PASSTHROUGH_ENABLE
	if (c == '{') {
	    lexer->state = JSON_LEADER;
	    character_pushback(lexer);
	}
#endif /* PASSTHROUGH_ENABLE */
	break;
    case COMMENT_BODY:
	if (c == '\n')
	    lexer->state = COMMENT_RECOGNIZED;
	else if (!isprint(c)) {
	    lexer->state = GROUND_STATE;
	    character_pushback(lexer);
	}
	break;
#ifdef NMEA_ENABLE
    case NMEA_DOLLAR:
	if (c == 'G')
	    lexer->state = NMEA_PUB_LEAD;
	else if (c == 'P')	/* vendor sentence */
	    lexer->state = NMEA_VENDOR_LEAD;
	else if (c == 'I')	/* Seatalk */
	    lexer->state = SEATALK_LEAD_1;
	else if (c == 'A')	/* SiRF Ack */
	    lexer->state = SIRF_ACK_LEAD_1;
#ifdef OCEANSERVER_ENABLE
	else if (c == 'C')
	    lexer->state = NMEA_LEADER_END;
#endif /* OCEANSERVER_ENABLE */
	else
	    lexer->state = GROUND_STATE;
	break;
    case NMEA_PUB_LEAD:
	/*
	 * $GP == GPS, $GL = GLONASS only, $GN = mixed GPS and GLONASS,
	 * according to NMEA (IEIC 61162-1) DRAFT 02/06/2009.
	 */
	if (c == 'P' || c == 'N' || c == 'L')
	    lexer->state = NMEA_LEADER_END;
	else
	    lexer->state = GROUND_STATE;
	break;
    case NMEA_VENDOR_LEAD:
	if (c == 'A')
	    lexer->state = NMEA_PASHR_A;
	else if (isalpha(c))
	    lexer->state = NMEA_LEADER_END;
	else
	    lexer->state = GROUND_STATE;
	break;
    /*
     * Without the following six states, DLE in a $PASHR can fool the
     * sniffer into thinking it sees a TSIP packet.  Hilarity ensues.
     */
    case NMEA_PASHR_A:
	if (c == 'S')
	    lexer->state = NMEA_PASHR_S;
	else if (isalpha(c))
	    lexer->state = NMEA_LEADER_END;
	else
	    lexer->state = GROUND_STATE;
	break;
    case NMEA_PASHR_S:
	if (c == 'H')
	    lexer->state = NMEA_PASHR_H;
	else if (isalpha(c))
	    lexer->state = NMEA_LEADER_END;
	else
	    lexer->state = GROUND_STATE;
	break;
    case NMEA_PASHR_H:
	if (c == 'R')
	    lexer->state = NMEA_BINARY_BODY;
	else if (isalpha(c))
	    lexer->state = NMEA_LEADER_END;
	else
	    lexer->state = GROUND_STATE;
	break;
    case NMEA_BINARY_BODY:
	if (c == '\r')
	    lexer->state = NMEA_BINARY_CR;
	break;
    case NMEA_BINARY_CR:
	if (c == '\n')
	    lexer->state = NMEA_BINARY_NL;
	else
	    lexer->state = NMEA_BINARY_BODY;
	break;
    case NMEA_BINARY_NL:
	if (c == '$') {
	    character_pushback(lexer);
	    lexer->state = NMEA_RECOGNIZED;
	} else
	    lexer->state = NMEA_BINARY_BODY;
	break;
    case NMEA_BANG:
	if (c == 'A')
	    lexer->state = AIS_LEAD_1;
	else
	    lexer->state = GROUND_STATE;
	break;
    case AIS_LEAD_1:
	if (c == 'I')
	    lexer->state = AIS_LEAD_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case AIS_LEAD_2:
	if (isalpha(c))
	    lexer->state = NMEA_LEADER_END;
	else
	    lexer->state = GROUND_STATE;
	break;
#if defined(TNT_ENABLE) || defined(GARMINTXT_ENABLE) || defined(ONCORE_ENABLE)
    case AT1_LEADER:
	switch (c) {
#ifdef ONCORE_ENABLE
	case '@':
	    lexer->state = ONCORE_AT2;
	    break;
#endif /* ONCORE_ENABLE */
#ifdef TNT_ENABLE
	case '*':
	    /*
	     * TNT has similar structure to NMEA packet, '*' before
	     * optional checksum ends the packet. Since '*' cannot be
	     * received from GARMIN working in TEXT mode, use this
	     * difference to tell that this is not GARMIN TEXT packet,
	     * could be TNT.
	     */
	    lexer->state = NMEA_LEADER_END;
	    break;
#endif /* TNT_ENABLE */
#if defined(GARMINTXT_ENABLE)
	case '\r':
	    /* stay in this state, next character should be '\n' */
	    /* in the theory we can stop search here and don't wait for '\n' */
	    lexer->state = AT1_LEADER;
	    break;
	case '\n':
	    /* end of packet found */
	    lexer->state = GTXT_RECOGNIZED;
	    break;
#endif /* GARMINTXT_ENABLE */
	default:
	    if (!isprint(c))
		lexer->state = GROUND_STATE;
	}
	break;
#endif /* defined(TNT_ENABLE) || defined(GARMINTXT_ENABLE) || defined(ONCORE_ENABLE) */
    case NMEA_LEADER_END:
	if (c == '\r')
	    lexer->state = NMEA_CR;
	else if (c == '\n')
	    /* not strictly correct, but helps for interpreting logfiles */
	    lexer->state = NMEA_RECOGNIZED;
	else if (c == '$'){
	    /* faster recovery from missing sentence trailers */
	    lexer->state = NMEA_DOLLAR;
	    lexer->inbufptr += (n-1);
	} else if (!isprint(c))
	    lexer->state = GROUND_STATE;
	break;
    case NMEA_CR:
	if (c == '\n')
	    lexer->state = NMEA_RECOGNIZED;
	/*
	 * There's a GPS called a Jackson Labs Firefly-1a that emits \r\r\n
	 * at the end of each sentence.  Don't be confused by this.
	 */
	else if (c == '\r')
	    lexer->state = NMEA_CR;
	else
	    lexer->state = GROUND_STATE;
	break;
    case NMEA_RECOGNIZED:
	if (c == '#')
	    lexer->state = COMMENT_BODY;
	else if (c == '$')
	    lexer->state = NMEA_DOLLAR;
	else if (c == '!')
	    lexer->state = NMEA_BANG;
#ifdef UBX_ENABLE
	else if (c == 0xb5)	/* LEA-5H can and will output NMEA and UBX back to back */
	    lexer->state = UBX_LEADER_1;
#endif
	else
	    lexer->state = GROUND_STATE;
	break;
    case SEATALK_LEAD_1:
	if (c == 'I' || c == 'N')	/* II or IN are accepted */
	    lexer->state = NMEA_LEADER_END;
	else
	    lexer->state = GROUND_STATE;
	break;
#ifdef TRIPMATE_ENABLE
    case ASTRAL_1:
	if (c == 'S') {
#ifdef RTCM104V2_ENABLE
	    if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_SYNC) {
		lexer->state = RTCM2_SYNC_STATE;
		break;
	    } else if (isgpsstat == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = ASTRAL_2;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case ASTRAL_2:
	if (c == 'T') {
#ifdef RTCM104V2_ENABLE
	    if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_SYNC) {
		lexer->state = RTCM2_SYNC_STATE;
		break;
	    } else if (isgpsstat == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = ASTRAL_3;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case ASTRAL_3:
	if (c == 'R') {
#ifdef RTCM104V2_ENABLE
	    if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_SYNC) {
		lexer->state = RTCM2_SYNC_STATE;
		break;
	    } else if (isgpsstat == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = ASTRAL_5;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case ASTRAL_4:
	if (c == 'A') {
#ifdef RTCM104V2_ENABLE
	    if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_SYNC) {
		lexer->state = RTCM2_SYNC_STATE;
		break;
	    } else if (isgpsstat == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = ASTRAL_2;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case ASTRAL_5:
	if (c == 'L') {
#ifdef RTCM104V2_ENABLE
	    if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_SYNC) {
		lexer->state = RTCM2_SYNC_STATE;
		break;
	    } else if (isgpsstat == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = NMEA_RECOGNIZED;
	} else
	    lexer->state = GROUND_STATE;
	break;
#endif /* TRIPMATE_ENABLE */
#ifdef EARTHMATE_ENABLE
    case EARTHA_1:
	if (c == 'A') {
#ifdef RTCM104V2_ENABLE
	    if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_SYNC) {
		lexer->state = RTCM2_SYNC_STATE;
		break;
	    } else if (isgpsstat == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = EARTHA_2;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case EARTHA_2:
	if (c == 'R') {
#ifdef RTCM104V2_ENABLE
	    if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_SYNC) {
		lexer->state = RTCM2_SYNC_STATE;
		break;
	    } else if (isgpsstat == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = EARTHA_3;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case EARTHA_3:
	if (c == 'T') {
#ifdef RTCM104V2_ENABLE
	    if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_SYNC) {
		lexer->state = RTCM2_SYNC_STATE;
		break;
	    } else if (isgpsstat == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = EARTHA_4;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case EARTHA_4:
	if (c == 'H') {
#ifdef RTCM104V2_ENABLE
	    if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_SYNC) {
		lexer->state = RTCM2_SYNC_STATE;
		break;
	    } else if (isgpsstat == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = EARTHA_5;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case EARTHA_5:
	if (c == 'A') {
#ifdef RTCM104V2_ENABLE
	    if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_SYNC) {
		lexer->state = RTCM2_SYNC_STATE;
		break;
	    } else if (isgpsstat == ISGPS_MESSAGE) {
		lexer->state = RTCM2_RECOGNIZED;
		break;
	    }
#endif /* RTCM104V2_ENABLE */
	    lexer->state = NMEA_RECOGNIZED;
	} else
	    lexer->state = GROUND_STATE;
	break;
#endif /* EARTHMATE_ENABLE */
    case SIRF_ACK_LEAD_1:
	if (c == 'c')
	    lexer->state = SIRF_ACK_LEAD_2;
	else if (c == 'I')
	    lexer->state = AIS_LEAD_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case SIRF_ACK_LEAD_2:
	if (c == 'k')
	    lexer->state = NMEA_LEADER_END;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* NMEA_ENABLE */
#ifdef SIRF_ENABLE
    case SIRF_LEADER_1:
	if (c == 0xa2)
	    lexer->state = SIRF_LEADER_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case SIRF_LEADER_2:
	lexer->length = (size_t) (c << 8);
	lexer->state = SIRF_LENGTH_1;
	break;
    case SIRF_LENGTH_1:
	lexer->length += c + 2;
	if (lexer->length <= MAX_PACKET_LENGTH)
	    lexer->state = SIRF_PAYLOAD;
	else
	    lexer->state = GROUND_STATE;
	break;
    case SIRF_PAYLOAD:
	if (--lexer->length == 0)
	    lexer->state = SIRF_DELIVERED;
	break;
    case SIRF_DELIVERED:
	if (c == 0xb0)
	    lexer->state = SIRF_TRAILER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
    case SIRF_TRAILER_1:
	if (c == 0xb3)
	    lexer->state = SIRF_RECOGNIZED;
	else
	    lexer->state = GROUND_STATE;
	break;
    case SIRF_RECOGNIZED:
	if (c == 0xa0)
	    lexer->state = SIRF_LEADER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* SIRF_ENABLE */
#ifdef SUPERSTAR2_ENABLE
    case SUPERSTAR2_LEADER:
	ctmp = c;
	lexer->state = SUPERSTAR2_ID1;
	break;
    case SUPERSTAR2_ID1:
	if ((ctmp ^ 0xff) == c)
	    lexer->state = SUPERSTAR2_ID2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case SUPERSTAR2_ID2:
	lexer->length = (size_t) c;	/* how many data bytes follow this byte */
	if (lexer->length)
	    lexer->state = SUPERSTAR2_PAYLOAD;
	else
	    lexer->state = SUPERSTAR2_CKSUM1;	/* no data, jump to checksum */
	break;
    case SUPERSTAR2_PAYLOAD:
	if (--lexer->length == 0)
	    lexer->state = SUPERSTAR2_CKSUM1;
	break;
    case SUPERSTAR2_CKSUM1:
	lexer->state = SUPERSTAR2_CKSUM2;
	break;
    case SUPERSTAR2_CKSUM2:
	lexer->state = SUPERSTAR2_RECOGNIZED;
	break;
    case SUPERSTAR2_RECOGNIZED:
	if (c == SOH)
	    lexer->state = SUPERSTAR2_LEADER;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* SUPERSTAR2_ENABLE */
#ifdef ONCORE_ENABLE
    case ONCORE_AT2:
	if (isupper(c)) {
	    lexer->length = (size_t) c;
	    lexer->state = ONCORE_ID1;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case ONCORE_ID1:
	if (isalpha(c)) {
	    lexer->length =
		oncore_payload_cksum_length((unsigned char)lexer->length, c);
	    if (lexer->length != 0) {
		lexer->state = ONCORE_PAYLOAD;
		break;
	    }
	}
	lexer->state = GROUND_STATE;
	break;
    case ONCORE_PAYLOAD:
	if (--lexer->length == 0)
	    lexer->state = ONCORE_CHECKSUM;
	break;
    case ONCORE_CHECKSUM:
	if (c != '\r')
	    lexer->state = GROUND_STATE;
	else
	    lexer->state = ONCORE_CR;
	break;
    case ONCORE_CR:
	if (c == '\n')
	    lexer->state = ONCORE_RECOGNIZED;
	else
	    lexer->state = ONCORE_PAYLOAD;
	break;
    case ONCORE_RECOGNIZED:
	if (c == '@')
	    lexer->state = AT1_LEADER;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* ONCORE_ENABLE */
#if defined(TSIP_ENABLE) || defined(EVERMORE_ENABLE) || defined(GARMIN_ENABLE)
    case DLE_LEADER:
#ifdef EVERMORE_ENABLE
	if (c == STX) {
	    lexer->state = EVERMORE_LEADER_2;
	    break;
	}
#endif /* EVERMORE_ENABLE */
#if defined(TSIP_ENABLE) || defined(GARMIN_ENABLE) || defined(NAVCOM_ENABLE)
	/* garmin is special case of TSIP */
	/* check last because there's no checksum */
#if defined(TSIP_ENABLE)
	if (c >= 0x13) {
	    lexer->state = TSIP_PAYLOAD;
	    break;
	}
#endif /* TSIP_ENABLE */
	if (c == DLE) {
	    lexer->state = GROUND_STATE;
	    break;
	}
	// FALL-THRU!!!!! no break here
#endif /* TSIP_ENABLE */
#ifdef NAVCOM_ENABLE
    case NAVCOM_LEADER_1:
	if (c == 0x99)
	    lexer->state = NAVCOM_LEADER_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case NAVCOM_LEADER_2:
	if (c == 0x66)
	    lexer->state = NAVCOM_LEADER_3;
	else
	    lexer->state = GROUND_STATE;
	break;
    case NAVCOM_LEADER_3:
	lexer->state = NAVCOM_ID;
	break;
    case NAVCOM_ID:
	lexer->length = (size_t) c - 4;
	lexer->state = NAVCOM_LENGTH_1;
	break;
    case NAVCOM_LENGTH_1:
	lexer->length += (c << 8);
	lexer->state = NAVCOM_LENGTH_2;
	break;
    case NAVCOM_LENGTH_2:
	if (--lexer->length == 0)
	    lexer->state = NAVCOM_PAYLOAD;
	break;
    case NAVCOM_PAYLOAD:
    {
	unsigned char csum = lexer->inbuffer[3];
	for (n = 4;
	     (unsigned char *)(lexer->inbuffer + n) < lexer->inbufptr - 1;
	     n++)
	    csum ^= lexer->inbuffer[n];
	if (csum != c) {
	    gpsd_report(LOG_IO,
			"Navcom packet type 0x%hhx bad checksum 0x%hhx, expecting 0x%x\n",
			lexer->inbuffer[3], csum, c);
	    lexer->state = GROUND_STATE;
	    break;
	}
    }
	lexer->state = NAVCOM_CSUM;
	break;
    case NAVCOM_CSUM:
	if (c == 0x03)
	    lexer->state = NAVCOM_RECOGNIZED;
	else
	    lexer->state = GROUND_STATE;
	break;
    case NAVCOM_RECOGNIZED:
	if (c == 0x02)
	    lexer->state = NAVCOM_LEADER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* NAVCOM_ENABLE */
#endif /* TSIP_ENABLE || EVERMORE_ENABLE || GARMIN_ENABLE */
#ifdef RTCM104V3_ENABLE
    case RTCM3_LEADER_1:
	/* high 6 bits must be zero, low 2 bits are MSB of a 10-bit length */
	if ((c & 0xFC) == 0) {
	    lexer->length = (size_t) (c << 8);
	    lexer->state = RTCM3_LEADER_2;
	    break;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case RTCM3_LEADER_2:
	/* third byte is the low 8 bits of the RTCM3 packet length */
	lexer->length |= c;
	lexer->length += 3;	/* to get the three checksum bytes */
	lexer->state = RTCM3_PAYLOAD;
	break;
    case RTCM3_PAYLOAD:
	if (--lexer->length == 0)
	    lexer->state = RTCM3_RECOGNIZED;
	break;
#endif /* RTCM104V3_ENABLE */
#ifdef ZODIAC_ENABLE
    case ZODIAC_EXPECTED:
    case ZODIAC_RECOGNIZED:
	if (c == 0xff)
	    lexer->state = ZODIAC_LEADER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
    case ZODIAC_LEADER_1:
	if (c == 0x81)
	    lexer->state = ZODIAC_LEADER_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case ZODIAC_LEADER_2:
	lexer->state = ZODIAC_ID_1;
	break;
    case ZODIAC_ID_1:
	lexer->state = ZODIAC_ID_2;
	break;
    case ZODIAC_ID_2:
	lexer->length = (size_t) c;
	lexer->state = ZODIAC_LENGTH_1;
	break;
    case ZODIAC_LENGTH_1:
	lexer->length += (c << 8);
	lexer->state = ZODIAC_LENGTH_2;
	break;
    case ZODIAC_LENGTH_2:
	lexer->state = ZODIAC_FLAGS_1;
	break;
    case ZODIAC_FLAGS_1:
	lexer->state = ZODIAC_FLAGS_2;
	break;
    case ZODIAC_FLAGS_2:
	lexer->state = ZODIAC_HSUM_1;
	break;
    case ZODIAC_HSUM_1:
    {
#define getword(i) (short)(lexer->inbuffer[2*(i)] | (lexer->inbuffer[2*(i)+1] << 8))
	short sum = getword(0) + getword(1) + getword(2) + getword(3);
	sum *= -1;
	if (sum != getword(4)) {
	    gpsd_report(LOG_IO,
			"Zodiac Header checksum 0x%hx expecting 0x%hx\n",
			sum, getword(4));
	    lexer->state = GROUND_STATE;
	    break;
	}
    }
	gpsd_report(LOG_RAW + 1, "Zodiac header id=%hd len=%hd flags=%hx\n",
		    getword(1), getword(2), getword(3));
#undef getword
	if (lexer->length == 0) {
	    lexer->state = ZODIAC_RECOGNIZED;
	    break;
	}
	lexer->length *= 2;	/* word count to byte count */
	lexer->length += 2;	/* checksum */
	/* 10 bytes is the length of the Zodiac header */
	if (lexer->length <= MAX_PACKET_LENGTH - 10)
	    lexer->state = ZODIAC_PAYLOAD;
	else
	    lexer->state = GROUND_STATE;
	break;
    case ZODIAC_PAYLOAD:
	if (--lexer->length == 0)
	    lexer->state = ZODIAC_RECOGNIZED;
	break;
#endif /* ZODIAC_ENABLE */
#ifdef UBX_ENABLE
    case UBX_LEADER_1:
	if (c == 0x62)
	    lexer->state = UBX_LEADER_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case UBX_LEADER_2:
	lexer->state = UBX_CLASS_ID;
	break;
    case UBX_CLASS_ID:
	lexer->state = UBX_MESSAGE_ID;
	break;
    case UBX_MESSAGE_ID:
	lexer->length = (size_t) c;
	lexer->state = UBX_LENGTH_1;
	break;
    case UBX_LENGTH_1:
	lexer->length += (c << 8);
	if (lexer->length <= MAX_PACKET_LENGTH)
	    lexer->state = UBX_LENGTH_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case UBX_LENGTH_2:
	lexer->state = UBX_PAYLOAD;
	break;
    case UBX_PAYLOAD:
	if (--lexer->length == 0)
	    lexer->state = UBX_CHECKSUM_A;
	/* else stay in payload state */
	break;
    case UBX_CHECKSUM_A:
	lexer->state = UBX_RECOGNIZED;
	break;
    case UBX_RECOGNIZED:
	if (c == 0xb5)
	    lexer->state = UBX_LEADER_1;
#ifdef NMEA_ENABLE
	else if (c == '$')	/* LEA-5H can and will output NMEA and UBX back to back */
	    lexer->state = NMEA_DOLLAR;
#endif /* NMEA_ENABLE */
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* UBX_ENABLE */
#ifdef EVERMORE_ENABLE
    case EVERMORE_LEADER_1:
	if (c == STX)
	    lexer->state = EVERMORE_LEADER_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case EVERMORE_LEADER_2:
	lexer->length = (size_t) c;
	if (c == DLE)
	    lexer->state = EVERMORE_PAYLOAD_DLE;
	else
	    lexer->state = EVERMORE_PAYLOAD;
	break;
    case EVERMORE_PAYLOAD:
	if (c == DLE)
	    lexer->state = EVERMORE_PAYLOAD_DLE;
	else if (--lexer->length == 0)
	    lexer->state = GROUND_STATE;
	break;
    case EVERMORE_PAYLOAD_DLE:
	switch (c) {
	case DLE:
	    lexer->state = EVERMORE_PAYLOAD;
	    break;
	case ETX:
	    lexer->state = EVERMORE_RECOGNIZED;
	    break;
	default:
	    lexer->state = GROUND_STATE;
	}
	break;
    case EVERMORE_RECOGNIZED:
	if (c == DLE)
	    lexer->state = EVERMORE_LEADER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* EVERMORE_ENABLE */
#ifdef ITRAX_ENABLE
    case ITALK_LEADER_1:
	if (c == '!')
	    lexer->state = ITALK_LEADER_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case ITALK_LEADER_2:
	lexer->length = (size_t) (lexer->inbuffer[6] & 0xff);
	lexer->state = ITALK_LENGTH;
	break;
    case ITALK_LENGTH:
	lexer->length += 1;	/* fix number of words in payload */
	lexer->length *= 2;	/* convert to number of bytes */
	lexer->length += 3;	/* add trailer length */
	lexer->state = ITALK_PAYLOAD;
	break;
    case ITALK_PAYLOAD:
	/* lookahead for "<!" because sometimes packets are short but valid */
	if ((c == '>') && (lexer->inbufptr[0] == '<') &&
	    (lexer->inbufptr[1] == '!')) {
	    lexer->state = ITALK_RECOGNIZED;
	    gpsd_report(LOG_IO, "ITALK: trying to process runt packet\n");
	    break;
	} else if (--lexer->length == 0)
	    lexer->state = ITALK_DELIVERED;
	break;
    case ITALK_DELIVERED:
	if (c == '>')
	    lexer->state = ITALK_RECOGNIZED;
	else
	    lexer->state = GROUND_STATE;
	break;
    case ITALK_RECOGNIZED:
	if (c == '<')
	    lexer->state = ITALK_LEADER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* ITRAX_ENABLE */
#ifdef GEOSTAR_ENABLE
    case GEOSTAR_LEADER_1:
	if (c == 'S')
	    lexer->state = GEOSTAR_LEADER_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case GEOSTAR_LEADER_2:
	if (c == 'G')
	    lexer->state = GEOSTAR_LEADER_3;
	else
	    lexer->state = GROUND_STATE;
	break;
    case GEOSTAR_LEADER_3:
	if (c == 'G')
	    lexer->state = GEOSTAR_LEADER_4;
	else
	    lexer->state = GROUND_STATE;
	break;
    case GEOSTAR_LEADER_4:
	lexer->state = GEOSTAR_MESSAGE_ID_1;
	break;
    case GEOSTAR_MESSAGE_ID_1:
	lexer->state = GEOSTAR_MESSAGE_ID_2;
	break;
    case GEOSTAR_MESSAGE_ID_2:
	lexer->length = (size_t)(c * 4);
	lexer->state = GEOSTAR_LENGTH_1;
	break;
    case GEOSTAR_LENGTH_1:
	lexer->length += (c << 8) * 4;
	if (lexer->length <= MAX_PACKET_LENGTH)
	    lexer->state = GEOSTAR_LENGTH_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case GEOSTAR_LENGTH_2:
	lexer->state = GEOSTAR_PAYLOAD;
	break;
    case GEOSTAR_PAYLOAD:
	if (--lexer->length == 0)
	    lexer->state = GEOSTAR_CHECKSUM_A;
	/* else stay in payload state */
	break;
    case GEOSTAR_CHECKSUM_A:
	lexer->state = GEOSTAR_CHECKSUM_B;
	break;
    case GEOSTAR_CHECKSUM_B:
	lexer->state = GEOSTAR_CHECKSUM_C;
	break;
    case GEOSTAR_CHECKSUM_C:
	lexer->state = GEOSTAR_RECOGNIZED;
	break;
    case GEOSTAR_RECOGNIZED:
	if (c == 'P')
	    lexer->state = GEOSTAR_LEADER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* GEOSTAR_ENABLE */
#ifdef TSIP_ENABLE
    case TSIP_LEADER:
	/* unused case */
	if (c >= 0x13)
	    lexer->state = TSIP_PAYLOAD;
	else
	    lexer->state = GROUND_STATE;
	break;
    case TSIP_PAYLOAD:
	if (c == DLE)
	    lexer->state = TSIP_DLE;
	break;
    case TSIP_DLE:
	switch (c) {
	case ETX:
	    lexer->state = TSIP_RECOGNIZED;
	    break;
	case DLE:
	    lexer->state = TSIP_PAYLOAD;
	    break;
	default:
	    lexer->state = GROUND_STATE;
	    break;
	}
	break;
    case TSIP_RECOGNIZED:
	if (c == DLE)
	    /*
	     * Don't go to TSIP_LEADER state -- TSIP packets aren't
	     * checksummed, so false positives are easy.  We might be
	     * looking at another DLE-stuffed protocol like EverMore
	     * or Garmin streaming binary.
	     */
	    lexer->state = DLE_LEADER;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* TSIP_ENABLE */
#ifdef RTCM104V2_ENABLE
    case RTCM2_SYNC_STATE:
    case RTCM2_SKIP_STATE:
	if ((isgpsstat = rtcm2_decode(lexer, c)) == ISGPS_MESSAGE) {
	    lexer->state = RTCM2_RECOGNIZED;
	    break;
	} else if (isgpsstat == ISGPS_NO_SYNC)
	    lexer->state = GROUND_STATE;
	break;

    case RTCM2_RECOGNIZED:
	if (rtcm2_decode(lexer, c) == ISGPS_SYNC) {
	    lexer->state = RTCM2_SYNC_STATE;
	    break;
	} else
	    lexer->state = GROUND_STATE;
	break;
#endif /* RTCM104V2_ENABLE */
#ifdef PASSTHROUGH_ENABLE
    case JSON_LEADER:
	if (c == '{' || c == '[') {
	    lexer->json_depth++;
	} else if (c == '}' || c == ']') {
	    if (--lexer->json_depth == 0)
		lexer->state = JSON_RECOGNIZED;
	} else if (isspace(c) || c == ',')
	    break;
	else if (c == '"') {
	    lexer->state = JSON_STRINGLITERAL;
	    lexer->json_after = JSON_END_ATTRIBUTE;
	} else {
	    gpsd_report(LOG_RAW + 2, 
			"%08ld: missing attribute start after header\n",
			lexer->char_counter);
	    lexer->state = GROUND_STATE;
	}
	break;
    case JSON_STRINGLITERAL:
	if (c == '\\')
	    lexer->state = JSON_STRING_SOLIDUS;
	else if (c == '"')
	    lexer->state = lexer->json_after;
	break;
    case JSON_STRING_SOLIDUS:
	lexer->state = JSON_STRINGLITERAL;
	break;
    case JSON_END_ATTRIBUTE:
	if (isspace(c))
	    break;
	else if (c == ':')
	    lexer->state = JSON_EXPECT_VALUE;
	else
	    /* saw something other than value start after colon */
	    lexer->state = GROUND_STATE;
	break;
    case JSON_EXPECT_VALUE:
	if (isspace(c))
	    break;
	else if (c == '"') {
	    lexer->state = JSON_STRINGLITERAL;
	    lexer->json_after = JSON_END_VALUE;
	} else if (c == '{' || c == '[') {
	    lexer->state = JSON_LEADER;
	    character_pushback(lexer);
	} else if (strchr("-0123456789", c) != NULL) {
	    lexer->state = JSON_NUMBER;
	} else if (c == 't' || c == 'f' || c == 'n')
	    /*
	     * This is a bit more permissive than strictly necessary, as
	     * GPSD JSON does not include the null token.  Still, it's
	     * futureproofing.
	     */
	    lexer->state = JSON_SPECIAL;
	else
	    /* couldn't recognize start of value literal */
	    lexer->state = GROUND_STATE;
	break;
    case JSON_NUMBER:
	/*
	 * Will recognize some ill-formed numeric literals.
	 * Should be OK as we're already three stages deep inside
	 * JSON recognition; odds that we'll actually see an
	 * ill-formed literal are quite low. and the worst
	 * possible result if it happens is our JSON parser will
	 * quietly chuck out the object.
	 */
	if (strchr("1234567890.eE+-", c) == NULL) {
	    lexer->state = JSON_END_VALUE;
	    character_pushback(lexer);
	}
	break;
    case JSON_SPECIAL:
	if (strchr("truefalsnil", c) == NULL) {
	    lexer->state = JSON_END_VALUE;
	    character_pushback(lexer);
	}
	break;
    case JSON_END_VALUE:
	if (isspace(c))
	    break;
	else if (c == ',')
	    lexer->state = JSON_LEADER;
	else if (c == '}' || c == ']') {
	    lexer->state = JSON_LEADER;
	    character_pushback(lexer);
	} else
	    /* trailing garbage after JSON value */
	    lexer->state = GROUND_STATE;
	break;
#endif /* PASSTHROUGH_ENABLE */
    }
/*@ -charint +casebreak @*/
}

static void packet_accept(struct gps_packet_t *lexer, int packet_type)
/* packet grab succeeded, move to output buffer */
{
    size_t packetlen = lexer->inbufptr - lexer->inbuffer;
    if (packetlen < sizeof(lexer->outbuffer)) {
	memcpy(lexer->outbuffer, lexer->inbuffer, packetlen);
	lexer->outbuflen = packetlen;
	lexer->outbuffer[packetlen] = '\0';
	lexer->type = packet_type;
	if (lexer->debug >= LOG_RAW+1)
	    gpsd_report(LOG_RAW+1, "Packet type %d accepted %zu = %s\n",
		    packet_type, packetlen,
			gpsd_hexdump((char *)lexer->outbuffer, lexer->outbuflen));
    } else {
	gpsd_report(LOG_ERROR, "Rejected too long packet type %d len %zu\n",
		    packet_type, packetlen);
    }
}

static void packet_discard(struct gps_packet_t *lexer)
/* shift the input buffer to discard all data up to current input pointer */
{
    size_t discard = lexer->inbufptr - lexer->inbuffer;
    size_t remaining = lexer->inbuflen - discard;
    lexer->inbufptr = memmove(lexer->inbuffer, lexer->inbufptr, remaining);
    lexer->inbuflen = remaining;
    if (lexer->debug >= LOG_RAW+1)
	gpsd_report(LOG_RAW + 1,
		    "Packet discard of %zu, chars remaining is %zu = %s\n",
		    discard, remaining,
		    gpsd_hexdump((char *)lexer->inbuffer, lexer->inbuflen));
}

static void character_discard(struct gps_packet_t *lexer)
/* shift the input buffer to discard one character and reread data */
{
    memmove(lexer->inbuffer, lexer->inbuffer + 1, (size_t)-- lexer->inbuflen);
    lexer->inbufptr = lexer->inbuffer;
    if (lexer->debug >= LOG_RAW+1)
	gpsd_report(LOG_RAW + 1, "Character discarded, buffer %zu chars = %s\n",
		    lexer->inbuflen,
		    gpsd_hexdump((char *)lexer->inbuffer, lexer->inbuflen));
}

/* get 0-origin big-endian words relative to start of packet buffer */
#define getword(i) (short)(lexer->inbuffer[2*(i)] | (lexer->inbuffer[2*(i)+1] << 8))

/* entry points begin here */

void packet_init( /*@out@*/ struct gps_packet_t *lexer)
{
    lexer->char_counter = 0;
    lexer->retry_counter = 0;
#ifdef PASSTHROUGH_ENABLE
    lexer->json_depth = 0;
#endif /* PASSTHROUGH_ENABLE */
    packet_reset(lexer);
}

void packet_parse(struct gps_packet_t *lexer)
/* grab a packet from the input buffer */
{
    lexer->outbuflen = 0;
    while (packet_buffered_input(lexer) > 0) {
	/*@ -modobserver @*/
	unsigned char c = *lexer->inbufptr++;
	/*@ +modobserver @*/
	char *state_table[] = {
#include "packet_names.h"
	};
	nextstate(lexer, c);
	gpsd_report(LOG_RAW + 2,
		    "%08ld: character '%c' [%02x], new state: %s\n",
		    lexer->char_counter, (isprint(c) ? c : '.'), c,
		    state_table[lexer->state]);
	lexer->char_counter++;

	if (lexer->state == GROUND_STATE) {
	    character_discard(lexer);
	} else if (lexer->state == COMMENT_RECOGNIZED) {
	    packet_accept(lexer, COMMENT_PACKET);
	    packet_discard(lexer);
	    lexer->state = GROUND_STATE;
	    break;
	}
#ifdef NMEA_ENABLE
	else if (lexer->state == NMEA_RECOGNIZED) {
	    /*
	     * $PASHR packets have no checksum. Avoid the possibility
	     * that random garbage might make it look like they do.
	     */
	    if (strncmp((const char *)lexer->inbuffer, "$PASHR,", 7) != 0)
	    {
		bool checksum_ok = true;
		char csum[3] = { '0', '0', '0' };
		char *end;
		/*
		 * Back up past any whitespace.  Need to do this because
		 * at least one GPS (the Firefly 1a) emits \r\r\n
		 */
		for (end = (char *)lexer->inbufptr - 1; isspace(*end); end--)
		    continue;
		while (strchr("0123456789ABCDEF", *end))
		    --end;
		if (*end == '*') {
		    unsigned int n, crc = 0;
		    for (n = 1; (char *)lexer->inbuffer + n < end; n++)
			crc ^= lexer->inbuffer[n];
		    (void)snprintf(csum, sizeof(csum), "%02X", crc);
		    checksum_ok = (csum[0] == toupper(end[1])
				   && csum[1] == toupper(end[2]));
		}
		if (!checksum_ok) {
		    gpsd_report(LOG_WARN,
				"bad checksum in NMEA packet; expected %s.\n",
				csum);
		    packet_accept(lexer, BAD_PACKET);
		    lexer->state = GROUND_STATE;
		    packet_discard(lexer);
		    break;    /* exit case */
		}
	    }
	    /* checksum passed or not present */
#ifdef AIVDM_ENABLE
	    if (strncmp((char *)lexer->inbuffer, "!AIVDM", 6) == 0)
		packet_accept(lexer, AIVDM_PACKET);
	    else if (strncmp((char *)lexer->inbuffer, "!AIVDO", 6) == 0)
		packet_accept(lexer, AIVDM_PACKET);
	    else
#endif /* AIVDM_ENABLE */
		packet_accept(lexer, NMEA_PACKET);
	    packet_discard(lexer);
	    break;
	}
#endif /* NMEA_ENABLE */
#ifdef SIRF_ENABLE
	else if (lexer->state == SIRF_RECOGNIZED) {
	    unsigned char *trailer = lexer->inbufptr - 4;
	    unsigned int checksum =
		(unsigned)((trailer[0] << 8) | trailer[1]);
	    unsigned int n, crc = 0;
	    for (n = 4; n < (unsigned)(trailer - lexer->inbuffer); n++)
		crc += (int)lexer->inbuffer[n];
	    crc &= 0x7fff;
	    if (checksum == crc)
		packet_accept(lexer, SIRF_PACKET);
	    else {
		packet_accept(lexer, BAD_PACKET);
		lexer->state = GROUND_STATE;
	    }
	    packet_discard(lexer);
	    break;
	}
#endif /* SIRF_ENABLE */
#ifdef SUPERSTAR2_ENABLE
	else if (lexer->state == SUPERSTAR2_RECOGNIZED) {
	    unsigned a = 0, b;
	    size_t n;
	    lexer->length = 4 + (size_t) lexer->inbuffer[3] + 2;
	    for (n = 0; n < lexer->length - 2; n++)
		a += (unsigned)lexer->inbuffer[n];
	    b = (unsigned)getleu16(lexer->inbuffer, lexer->length - 2);
	    gpsd_report(LOG_IO, "SuperStarII pkt dump: type %u len %u\n",
			lexer->inbuffer[1], (unsigned int)lexer->length);
	    if (a != b) {
		gpsd_report(LOG_IO, "REJECT SuperStarII packet type 0x%02x"
			    "%zd bad checksum 0x%04x, expecting 0x%04x\n",
			    lexer->inbuffer[1], lexer->length, a, b);
		packet_accept(lexer, BAD_PACKET);
		lexer->state = GROUND_STATE;
	    } else {
		packet_accept(lexer, SUPERSTAR2_PACKET);
	    }
	    packet_discard(lexer);
	    break;
	}
#endif /* SUPERSTAR2_ENABLE */
#ifdef ONCORE_ENABLE
	else if (lexer->state == ONCORE_RECOGNIZED) {
	    char a, b;
	    int i, len;

	    len = lexer->inbufptr - lexer->inbuffer;
	    a = (char)(lexer->inbuffer[len - 3]);
	    b = '\0';
	    for (i = 2; i < len - 3; i++)
		b ^= lexer->inbuffer[i];
	    if (a == b) {
		gpsd_report(LOG_IO, "Accept OnCore packet @@%c%c len %d\n",
			    lexer->inbuffer[2], lexer->inbuffer[3], len);
		packet_accept(lexer, ONCORE_PACKET);
	    } else {
		gpsd_report(LOG_IO, "REJECT OnCore packet @@%c%c len %d\n",
			    lexer->inbuffer[2], lexer->inbuffer[3], len);
		packet_accept(lexer, BAD_PACKET);
		lexer->state = GROUND_STATE;
	    }
	    packet_discard(lexer);
	    break;
	}
#endif /* ONCORE_ENABLE */
#if defined(TSIP_ENABLE) || defined(GARMIN_ENABLE)
	else if (lexer->state == TSIP_RECOGNIZED) {
	    size_t packetlen = lexer->inbufptr - lexer->inbuffer;
#ifdef TSIP_ENABLE
	    unsigned int pos, dlecnt;
	    /* don't count stuffed DLEs in the length */
	    dlecnt = 0;
	    for (pos = 0; pos < (unsigned int)packetlen; pos++)
		if (lexer->inbuffer[pos] == DLE)
		    dlecnt++;
	    if (dlecnt > 2) {
		dlecnt -= 2;
		dlecnt /= 2;
		gpsd_report(LOG_RAW, "Unstuffed %d DLEs\n", dlecnt);
		packetlen -= dlecnt;
	    }
#endif /* TSIP_ENABLE */
	    if (packetlen < 5) {
		lexer->state = GROUND_STATE;
	    } else {
		unsigned int pkt_id;
#ifdef GARMIN_ENABLE
		unsigned int len;
		size_t n;
		unsigned int ch, chksum;
		n = 0;
		/*@ +charint */
#ifdef TSIP_ENABLE
		/* shortcut garmin */
		if (TSIP_PACKET == lexer->type)
		    goto not_garmin;
#endif /* TSIP_ENABLE */
		if (lexer->inbuffer[n++] != DLE)
		    goto not_garmin;
		pkt_id = lexer->inbuffer[n++];	/* packet ID */
		len = lexer->inbuffer[n++];
		chksum = len + pkt_id;
		if (len == DLE) {
		    if (lexer->inbuffer[n++] != DLE)
			goto not_garmin;
		}
		for (; len > 0; len--) {
		    chksum += lexer->inbuffer[n];
		    if (lexer->inbuffer[n++] == DLE) {
			if (lexer->inbuffer[n++] != DLE)
			    goto not_garmin;
		    }
		}
		/* check sum byte */
		ch = lexer->inbuffer[n++];
		chksum += ch;
		if (ch == DLE) {
		    if (lexer->inbuffer[n++] != DLE)
			goto not_garmin;
		}
		if (lexer->inbuffer[n++] != DLE)
		    goto not_garmin;
		/* we used to say n++ here, but scan-build complains */
		if (lexer->inbuffer[n] != ETX)
		    goto not_garmin;
		/*@ +charint */
		chksum &= 0xff;
		if (chksum) {
		    gpsd_report(LOG_IO,
				"Garmin checksum failed: %02x!=0\n", chksum);
		    goto not_garmin;
		}
		packet_accept(lexer, GARMIN_PACKET);
		packet_discard(lexer);
		break;
	      not_garmin:;
		gpsd_report(LOG_RAW + 1, "Not a Garmin packet\n");
#endif /* GARMIN_ENABLE */
#ifdef TSIP_ENABLE
		/* check for some common TSIP packet types:
		 * 0x13, TSIP Parsing Error Notification
		 * 0x41, GPS time, data length 10
		 * 0x42, Single Precision Fix, data length 16
		 * 0x43, Velocity Fix, data length 20
		 * 0x45, Software Version Information, data length 10
		 * 0x46, Health of Receiver, data length 2
		 * 0x48, GPS System Messages
		 * 0x49, Almanac Health Page
		 * 0x4a, LLA Position, data length 20
		 * 0x4b, Machine Code Status, data length 3
		 * 0x4c, Operating Parameters Report
		 * 0x54, One Satellite Bias
		 * 0x56, Velocity Fix (ENU), data length 20
		 * 0x57, Last Computed Fix Report
		 * 0x5a, Raw Measurements
		 * 0x5b, Satellite Ephemeris Status
		 * 0x5c, Satellite Tracking Status, data length 24
		 * 0x5e, Additional Fix Status Report
		 * 0x6d, All-In-View Satellite Selection, data length 16+numSV
		 * 0x82, Differential Position Fix Mode, data length 1
		 * 0x83, Double Precision XYZ, data length 36
		 * 0x84, Double Precision LLA, data length 36
		 * 0xbb, GPS Navigation Configuration
		 * 0xbc, Receiver Port Configuration
		 *
		 * <DLE>[pkt id] [data] <DLE><ETX>
		 */
		/*@ +charint @*/
		pkt_id = lexer->inbuffer[1];	/* packet ID */
                /* *INDENT-OFF* */
		if (!((0x13 == pkt_id) || (0xbb == pkt_id) || (0xbc == pkt_id))
		    && ((0x41 > pkt_id) || (0x8f < pkt_id))) {
		    gpsd_report(LOG_IO,
				"Packet ID 0x%02x out of range for TSIP\n",
				pkt_id);
		    goto not_tsip;
		}
                /* *INDENT-ON* */
		/*@ -ifempty */
		if ((0x13 == pkt_id) && (0x01 <= packetlen))
		    /* pass */ ;
		else if ((0x41 == pkt_id)
			 && ((0x0e == packetlen) || (0x0f == packetlen)))
		    /* pass */ ;
		else if ((0x42 == pkt_id) && (0x14 == packetlen))
		    /* pass */ ;
		else if ((0x43 == pkt_id) && (0x18 == packetlen))
		    /* pass */ ;
		else if ((0x45 == pkt_id) && (0x0e == packetlen))
		    /* pass */ ;
		else if ((0x46 == pkt_id) && (0x06 == packetlen))
		    /* pass */ ;
		else if ((0x48 == pkt_id) && (0x1a == packetlen))
		    /* pass */ ;
		else if ((0x49 == pkt_id) && (0x24 == packetlen))
		    /* pass */ ;
		else if ((0x4a == pkt_id) && (0x18 == packetlen))
		    /* pass */ ;
		else if ((0x4b == pkt_id) && (0x07 == packetlen))
		    /* pass */ ;
		else if ((0x4c == pkt_id) && (0x15 == packetlen))
		    /* pass */ ;
		else if ((0x54 == pkt_id) && (0x10 == packetlen))
		    /* pass */ ;
		else if ((0x55 == pkt_id) && (0x08 == packetlen))
		    /* pass */ ;
		else if ((0x56 == pkt_id) && (0x18 == packetlen))
		    /* pass */ ;
		else if ((0x57 == pkt_id) && (0x0c == packetlen))
		    /* pass */ ;
		else if ((0x5a == pkt_id)
			 && ((0x1d <= packetlen) && (0x1f >= packetlen)))
		    /* pass */ ;
		else if ((0x5b == pkt_id) && (0x24 == packetlen))
		    /* pass */ ;
		else if ((0x5c == pkt_id)
			 && ((0x1c <= packetlen) && (0x1e >= packetlen)))
		    /* pass */ ;
		else if ((0x5e == pkt_id) && (0x06 == packetlen))
		    /* pass */ ;
		else if ((0x5f == pkt_id) && (70 == packetlen))
		    /* pass */ ;
		else if ((0x6d == pkt_id)
			 && ((0x14 <= packetlen) && (0x20 >= packetlen)))
		    /* pass */ ;
		else if ((0x82 == pkt_id) && (0x05 == packetlen))
		    /* pass */ ;
		else if ((0x84 == pkt_id)
			 && ((0x28 <= packetlen) && (0x29 >= packetlen)))
		    /* pass */ ;
		else if ((0x8e == pkt_id))
		    /* pass */ ;
		else if ((0x8f == pkt_id))
		    /* pass */ ;
		else if ((0xbb == pkt_id) && (0x2c == packetlen))
		    /* pass */ ;
		else {
		    gpsd_report(LOG_IO,
				"TSIP REJECT pkt_id = %#02x, packetlen= %zu\n",
				pkt_id, packetlen);
		    goto not_tsip;
		}
		/* Debug */
		gpsd_report(LOG_RAW,
			    "TSIP pkt_id = %#02x, packetlen= %zu\n",
			    pkt_id, packetlen);
		/*@ -charint +ifempty @*/
		packet_accept(lexer, TSIP_PACKET);
		packet_discard(lexer);
		break;
	      not_tsip:
		gpsd_report(LOG_RAW + 1, "Not a TSIP packet\n");
		/*
		 * More attempts to recognize ambiguous TSIP-like
		 * packet types could go here.
		 */
		packet_accept(lexer, BAD_PACKET);
		lexer->state = GROUND_STATE;
		packet_discard(lexer);
		break;
#endif /* TSIP_ENABLE */
	    }
	}
#endif /* TSIP_ENABLE || GARMIN_ENABLE */
#ifdef RTCM104V3_ENABLE
	else if (lexer->state == RTCM3_RECOGNIZED) {
	    if (crc24q_check(lexer->inbuffer,
			     lexer->inbufptr - lexer->inbuffer)) {
		packet_accept(lexer, RTCM3_PACKET);
	    } else {
		gpsd_report(LOG_IO, "RTCM3 data checksum failure, "
			    "%0x against %02x %02x %02x\n",
			    crc24q_hash(lexer->inbuffer,
					lexer->inbufptr - lexer->inbuffer -
					3), lexer->inbufptr[-3],
			    lexer->inbufptr[-2], lexer->inbufptr[-1]);
		packet_accept(lexer, BAD_PACKET);
	    }
	    packet_discard(lexer);
	    lexer->state = GROUND_STATE;
	    break;
	}
#endif /* RTCM104V3_ENABLE */
#ifdef ZODIAC_ENABLE
	else if (lexer->state == ZODIAC_RECOGNIZED) {
	    short len, n, sum;
	    len = getword(2);
	    for (n = sum = 0; n < len; n++)
		sum += getword(5 + n);
	    sum *= -1;
	    if (len == 0 || sum == getword(5 + len)) {
		packet_accept(lexer, ZODIAC_PACKET);
	    } else {
		gpsd_report(LOG_IO,
			    "Zodiac data checksum 0x%hx over length %hd, expecting 0x%hx\n",
			    sum, len, getword(5 + len));
		packet_accept(lexer, BAD_PACKET);
		lexer->state = GROUND_STATE;
	    }
	    packet_discard(lexer);
	    break;
	}
#endif /* ZODIAC_ENABLE */
#ifdef UBX_ENABLE
	else if (lexer->state == UBX_RECOGNIZED) {
	    /* UBX use a TCP like checksum */
	    int n, len;
	    unsigned char ck_a = (unsigned char)0;
	    unsigned char ck_b = (unsigned char)0;
	    len = lexer->inbufptr - lexer->inbuffer;
	    gpsd_report(LOG_IO, "UBX: len %d\n", len);
	    for (n = 2; n < (len - 2); n++) {
		ck_a += lexer->inbuffer[n];
		ck_b += ck_a;
	    }
	    if (ck_a == lexer->inbuffer[len - 2] &&
		ck_b == lexer->inbuffer[len - 1])
		packet_accept(lexer, UBX_PACKET);
	    else {
		gpsd_report(LOG_IO,
			    "UBX checksum 0x%02hhx%02hhx over length %d,"
			    " expecting 0x%02hhx%02hhx (type 0x%02hhx%02hhx)\n",
			    ck_a,
			    ck_b,
			    len,
			    lexer->inbuffer[len - 2],
			    lexer->inbuffer[len - 1],
			    lexer->inbuffer[2], lexer->inbuffer[3]);
		packet_accept(lexer, BAD_PACKET);
		lexer->state = GROUND_STATE;
	    }
	    packet_discard(lexer);
	    break;
	}
#endif /* UBX_ENABLE */
#ifdef EVERMORE_ENABLE
	else if (lexer->state == EVERMORE_RECOGNIZED) {
	    unsigned int n, crc, checksum, len;
	    n = 0;
	    /*@ +charint */
	    if (lexer->inbuffer[n++] != DLE)
		goto not_evermore;
	    if (lexer->inbuffer[n++] != STX)
		goto not_evermore;
	    len = lexer->inbuffer[n++];
	    if (len == DLE) {
		if (lexer->inbuffer[n++] != DLE)
		    goto not_evermore;
	    }
	    len -= 2;
	    crc = 0;
	    for (; len > 0; len--) {
		crc += lexer->inbuffer[n];
		if (lexer->inbuffer[n++] == DLE) {
		    if (lexer->inbuffer[n++] != DLE)
			goto not_evermore;
		}
	    }
	    checksum = lexer->inbuffer[n++];
	    if (checksum == DLE) {
		if (lexer->inbuffer[n++] != DLE)
		    goto not_evermore;
	    }
	    if (lexer->inbuffer[n++] != DLE)
		goto not_evermore;
	    /* we used to say n++ here, but scan-build complains */
	    if (lexer->inbuffer[n] != ETX)
		goto not_evermore;
	    crc &= 0xff;
	    if (crc != checksum) {
		gpsd_report(LOG_IO,
			    "EverMore checksum failed: %02x != %02x\n",
			    crc, checksum);
		goto not_evermore;
	    }
	    /*@ +charint */
	    packet_accept(lexer, EVERMORE_PACKET);
	    packet_discard(lexer);
	    break;
	  not_evermore:
	    packet_accept(lexer, BAD_PACKET);
	    lexer->state = GROUND_STATE;
	    packet_discard(lexer);
	    break;
	}
#endif /* EVERMORE_ENABLE */
/* XXX CSK */
#ifdef ITRAX_ENABLE
#define getib(j) ((uint8_t)lexer->inbuffer[(j)])
#define getiw(i) ((uint16_t)(((uint16_t)getib((i)+1) << 8) | (uint16_t)getib((i))))

	else if (lexer->state == ITALK_RECOGNIZED) {
	    volatile uint16_t len, n, csum, xsum;

	    /* number of words */
	    len = (uint16_t) (lexer->inbuffer[6] & 0xff);

	    /*@ -type @*/
	    /* expected checksum */
	    xsum = getiw(7 + 2 * len);


	    csum = 0;
	    for (n = 0; n < len; n++) {
		volatile uint16_t tmpw = getiw(7 + 2 * n);
		volatile uint32_t tmpdw  = (csum + 1) * (tmpw + n);
		csum ^= (tmpdw & 0xffff) ^ ((tmpdw >> 16) & 0xffff);
	    }
	    /*@ +type @*/
	    if (len == 0 || csum == xsum)
		packet_accept(lexer, ITALK_PACKET);
	    else {
		gpsd_report(LOG_IO,
			    "ITALK: checksum failed - "
			    "type 0x%02x expected 0x%04x got 0x%04x\n",
			    lexer->inbuffer[4], xsum, csum);
		packet_accept(lexer, BAD_PACKET);
		lexer->state = GROUND_STATE;
	    }
	    packet_discard(lexer);
	    break;
	}
#undef getiw
#undef getib
#endif /* ITRAX_ENABLE */
#ifdef NAVCOM_ENABLE
	else if (lexer->state == NAVCOM_RECOGNIZED) {
	    /* By the time we got here we know checksum is OK */
	    packet_accept(lexer, NAVCOM_PACKET);
	    packet_discard(lexer);
	    break;
	}
#endif /* NAVCOM_ENABLE */
#ifdef GEOSTAR_ENABLE
	else if (lexer->state == GEOSTAR_RECOGNIZED) {
	    /* GeoStar uses a XOR 32bit checksum */
	    int n, len;
	    unsigned int cs = 0L;
	    len = lexer->inbufptr - lexer->inbuffer;

	    /* Calculate checksum */
	    for (n = 0; n < len; n += 4) {
		cs ^= getleu32(lexer->inbuffer, n);
	    }

	    if (cs == 0)
		packet_accept(lexer, GEOSTAR_PACKET);
	    else {
		gpsd_report(LOG_IO,
			    "GeoStar checksum failed 0x%x over length %d\n",
			    cs, len);
		packet_accept(lexer, BAD_PACKET);
		lexer->state = GROUND_STATE;
	    }
	    packet_discard(lexer);
	    break;
	}
#endif /* GEOSTAR_ENABLE */
#ifdef RTCM104V2_ENABLE
	else if (lexer->state == RTCM2_RECOGNIZED) {
	    /*
	     * RTCM packets don't have checksums.  The six bits of parity
	     * per word and the preamble better be good enough.
	     */
	    packet_accept(lexer, RTCM2_PACKET);
	    lexer->state = RTCM2_SYNC_STATE;
	    packet_discard(lexer);
	    break;
	}
#endif /* RTCM104V2_ENABLE */
#ifdef GARMINTXT_ENABLE
	else if (lexer->state == GTXT_RECOGNIZED) {
	    size_t packetlen = lexer->inbufptr - lexer->inbuffer;
	    if (57 <= packetlen) {
		packet_accept(lexer, GARMINTXT_PACKET);
		packet_discard(lexer);
		lexer->state = GROUND_STATE;
		break;
	    } else {
		packet_accept(lexer, BAD_PACKET);
		lexer->state = GROUND_STATE;
	    }
	}
#endif
#ifdef PASSTHROUGH_ENABLE
	else if (lexer->state == JSON_RECOGNIZED) {
	    size_t packetlen = lexer->inbufptr - lexer->inbuffer;
	    if (packetlen >= 11)
		/* {"class": } */
		packet_accept(lexer, JSON_PACKET);
	    else
		packet_accept(lexer, BAD_PACKET);
	    packet_discard(lexer);
	    lexer->state = GROUND_STATE;
	    break;
	}
#endif /* PASSTHROUGH_ENABLE */
    }				/* while */
}

#undef getword

ssize_t packet_get(int fd, struct gps_packet_t *lexer)
/* grab a packet; return -1=>I/O error, 0=>EOF, BAD_PACKET or a length */
{
    ssize_t recvd;

    /*@ -modobserver @*/
    errno = 0;
    recvd = read(fd, lexer->inbuffer + lexer->inbuflen,
		 sizeof(lexer->inbuffer) - (lexer->inbuflen));
    /*@ +modobserver @*/
    if (recvd == -1) {
	if ((errno == EAGAIN) || (errno == EINTR)) {
	    gpsd_report(LOG_RAW + 2, "no bytes ready\n");
	    recvd = 0;
	    /* fall through, input buffer may be nonempty */
	} else {
	    gpsd_report(LOG_RAW + 2, "errno: %s\n", strerror(errno));
	    return -1;
	}
    } else {
	if (lexer->debug >= LOG_RAW+1)
	    gpsd_report(LOG_RAW + 1,
			"Read %zd chars to buffer offset %zd (total %zd): %s\n",
			recvd, lexer->inbuflen, lexer->inbuflen + recvd,
			gpsd_hexdump((char *)lexer->inbufptr, (size_t) recvd));
	lexer->inbuflen += recvd;
    }
    gpsd_report(LOG_SPIN, "packet_get() fd %d -> %zd (%d)\n",
		fd, recvd, errno);

    /*
     * Bail out, indicating no more input, only if we just received
     * nothing from the device and there is nothing waiting in the
     * packet input buffer.
     */
    if (recvd <= 0 && packet_buffered_input(lexer) <= 0)
	return recvd;

    /* Otherwise, consume from the packet input buffer */
    packet_parse(lexer);

    /* if input buffer is full, discard */
    if (sizeof(lexer->inbuffer) == (lexer->inbuflen)) {
	packet_discard(lexer);
	lexer->state = GROUND_STATE;
    }

    /*
     * If we gathered a packet, return its length; it will have been
     * consumed out of the input buffer and moved to the output
     * buffer.  We don't care whether the read() returned 0 or -1 and
     * gathered packet data was all buffered or whether ot was partly
     * just physically read.
     *
     * Note: this choice greatly simplifies life for callers of
     * packet_get(), but means that they cannot tell when a nonzero
     * return means there was a successful physical read.  They will
     * thus credit a data source that drops out with being alive
     * slightly longer than it actually was.  This is unlikely to
     * matter as long as any policy timeouts are large compared to
     * the time required to consume the greatest possible amount
     * of buffered input, but if you hack this code you need to
     * be aware of the issue. It might also slightly affect
     * performance profiling.
     */
    if (lexer->outbuflen > 0)
	return (ssize_t) lexer->outbuflen;
    else
	/*
	 * Otherwise recvd is the size of whatever packet fragment we got.
	 * It can still be 0 or -1 at this point even if buffer data
	 * was consumed.
	 */
	return recvd;
}

void packet_reset( /*@out@*/ struct gps_packet_t *lexer)
/* return the packet machine to the ground state */
{
    lexer->type = BAD_PACKET;
    lexer->state = GROUND_STATE;
    lexer->inbuflen = 0;
    lexer->inbufptr = lexer->inbuffer;
#ifdef BINARY_ENABLE
    isgps_init(lexer);
#endif /* BINARY_ENABLE */
}


#ifdef __UNUSED__
void packet_pushback(struct gps_packet_t *lexer)
/* push back the last packet grabbed */
{
    if (lexer->outbuflen + lexer->inbuflen < MAX_PACKET_LENGTH) {
	memmove(lexer->inbuffer + lexer->outbuflen,
		lexer->inbuffer, lexer->inbuflen);
	memmove(lexer->inbuffer, lexer->outbuffer, lexer->outbuflen);
	lexer->inbuflen += lexer->outbuflen;
	lexer->inbufptr += lexer->outbuflen;
	lexer->outbuflen = 0;
    }
}
#endif /* __UNUSED */

