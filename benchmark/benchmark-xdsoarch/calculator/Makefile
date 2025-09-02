PROJECT_CXX	?= clang++
CXXFLAGS 	?= -fuse-ld=lld -flto -fvisibility=default -fsanitize=cfi-vcall -fsanitize-cfi-cross-dso -O0 -g


all: main libAdd.so libSub.so


libAdd.so: add.cpp
	${PROJECT_CXX} ${CXXFLAGS} -fPIC -shared -o $@ $<


libSub.so: sub.cpp
	${PROJECT_CXX} ${CXXFLAGS} -fPIC -shared -o $@ $<


main: main.cpp
	${PROJECT_CXX} ${CXXFLAGS} -no-pie -o $@ $< -ldl


run: main libAdd.so libSub.so
	./main 10


clean:
	rm -rf main libAdd.so libSub.so
