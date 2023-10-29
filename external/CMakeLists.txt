cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

include(FetchContent)

if (NOT TARGET crdt_version3)
	FetchContent_Declare(crdt_version3
		GIT_REPOSITORY https://github.com/Green-Sky/green_crdt.git
		GIT_TAG master
	)
	FetchContent_MakeAvailable(crdt_version3)
endif()

