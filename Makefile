build-debug:
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make && cd .. && ./build/vulkanDemo

build-release:
	cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make && cd

run:
	make build-release && ./build/vulkanDemo