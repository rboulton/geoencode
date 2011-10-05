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

bool check(double lat, double lon, double expected_lat, double expected_lon) {
    string encoded;
    bool ok = geo_encode(lat, lon, encoded);
    if (!ok) {
	fprintf(stderr, "encoding failed\n");
	return false;
    }
    LatLongCoord result = geo_decode(encoded);
    if (fabs(result.lat - expected_lat) > 0.00000001) {
	fprintf(stderr, "result.lat != expected_lat: %.15g != %.15g (input=%.15g,%.15g)\n", result.lat, expected_lat, lat, lon);
	return false;
    }
    if (fabs(result.lon - expected_lon) > 0.00000001) {
	fprintf(stderr, "result.lon != expected_lon: %.15g != %.15g (input=%.15g,%.15g)\n", result.lon, expected_lon, lat, lon);
	return false;
    }
    return true;
}

bool check(double lat, double lon) {
    return check(lat, lon, lat, lon);
}

bool check_fail(double lat, double lon) {
    string encoded;
    bool ok = geo_encode(lat, lon, encoded);
    if (ok) {
	fprintf(stderr, "expected failure but encoded without error\n");
	return false;
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

    for (int i = 0; i != 1000000; ++i) {
	double lat = ((random() * 180.0) / RAND_MAX) - 90.0;
	double lon = ((random() * 360.0 * 10) / RAND_MAX) - (360.0 * 5);
	check(lat, lon,
	      round(lat * (3600.0 * 16)) / (3600.0 * 16),
	      round(fmod(lon + 3600.0, 360.0) * (3600.0 * 16)) / (3600.0 * 16)
	     );
    }
}
