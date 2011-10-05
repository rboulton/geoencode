geoencode_test:
	$(CXX) -I . geoencode.cc geoencode_test.cc -o $@

check: geoencode_test
	./geoencode_test
