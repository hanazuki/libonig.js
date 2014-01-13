DOCKER=docker
DOCKER_FLAGS=
DOCKER_IMAGE=hanazuki/ubuntu-emscripten

.PHONY: all clean

all:
	${DOCKER} ${DOCKER_FLAGS} run -rm -v ${PWD}:/src ${DOCKER_IMAGE} \
	  make -f Emscripten.mk all

clean:
	${DOCKER} ${DOCKER_FLAGS} run -rm -v ${PWD}:/src ${DOCKER_IMAGE} \
	  make -f Emscripten.mk clean
