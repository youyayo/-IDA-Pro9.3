all: libidahook.so

libidahook.so: hook.cpp
	g++ -O2 -fPIC -shared hook.cpp -o libidahook.so -ldl

clean:
	rm -f libidahook.so test_hook
