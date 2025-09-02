PROJECT_CXX	?= clang++
CXXFLAGS 	?= -fuse-ld=lld -flto -fvisibility=default -fsanitize=cfi-vcall -fsanitize-cfi-cross-dso -O0 -g
all: main libBlurFilter.so libShapFilter.so
	

libBlurFilter.so: BlurFilter.cpp
	${PROJECT_CXX} ${CXXFLAGS} -fPIC -shared -o $@ $< `pkg-config --cflags --libs opencv4`
 

libShapFilter.so: ShapFilter.cpp
	${PROJECT_CXX} ${CXXFLAGS} -fPIC -shared -o $@ $< `pkg-config --cflags --libs opencv4`


main: main.cpp
	${PROJECT_CXX} ${CXXFLAGS} -no-pie -o $@ $< `pkg-config --cflags --libs opencv4` -ldl


run: main libBlurFilter.so libShapFilter.so
	./main 10


clean:
	rm -rf main libBlurFilter.so libShapFilter.so output.jpg
