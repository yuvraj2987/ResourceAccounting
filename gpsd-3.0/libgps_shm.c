/****************************************************************************

NAME
   libgps_shm.c - reasder access to shared-memory export

DESCRIPTION
   This is a very lightweight alternative to JSON-over-sockets.  Clients
won't be able to filter by device, and won't get device activation/deactivation
notifications.  But both client and daemon will avoid all the marshalling and 
unmarshalling overhead.

PERMISSIONS
   This file is Copyright (c) 2010 by the GPSD project
   BSD terms apply: see the file COPYING in the distribution root for details.

***************************************************************************/
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "gpsd.h"

#ifdef SHM_EXPORT_ENABLE

int gps_shm_open(/*@out@*/struct gps_data_t *gpsdata)
/* open a shared-memory connection to the daemon */
{
    int shmid;

    libgps_debug_trace((DEBUG_CALLS, "gps_shm_open()\n"));

    gpsdata->privdata = NULL;
    shmid = shmget((key_t)GPSD_KEY, sizeof(struct gps_data_t), 0);
    if (shmid == -1) {
	/* daemon isn't running or failed to create shared segment */
	return -1;
    } 
    gpsdata->privdata = shmat(shmid, 0, 0);
    if ((int)(long)gpsdata->privdata == -1) {
	/* attach failed for sume unknown reason */
	return -2;
    }
#ifndef USE_QT
    gpsdata->gps_fd = -1;
#else
    gpsdata->gps_fd = (void *)(intptr_t)-1;
#endif /* USE_QT */
    return 0;
}

int gps_shm_read(struct gps_data_t *gpsdata)
/* read an update from the shared-memory segment */
{
    /*@ -compdestroy */
    if (gpsdata->privdata == NULL)
	return -1;
    else
    {
	int before, after;
	void *private_save = gpsdata->privdata;
	volatile struct shmexport_t *shared = (struct shmexport_t *)gpsdata->privdata;
	struct gps_data_t noclobber;

	/*
	 * Following block of instructions must not be reordered, otherwise 
	 * havoc will ensue.  The barrier() call should prevent reordering
	 * of the data accesses.
	 *
	 * This is a simple optimistic-concurrency technique.  We wrote
	 * the second bookend first, then the data, then the first bookend.
	 * Reader copies what it sees in normal order; that way, if we
	 * start to write the segment during the read, the second bookend will
	 * get clobbered first and the data can be detected as bad.
	 */
	before = shared->bookend1;
	barrier();
	(void)memcpy((void *)&noclobber, 
		     (void *)&shared->gpsdata, 
		     sizeof(struct gps_data_t));
	barrier();
	after = shared->bookend2;

	if (before != after) 
	    return 0;
	else {
	    (void)memcpy((void *)gpsdata, 
			 (void *)&noclobber, 
			 sizeof(struct gps_data_t));
	    /*@i1@*/gpsdata->privdata = private_save;
	    if ((gpsdata->set & REPORT_IS)!=0) {
		if (gpsdata->fix.mode >= 2)
		    gpsdata->status = STATUS_FIX;
		else
		    gpsdata->status = STATUS_NO_FIX;
		gpsdata->set = STATUS_SET;
	    }
	    return (int)sizeof(struct gps_data_t);
	}
    }
    /*@ +compdestroy */
}

void gps_shm_close(struct gps_data_t *gpsdata)
{
    if (gpsdata->privdata != NULL)
	(void)shmdt((const void *)gpsdata->privdata);
}

#endif /* SHM_EXPORT_ENABLE */

/* end */
