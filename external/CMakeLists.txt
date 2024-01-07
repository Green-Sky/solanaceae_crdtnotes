cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

include(FetchContent)

if (NOT TARGET crdt_version3)
	FetchContent_Declare(crdt_version3
		GIT_REPOSITORY https://github.com/Green-Sky/green_crdt.git
		GIT_TAG master
	)
	FetchContent_MakeAvailable(crdt_version3)
endif()

if (NOT TARGET imgui)
	FetchContent_Declare(imgui
		GIT_REPOSITORY https://github.com/ocornut/imgui.git
		GIT_TAG d4ddc46e7
		EXCLUDE_FROM_ALL
	)

	# imgui does not provide a cmake
	#FetchContent_GetProperties(imgui_interface)
	#if(NOT imgui_interface_POPULATED)
		#FetchContent_Populate(imgui_interface)

		#add_library(imgui_interface INTERFACE
			#${imgui_interface_SOURCE_DIR}/imgui.h
			#${imgui_interface_SOURCE_DIR}/misc/cpp/imgui_stdlib.h
		#)
		#target_include_directories(imgui_interface INTERFACE ${imgui_interface_SOURCE_DIR})
	#endif()
	FetchContent_GetProperties(imgui)
	if(NOT imgui_POPULATED)
		FetchContent_Populate(imgui)

		add_library(imgui STATIC
			${imgui_SOURCE_DIR}/imgui.h
			${imgui_SOURCE_DIR}/imgui_internal.h

			${imgui_SOURCE_DIR}/imgui.cpp
			${imgui_SOURCE_DIR}/imgui_demo.cpp
			${imgui_SOURCE_DIR}/imgui_draw.cpp
			${imgui_SOURCE_DIR}/imgui_tables.cpp
			${imgui_SOURCE_DIR}/imgui_widgets.cpp
			${imgui_SOURCE_DIR}/imstb_rectpack.h
			${imgui_SOURCE_DIR}/imstb_textedit.h
			${imgui_SOURCE_DIR}/imstb_truetype.h

			${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h
			${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
		)
		target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
		target_compile_features(imgui PUBLIC cxx_std_11)
	endif()
	#FetchContent_MakeAvailable(imgui)
endif()

# TODO: move entt dep into solanaceae_contact
if (NOT TARGET EnTT::EnTT)
	FetchContent_Declare(EnTT
		GIT_REPOSITORY https://github.com/skypjack/entt.git
		GIT_TAG v3.12.2
	)
	FetchContent_MakeAvailable(EnTT)
endif()

if (NOT TARGET solanaceae_contact)
	FetchContent_Declare(solanaceae_contact
		GIT_REPOSITORY https://github.com/Green-Sky/solanaceae_contact.git
		GIT_TAG master
	)
	FetchContent_MakeAvailable(solanaceae_contact)
endif()

if (NOT TARGET solanaceae_plugin)
	FetchContent_Declare(solanaceae_plugin
		GIT_REPOSITORY https://github.com/Green-Sky/solanaceae_plugin.git
		GIT_TAG master
	)
	FetchContent_MakeAvailable(solanaceae_plugin)
endif()

