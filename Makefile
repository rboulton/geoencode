geoencode_test: geoencode.cc geoencode.h geoencode_test.cc
	$(CXX) -I . geoencode.cc geoencode_test.cc -o $@

check: geoencode_test
	./geoencode_test

docs: docs/always
docs/always:
	doxygen geoencode.doxygen
