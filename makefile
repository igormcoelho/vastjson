
all: check

check:
	cd tests && make

lib:
	g++ -std=c++14 -pedantic -Wall -Ofast -Isrc/ -Ilibs/ --shared src/vastjson/vastjson_lib.cpp -o src/vastjson_py/cpp-build/libvastjson.so -fPIC
