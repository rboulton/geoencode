/** @file geoencode_test.cc
 * @brief Tests for encoding of geospatial coordinates.
 */
/* Copyright (C) 2011 Richard Boulton
 * Based closely on a python version, copyright (C) 2010 Olly Betts
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <config.h>
#include "geoencode.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

using namespace std;

/** Check that encoding and then decoding a lat, lon pair gives expected
 *  values.
 */
bool check(double lat, double lon, double expected_lat, double expected_lon) {
    string encoded;
    bool ok = GeoEncode::encode(lat, lon, encoded);
    if (!ok) {
	fprintf(stderr, "encoding failed\n");
	return false;
    }
    double decoded_lat, decoded_lon;
    GeoEncode::decode(encoded, decoded_lat, decoded_lon);
    if (fabs(decoded_lat - expected_lat) > 0.00000001) {
	fprintf(stderr, "decoded_lat != expected_lat: %.15g != "
		"%.15g (input=%.15g,%.15g)\n", decoded_lat, expected_lat,
		lat, lon);
	return false;
    }
    if (fabs(decoded_lon - expected_lon) > 0.00000001) {
	fprintf(stderr, "decoded_lon != expected_lon: %.15g != "
		"%.15g (input=%.15g,%.15g)\n", decoded_lon, expected_lon,
		lat, lon);
	return false;
    }
    return true;
}

/** Check that encoding and then decoding a lat, lon pair returns them.
 */
bool check(double lat, double lon) {
    return check(lat, lon, lat, lon);
}

/** Check that encoding a given lat, lon pair returns an error.
 */
bool check_fail(double lat, double lon) {
    string encoded;
    bool ok = GeoEncode::encode(lat, lon, encoded);
    if (ok) {
	fprintf(stderr, "expected failure but encoded without error\n");
	return false;
    }
    return true;
}

/** Check that encoding and then decoding a lat, lon pair with a bounding box
 *  returns the appropriate value.
 */
bool check_bb(GeoEncode::DecoderWithBoundingBox & bb, double lat, double lon,
	      bool in_box)
{
    string encoded;
    bool ok = GeoEncode::encode(lat, lon, encoded);
    if (!ok) {
	fprintf(stderr, "encoding failed\n");
	return false;
    }
    double decoded_lat, decoded_lon;
    if (bb.decode(encoded, decoded_lat, decoded_lon)) {
	double decoded_lat2, decoded_lon2;
	GeoEncode::decode(encoded, decoded_lat2, decoded_lon2);
	if (decoded_lat != decoded_lat2) {
	    fprintf(stderr, "decoded_lat != decoded_lat2: %.15g != %.15g (input=%.15g,%.15g)\n", decoded_lat, decoded_lat2, lat, lon);
	    return false;
	}
	if (decoded_lon != decoded_lon2) {
	    fprintf(stderr, "decoded_lon != decoded_lon2: %.15g != %.15g (input=%.15g,%.15g)\n", decoded_lon, decoded_lon2, lat, lon);
	    return false;
	}
	if (!in_box) {
	    fprintf(stderr, "(%.15g, %.15g) was expected to be outside bounding box, but decode() claims it's inside the box\n", lat, lon);
	    return false;
	}
    } else {
	if (in_box) {
	    fprintf(stderr, "(%.15g, %.15g) was expected to be inside bounding box, but decode() claims it's outside the box\n", lat, lon);
	    return false;
	}
    }
    return true;
}

int main() {
    // Check some roundtrips of things which encode precisely.
    // (encoding resolution is 16ths of a second).
    check(0, 0);
    check(0.2, 23.8);
    check((7)/(3600.0 * 16),
	  (7)/(3600.0 * 16));

    check(10 + 7/60.0 + 5/3600.0 + 7/(3600.0 * 16),
	  10 + 7/60.0 + 5/3600.0 + 7/(3600.0 * 16));
    check(-(10 + 7/60.0 + 5/3600.0 + 7/(3600.0 * 16)),
	  -(10 + 7/60.0 + 5/3600.0 + 7/(3600.0 * 16)),
	  -(10 + 7/60.0 + 5/3600.0 + 7/(3600.0 * 16)),
	  360 -(10 + 7/60.0 + 5/3600.0 + 7/(3600.0 * 16))); 

    // This one is worth checking because it encodes the second byte as zero.
    check(10, 96);

    // Check the poles
    check(-90, 0);
    check(90, 0);
    check(-90, 1, -90, 0);
    check(90, 1, 90, 0);
    check(-89.9999999, 1, -90, 0);
    check(89.9999999, 1, 90, 0);
    check(0, 359.9999999, 0, 0);
    check(0, -359.9999999, 0, 0);

    check(-89.9999999, 359.9999999, -90, 0);
    check(-89.9999999, -359.9999999, -90, 0);
    check(89.9999999, 359.9999999, 90, 0);
    check(89.9999999, -359.9999999, 90, 0);

    // Check that encoding invalid coordinates fails.
    check_fail(91, 0);
    check_fail(-91, 0);

    // Check round trip of encoding and decoding 1,000,000 coordinates.
    for (int i = 0; i != 1000000; ++i) {
	double lat = ((random() * 180.0) / RAND_MAX) - 90.0;
	double lon = ((random() * 360.0 * 10) / RAND_MAX) - (360.0 * 5);
	check(lat, lon,
	      round(lat * (3600.0 * 16)) / (3600.0 * 16),
	      round(fmod(lon + 3600.0, 360.0) * (3600.0 * 16)) / (3600.0 * 16)
	     );
    }

    // Check decoding using a bounding box which includes the south pole.
    GeoEncode::DecoderWithBoundingBox bb(-90, -60, 10, 50);
    check_bb(bb, -90, 0, true);
    check_bb(bb, -90, 49, true);
    check_bb(bb, -90, 50, true);
    check_bb(bb, -90, 51, true);
    check_bb(bb, -90, 299, true);
    check_bb(bb, -90, 300, true);
    check_bb(bb, -90, 301, true);
    check_bb(bb, -90, 360, true);

    check_bb(bb, 0, 0, true);
    check_bb(bb, 0, 49, true);
    check_bb(bb, 0, 50, true);
    check_bb(bb, 0, 51, false);
    check_bb(bb, 0, 299, false);
    check_bb(bb, 0, 300, true);
    check_bb(bb, 0, 301, true);
    check_bb(bb, 0, 360, true);

    check_bb(bb, 10, 0, true);
    check_bb(bb, 10, 49, true);
    check_bb(bb, 10, 50, true);
    check_bb(bb, 10, 51, false);
    check_bb(bb, 10, 299, false);
    check_bb(bb, 10, 300, true);
    check_bb(bb, 10, 301, true);
    check_bb(bb, 10, 360, true);

    check_bb(bb, 20, 0, false);
    check_bb(bb, 20, 49, false);
    check_bb(bb, 20, 50, false);
    check_bb(bb, 20, 51, false);
    check_bb(bb, 20, 299, false);
    check_bb(bb, 20, 300, false);
    check_bb(bb, 20, 301, false);
    check_bb(bb, 20, 360, false);

    check_bb(bb, 90, 0, false);
    check_bb(bb, 90, 49, false);
    check_bb(bb, 90, 50, false);
    check_bb(bb, 90, 51, false);
    check_bb(bb, 90, 299, false);
    check_bb(bb, 90, 300, false);
    check_bb(bb, 90, 301, false);
    check_bb(bb, 90, 360, false);

    // Check decoding using a bounding box which includes the north pole.
    bb = GeoEncode::DecoderWithBoundingBox(-10, -60, 90, 50);
    check_bb(bb, -90, 0, false);
    check_bb(bb, -90, 49, false);
    check_bb(bb, -90, 50, false);
    check_bb(bb, -90, 51, false);
    check_bb(bb, -90, 299, false);
    check_bb(bb, -90, 300, false);
    check_bb(bb, -90, 301, false);
    check_bb(bb, -90, 360, false);

    check_bb(bb, 0, 0, true);
    check_bb(bb, 0, 49, true);
    check_bb(bb, 0, 50, true);
    check_bb(bb, 0, 51, false);
    check_bb(bb, 0, 299, false);
    check_bb(bb, 0, 300, true);
    check_bb(bb, 0, 301, true);
    check_bb(bb, 0, 360, true);

    check_bb(bb, 90, 0, true);
    check_bb(bb, 90, 49, true);
    check_bb(bb, 90, 50, true);
    check_bb(bb, 90, 51, true);
    check_bb(bb, 90, 299, true);
    check_bb(bb, 90, 300, true);
    check_bb(bb, 90, 301, true);
    check_bb(bb, 90, 360, true);

    // Check decoding using a bounding box which includes no poles.
    bb = GeoEncode::DecoderWithBoundingBox(-10, 0, 10, 50);
    check_bb(bb, -90, 0, false);
    check_bb(bb, -90, 49, false);
    check_bb(bb, -90, 50, false);
    check_bb(bb, -90, 51, false);
    check_bb(bb, -90, 299, false);
    check_bb(bb, -90, 300, false);
    check_bb(bb, -90, 301, false);
    check_bb(bb, -90, 360, false);

    check_bb(bb, 0, 0, true);
    check_bb(bb, 0, 49, true);
    check_bb(bb, 0, 50, true);
    check_bb(bb, 0, 51, false);
    check_bb(bb, 0, 300, false);
    check_bb(bb, 0, 359, false);
    check_bb(bb, 0, 360, true);

    check_bb(bb, 90, 0, false);
    check_bb(bb, 90, 49, false);
    check_bb(bb, 90, 50, false);
    check_bb(bb, 90, 51, false);
    check_bb(bb, 90, 299, false);
    check_bb(bb, 90, 300, false);
    check_bb(bb, 90, 301, false);
    check_bb(bb, 90, 360, false);

    // Check decoding using random bounding boxes.
    for (int i = 0; i != 1000000; ++i) {
	double lat1 = ((random() * 180.0) / RAND_MAX) - 90.0;
	double lon1 = ((random() * 360.0 * 10) / RAND_MAX) - (360.0 * 5);
	double lat2 = ((random() * 180.0) / RAND_MAX) - 90.0;
	double lon2 = ((random() * 360.0 * 10) / RAND_MAX) - (360.0 * 5);

	// Increase chance of a pole being involved to 1 in 10 (for each pole)
	if (random() % 10 == 0) {
	    lat1 = -90.0;
	}
	if (random() % 10 == 0) {
	    lat2 = 90.0;
	}

	if (lat1 > lat2) {
	    // ensure lat1 <= lat2
	    double tmp = lat1;
	    lat1 = lat2;
	    lat2 = tmp;
	}
	// longitudes can be in either order.
	bb = GeoEncode::DecoderWithBoundingBox(lat1, lon1, lat2, lon2);

	// wrap the longitudes to make it easier to calculate what in_box
	// should be, below.
	lon1 = fmod(lon1, 360.0);
	if (lon1 < 0) {
	    lon1 += 360;
	}
	lon2 = fmod(lon2, 360.0);
	if (lon2 < 0) {
	    lon2 += 360;
	}

	// Pick a random coordinate
	double lat = ((random() * 180.0) / RAND_MAX) - 90.0;
	double lon = ((random() * 360.0 * 10) / RAND_MAX) - (360.0 * 5);

	// Increase chance that the coordinate is on one of the bounding box edges.
	if (random() % 40 == 0) {
	    lat = lat1;
	}
	if (random() % 40 == 0) {
	    lat = lat2;
	}
	if (random() % 40 == 0) {
	    lon = lon1;
	}
	if (random() % 40 == 0) {
	    lon = lon2;
	}

	// Round to a coordinate that can be accurately represented.
	lat = round(lat * (3600.0 * 16)) / (3600.0 * 16);
	lon = round(fmod(lon + 3600.0, 360.0) * (3600.0 * 16)) / (3600.0 * 16);

	// Work out whether this coordinate would be in the box.
	bool in_box = true;
	{
	    double lon_wrapped = fmod(lon, 360.0);
	    if (lon_wrapped < 0) {
		lon_wrapped += 360;
	    }
	    if (lat < lat1 || lat > lat2) {
		in_box = false;
	    } else if (lat == -90 && lat1 == -90) {
		// in box
	    } else if (lat == 90 && lat2 == 90) {
		// in box
	    } else if (lon1 < lon2) {
		if (lon_wrapped < lon1 || lon2 < lon_wrapped) {
		    in_box = false;
		}
	    } else {
		if (lon2 < lon_wrapped && lon_wrapped < lon1) {
		    in_box = false;
		}
	    }
	}

	string encoded;
	check_bb(bb, lat, lon, in_box);
    }

    return 0;
}
