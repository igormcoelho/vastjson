
all: check

check:
	cd tests && make

format:
	# clang-format depends on .clang-format file which is YAML, or passing manually with -style option
	clang-format -i -style='{ BasedOnStyle : Mozilla, ColumnLimit : 0, IndentWidth: 3, AccessModifierOffset: -3}' src/vastjson/VastJSON.hpp

lib:
	g++ -std=c++14 -pedantic -Wall -Ofast -Isrc/ -Ilibs/ --shared src/vastjson/vastjson_lib.cpp -o src/vastjson_py/cpp-build/libvastjson.so -fPIC
