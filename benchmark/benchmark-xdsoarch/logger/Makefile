PROJECT_CXX	?= clang++
CXXFLAGS 	?= -fuse-ld=lld -flto -fvisibility=default -fsanitize=cfi-vcall -fsanitize-cfi-cross-dso -O0 -g
all: main libFileLogger.so libConsLogger.so


libFileLogger.so: FileLogger.cpp
	${PROJECT_CXX} ${CXXFLAGS} -fPIC -shared -o $@ $<


libConsLogger.so: ConsLogger.cpp
	${PROJECT_CXX} ${CXXFLAGS} -fPIC -shared -o $@ $<

	
main: main.cpp
	${PROJECT_CXX} ${CXXFLAGS} -no-pie -o $@ $< -ldl


run: main libFileLogger.so libConsLogger.so
	./main 10


clean:
	rm -rf main libFileLogger.so libConsLogger.so