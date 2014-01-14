EM=emcc
EMCXX=em++
EMFLAGS=-O2 --js-opts 0 --closure 1
EMCXXFLAGS=-O2 -Wall -Wextra -Werror
EMMAKE=emmake
EMCONFIGURE=emconfigure

space=$(eval) $(eval)
comma=,

.PHONY: all

all: onig.js

src/onigjs.o: src/onigjs.cpp vendor/include/oniguruma.h
	${EMCXX} ${EMCXXFLAGS} -Ivendor/include --bind -c -o $@ $<

onig.js: src/onigjs.o vendor/lib/libonig.a src/libonig_export.js
	EMCC_CLOSURE_ARGS="--output_wrapper '(function(){%output%})()'" \
	  ${EM} ${EMFLAGS} --bind --post-js src/libonig_export.js -o $@ \
	  src/onigjs.o vendor/lib/libonig.a

vendor/lib/libonig.a vendor/include/oniguruma.h:
	libonig

.PHONY: libonig
libonig:
	cd vendor/libonig; \
	  ${EMCONFIGURE} ./configure --prefix=/src/vendor --disable-shared && \
	  ${EMMAKE} ${MAKE} install

.PHONY: clean libonig-clean
clean: libonig-clean
	rm -rf vendor/{bin,include,lib} src/*.o libonig.js

libonig-clean:
	cd vendor/libonig; \
	  ${EMMAKE} ${MAKE} clean