/* json.c - unit test for JSON partsing into fixed-extent structures
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * BSD terms apply: see the file COPYING in the distribution root for details.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <getopt.h>

#include "gpsd.h"
#include "gps_json.h"


static void assert_case(int num, int status)
{
    if (status != 0) {
	(void)fprintf(stderr, "case %d FAILED, status %d (%s).\n", num,
		      status, json_error_string(status));
	exit(1);
    }
}

static void assert_string(char *attr, char *fld, char *val)
{
    if (strcmp(fld, val)) {
	(void)fprintf(stderr,
		      "'%s' string attribute eval failed, value = %s.\n",
		      attr, fld);
	exit(1);
    }
}

static void assert_integer(char *attr, int fld, int val)
{
    if (fld != val) {
	(void)fprintf(stderr,
		      "'%s' integer attribute eval failed, value = %d.\n",
		      attr, fld);
	exit(1);
    }
}

static void assert_uinteger(char *attr, uint fld, uint val)
{
    if (fld != val) {
	(void)fprintf(stderr,
		      "'%s' integer attribute eval failed, value = %u.\n",
		      attr, fld);
	exit(1);
    }
}

static void assert_boolean(char *attr, bool fld, bool val)
{
    /*@-boolcompare@*/
    if (fld != val) {
	(void)fprintf(stderr,
		      "'%s' boolean attribute eval failed, value = %s.\n",
		      attr, fld ? "true" : "false");
	exit(1);
    }
    /*@+boolcompare@*/
}

/*
 * Floating point comparisons are iffy, but at least if any of these fail
 * the output will make it clear whether it was a precision issue
 */
static void assert_real(char *attr, double fld, double val)
{
    if (fld != val) {
	(void)fprintf(stderr,
		      "'%s' real attribute eval failed, value = %f.\n", attr,
		      fld);
	exit(1);
    }
}

/*@ -fullinitblock @*/

static struct gps_data_t gpsdata;

/* Case 1: TPV report */

/* *INDENT-OFF* */
static const char json_str1[] = "{\"class\":\"TPV\",\
    \"device\":\"GPS#1\",\"tag\":\"MID2\",				\
    \"time\":\"2005-06-19T08:12:41.89Z\",\"lon\":46.498203637,\"lat\":7.568074350,\
    \"alt\":1327.780,\"epx\":21.000,\"epy\":23.000,\"epv\":124.484,\"mode\":3}";

/* Case 2: SKY report */

static const char *json_str2 = "{\"class\":\"SKY\",\
         \"tag\":\"MID4\",\"time\":\"2005-06-19T12:12:42.03Z\",   \
         \"satellites\":[\
         {\"PRN\":10,\"el\":45,\"az\":196,\"ss\":34,\"used\":true},\
         {\"PRN\":29,\"el\":67,\"az\":310,\"ss\":40,\"used\":true},\
         {\"PRN\":28,\"el\":59,\"az\":108,\"ss\":42,\"used\":true},\
         {\"PRN\":26,\"el\":51,\"az\":304,\"ss\":43,\"used\":true},\
         {\"PRN\":8,\"el\":44,\"az\":58,\"ss\":41,\"used\":true},\
         {\"PRN\":27,\"el\":16,\"az\":66,\"ss\":39,\"used\":true},\
         {\"PRN\":21,\"el\":10,\"az\":301,\"ss\":0,\"used\":false}]}";

/* Case 3: String list syntax */

static const char *json_str3 = "[\"foo\",\"bar\",\"baz\"]";

static char *stringptrs[3];
static char stringstore[256];
static int stringcount;

/*@-type@*/
static const struct json_array_t json_array_3 = {
    .element_type = t_string,
    .arr.strings.ptrs = stringptrs,
    .arr.strings.store = stringstore,
    .arr.strings.storelen = sizeof(stringstore),
    .count = &stringcount,
    .maxlen = sizeof(stringptrs)/sizeof(stringptrs[0]),
};
/*@+type@*/

/* Case 4: test defaulting of unspecified attributes */

static const char *json_str4 = "{\"flag1\":true,\"flag2\":false}";

static bool flag1, flag2;
static double dftreal;
static int dftinteger;
static unsigned int dftuinteger;

static const struct json_attr_t json_attrs_4[] = {
    {"dftint",  t_integer, .addr.integer = &dftinteger, .dflt.integer = -5},
    {"dftuint", t_integer, .addr.uinteger = &dftuinteger, .dflt.uinteger = 10},
    {"dftreal", t_real,    .addr.real = &dftreal,       .dflt.real = 23.17},
    {"flag1",   t_boolean, .addr.boolean = &flag1,},
    {"flag2",   t_boolean, .addr.boolean = &flag2,},
    {NULL},
};

/* Case 5: test DEVICE parsing */

static const char *json_str5 = "{\"class\":\"DEVICE\",\
           \"path\":\"/dev/ttyUSB0\",\
           \"flags\":5,\
           \"driver\":\"Foonly\",\"subtype\":\"Foonly Frob\"\
           }";

/* Case 6: test parsing of subobject list into array of structures */

static const char *json_str6 = "{\"parts\":[\
           {\"name\":\"Urgle\", \"flag\":true, \"count\":3},\
           {\"name\":\"Burgle\",\"flag\":false,\"count\":1},\
           {\"name\":\"Witter\",\"flag\":true, \"count\":4},\
           {\"name\":\"Thud\",  \"flag\":false,\"count\":1}]}";

struct dumbstruct_t {
    char name[64];
    bool flag;
    int count;
};
static struct dumbstruct_t dumbstruck[5];
static int dumbcount;

/*@-type@*/
static const struct json_attr_t json_attrs_6_subtype[] = {
    {"name",  t_string,  .addr.offset = offsetof(struct dumbstruct_t, name),
                         .len = 64},
    {"flag",  t_boolean, .addr.offset = offsetof(struct dumbstruct_t, flag),},
    {"count", t_integer, .addr.offset = offsetof(struct dumbstruct_t, count),},
    {NULL},
};

static const struct json_attr_t json_attrs_6[] = {
    {"parts", t_array, .addr.array.element_type = t_structobject,
                       .addr.array.arr.objects.base = (char*)&dumbstruck,
                       .addr.array.arr.objects.stride = sizeof(struct dumbstruct_t),
                       .addr.array.arr.objects.subtype = json_attrs_6_subtype,
                       .addr.array.count = &dumbcount,
                       .addr.array.maxlen = sizeof(dumbstruck)/sizeof(dumbstruck[0])},
    {NULL},
};
/*@+type@*/

/* Case 7: test parsing of version response */

static const char *json_str7 = "{\"class\":\"VERSION\",\
           \"release\":\"2.40dev\",\"rev\":\"dummy-revision\",\
           \"proto_major\":3,\"proto_minor\":1}";

/* Case 8: test parsing arrays of enumerated types */

static const char *json_str8 = "{\"fee\":\"FOO\",\"fie\":\"BAR\",\"foe\":\"BAZ\"}";
static const struct json_enum_t enum_table[] = {
    {"BAR", 6}, {"FOO", 3}, {"BAZ", 14}, {NULL}
};

static int fee, fie, foe;
static const struct json_attr_t json_attrs_8[] = {
    {"fee",  t_integer, .addr.integer = &fee, .map=enum_table},
    {"fie",  t_integer, .addr.integer = &fie, .map=enum_table},
    {"foe",  t_integer, .addr.integer = &foe, .map=enum_table},
    {NULL},
};
/*@ +fullinitblock @*/
/* *INDENT-ON* */

int main(int argc UNUSED, char *argv[]UNUSED)
{
    int status = 0;
    int option;

    while ((option = getopt(argc, argv, "hD:?")) != -1) {
	switch (option) {
	case 'D':
	    gps_enable_debug(atoi(optarg), stdout);
	    break;
	case '?':
	case 'h':
	default:
	    (void)fputs("usage: test_json [-D lvl]\n", stderr);
	    exit(1);
	}
    }

    (void)fprintf(stderr, "JSON unit test ");

    status = libgps_json_unpack(json_str1, &gpsdata, NULL);
    assert_case(1, status);
    assert_string("device", gpsdata.dev.path, "GPS#1");
    assert_string("tag", gpsdata.tag, "MID2");
    assert_integer("mode", gpsdata.fix.mode, 3);
    assert_real("time", gpsdata.fix.time, 1119168761.8900001);
    assert_real("lon", gpsdata.fix.longitude, 46.498203637);
    assert_real("lat", gpsdata.fix.latitude, 7.568074350);

    status = libgps_json_unpack(json_str2, &gpsdata, NULL);
    assert_case(2, status);
    assert_string("tag", gpsdata.tag, "MID4");
    assert_integer("used", gpsdata.satellites_used, 6);
    assert_integer("PRN[0]", gpsdata.PRN[0], 10);
    assert_integer("el[0]", gpsdata.elevation[0], 45);
    assert_integer("az[0]", gpsdata.azimuth[0], 196);
    assert_real("ss[0]", gpsdata.ss[0], 34);
    assert_integer("used[0]", gpsdata.used[0], 10);
    assert_integer("used[5]", gpsdata.used[5], 27);
    assert_integer("PRN[6]", gpsdata.PRN[6], 21);
    assert_integer("el[6]", gpsdata.elevation[6], 10);
    assert_integer("az[6]", gpsdata.azimuth[6], 301);
    assert_real("ss[6]", gpsdata.ss[6], 0);

    status = json_read_array(json_str3, &json_array_3, NULL);
    assert_case(3, status);
    assert(stringcount == 3);
    assert(strcmp(stringptrs[0], "foo") == 0);
    assert(strcmp(stringptrs[1], "bar") == 0);
    assert(strcmp(stringptrs[2], "baz") == 0);

    status = json_read_object(json_str4, json_attrs_4, NULL);
    assert_case(4, status);
    assert_integer("dftint", dftinteger, -5);	/* did the default work? */
    assert_uinteger("dftuint", dftuinteger, 10);	/* did the default work? */
    assert_real("dftreal", dftreal, 23.17);	/* did the default work? */
    assert_boolean("flag1", flag1, true);
    assert_boolean("flag2", flag2, false);

    status = libgps_json_unpack(json_str5, &gpsdata, NULL);
    assert_case(5, status);
    assert_string("path", gpsdata.dev.path, "/dev/ttyUSB0");
    assert_integer("flags", gpsdata.dev.flags, 5);
    assert_string("driver", gpsdata.dev.driver, "Foonly");

    status = json_read_object(json_str6, json_attrs_6, NULL);
    assert_case(6, status);
    assert_integer("dumbcount", dumbcount, 4);
    assert_string("dumbstruck[0].name", dumbstruck[0].name, "Urgle");
    assert_string("dumbstruck[1].name", dumbstruck[1].name, "Burgle");
    assert_string("dumbstruck[2].name", dumbstruck[2].name, "Witter");
    assert_string("dumbstruck[3].name", dumbstruck[3].name, "Thud");
    assert_boolean("dumbstruck[0].flag", dumbstruck[0].flag, true);
    assert_boolean("dumbstruck[1].flag", dumbstruck[1].flag, false);
    assert_boolean("dumbstruck[2].flag", dumbstruck[2].flag, true);
    assert_boolean("dumbstruck[3].flag", dumbstruck[3].flag, false);
    assert_integer("dumbstruck[0].count", dumbstruck[0].count, 3);
    assert_integer("dumbstruck[1].count", dumbstruck[1].count, 1);
    assert_integer("dumbstruck[2].count", dumbstruck[2].count, 4);
    assert_integer("dumbstruck[3].count", dumbstruck[3].count, 1);

    status = libgps_json_unpack(json_str7, &gpsdata, NULL);
    assert_case(7, status);
    assert_string("release", gpsdata.version.release, "2.40dev");
    assert_string("rev", gpsdata.version.rev, "dummy-revision");
    assert_integer("proto_major", gpsdata.version.proto_major, 3);
    assert_integer("proto_minor", gpsdata.version.proto_minor, 1);

    status = json_read_object(json_str8, json_attrs_8, NULL);
    assert_case(8, status);
    assert_integer("fee", fee, 3);
    assert_integer("fie", fie, 6);
    assert_integer("foe", foe, 14);

    (void)fprintf(stderr, "succeeded.\n");

    exit(0);
}
