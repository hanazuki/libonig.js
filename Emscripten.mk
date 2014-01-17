EM=emcc
EMCXX=em++
EMFLAGS=-O2 --js-opts 0 --closure 1
EMCXXFLAGS=-O2 -Wall -Wextra -Werror
EMMAKE=emmake
EMCONFIGURE=emconfigure

space=$(eval) $(eval)
comma=,

.PHONY: all
all: libonig.js

src/onigjs.o: src/onigjs.cpp vendor/include/oniguruma.h
	${EMCXX} ${EMCXXFLAGS} -Ivendor/include --bind -c -o $@ $<

libonig_.js: src/onigjs.o vendor/lib/libonig.a
	EMCC_CLOSURE_ARGS="--output_wrapper 'var libonig={};(function(Module){%output%})(libonig)'" \
	  ${EM} ${EMFLAGS} --bind -o $@ $^

libonig.js: libonig_license.js libonig_.js
	cat $^ > $@

vendor/lib/libonig.a vendor/include/oniguruma.h:
	cd vendor/libonig; \
	  ${EMCONFIGURE} ./configure --prefix=${PWD}/vendor --disable-shared && \
	  ${EMMAKE} ${MAKE} install

libonig_license.js: LICENSE vendor/libonig/COPYING
	( echo '/**'; \
	  ( echo '@license libonig.js copyright notice'; echo; \
	    cat LICENSE; echo; echo; \
	    sed 's|^.[*].\{0,1\}||' vendor/libonig/COPYING) | sed 's|^| * |; s| $$||'; \
	  echo ' */') > $@

.PHONY: clean libonig-clean
clean: libonig-clean
	rm -rf vendor/{bin,include,lib} src/*.o libonig.js

libonig-clean:
	cd vendor/libonig; \
	  ${EMMAKE} ${MAKE} clean