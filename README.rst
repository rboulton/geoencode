GeoEncode
=========

This repository contains code for converting geospatial coordinates (ie,
latitude-longitude coordinates) to and from a compact byte encoding.  The
code has been written for use by Xapian (http://xapian.org), but is being
kept as a separate project since it may be useful for other projects.

The encoding has some interesting features:

 - 6 bytes can encode a coordinate to an accuracy of 1/16th of an arcsecond
   (which translates to a maximum of slightly under 2 meters on the surface
   of the earth).

 - truncating the encoding to lengths of 5, 4, 3 or 2 bytes produces
   bounding boxes which include the box encoded by the 6 byte
   representation, at successively lower accuracy.  These truncated
   representations can be used for indexing.

 - tests on a several-years-old desktop show that over a million
   encode-decode cycles can be performed in a second.  Moreover, decoding
   is considerably faster than encoding.
