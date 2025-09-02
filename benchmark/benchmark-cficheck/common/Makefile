PROJECT_CXX	?= clang++
CXXFLAGS 	?= -fuse-ld=lld -flto -fvisibility=default -fsanitize=cfi-vcall -fsanitize-cfi-cross-dso -O0 -g


all: main libOpn.so


libOpn.so: Operation.cpp
	${PROJECT_CXX} ${CXXFLAGS} -fPIC -shared -o $@ $<


main: main.cpp
	${PROJECT_CXX} ${CXXFLAGS} -no-pie -o $@ $< -ldl


run: main libOpn.so
	./main 1000000


clean:
	rm -rf main libOpn.so
