CONFIG_FLAGS := 
GCC_FLAGS := -std=c++20
BUILD_FLAGS := --parallel 4


# force GCC on windows
ifeq ($(OS), Windows_NT)
	CONFIG_FLAGS += -G "MinGW Makefiles"
endif


all: debug


run: run_d
run_d: 
#ifeq ($(OS), Windows_NT)
	./bin/Debug/CGOLplus
#else
#	sudo ./bin/Debug/CGOLplus
#endif

run_r: 
#ifeq ($(OS), Windows_NT)
	./bin/Release/CGOLplus
#else
#	sudo ./bin/Release/CGOLplus
#endif


debug: CONFIG_FLAGS += -DCMAKE_BUILD_TYPE=Debug
debug: GCC_FLAGS += -g -Wall -Wextra -Wpedantic -march=native
debug: build

release: CONFIG_FLAGS += -DCMAKE_BUILD_TYPE=Release
release: BUILD_FLAGS += --config Release
release: GCC_FLAGS += -O3 -march=x86-64 -DNDEBUG -s -flto -static-libgcc -static-libstdc++
release: build
ifeq ($(OS), Windows_NT)
	cd build  &&  cpack -G ZIP
else 
	cd build  &&  cpack -G TGZ
endif


build: 
ifeq ($(OS), Windows_NT)
	if not exist build mkdir build
	cd build  &&  cmake $(CONFIG_FLAGS) -DCMAKE_CXX_FLAGS="$(GCC_FLAGS)" ..  &&  cmake --build . $(BUILD_FLAGS)
else
	mkdir -p build  &&  cd build  &&  cmake $(CONFIG_FLAGS) -DCMAKE_CXX_FLAGS="$(GCC_FLAGS)" ..  &&  cmake --build . $(BUILD_FLAGS)
endif


package: 
	cd build && cpack


clean: 
ifeq ($(OS), Windows_NT)
	rmdir /s /q build
	rmdir /s /q bin
else
	rm -rf build bin
endif


get_lines: 
ifeq ($(OS), Windows_NT)
	powershell -Command "git ls-files | ForEach-Object { Get-Content \$$_ } | Measure-Object -Line"
else
	git ls-files | xargs wc -l
endif

	
.PHONY: all debug release build clean run_d run_r run get_lines