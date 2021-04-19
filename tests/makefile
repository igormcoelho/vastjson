all: test
	./build/app_test -d yes

lib:	
	g++ --shared csBigIntegerpp/src/csBigIntegerLib.cpp csBigIntegerpp/src/BigInteger.cpp -lgmp -lgmpxx -o csbiginteger/csbiginteger.so -fPIC

test:
	mkdir -p build/
	g++ --std=c++17 -fsanitize=address -g3 -I../src -I../libs all_tests.cpp -o build/app_test

clean:
	mkdir -p build
	rm -f *.gcda
	rm -f *.gcno
	rm -f build/*_test
