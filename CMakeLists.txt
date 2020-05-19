cmake_minimum_required(VERSION 3.10)

# set the project name
project(Tutorial)


find_package(RELIC QUIET)
if(RELIC_FOUND)
	message(STATUS "Found RELIC")
elseif(NOT RELIC_FOUND AND NOT TARGET RELIC::relic)
	message("RELIC was not found: add RELIC subdirectory")
	add_subdirectory(/home/lx/seminar/extern/relic)
endif()

find_package(ABY QUIET)
if(ABY_FOUND)
	message(STATUS "Found ABY")
elseif (NOT ABY_FOUND AND NOT TARGET ABY::aby)
	message("ABY was not found: add ABY subdirectory")
	add_subdirectory(/home/lx/seminar/aby/ABY/)
endif()

find_package(Threads REQUIRED)
find_package(Boost 1.66.0 REQUIRED COMPONENTS thread system)
find_package(ENCRYPTO_utils QUIET)

add_library(RELIC::relic STATIC IMPORTED)
set_target_properties(RELIC::relic PROPERTIES
	IMPORTED_LOCATION "/home/lx/seminar/extern/relic/relic-target/lib/librelic_s.a"
	INTERFACE_INCLUDE_DIRECTORIES "/home/lx/seminar/extern/relic/include;/home/lx/seminar/extern/relic/include/low;/home/lx/seminar/extern/relic/include"
)

# add the executable
add_executable(Tutorial test.cpp)
target_link_libraries(Tutorial RELIC::relic)
target_link_libraries(Tutorial ABY::aby)


