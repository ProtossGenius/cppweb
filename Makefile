##Tail
prebuild:
	python3 ./format
	smdcatalog	
	smlines -ext ".cpp,.h"

debug:

qrun:
	rm -rf build && mkdir build
	cd build && cmake .. && make -j8 && sudo make install && cd -
	cd tests/demo && make qrun
test:
	rm -rf build && mkdir build
	cd build && cmake .. && make -j8 && sudo make install && cd -
	cd tests/demo && make build 
install:

clean:
