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
		GIT_TAG bf75bfec48fc00f532af8926130b70c0e26eb099 # v1.92.3
		EXCLUDE_FROM_ALL
		CONFIGURE_COMMAND "" # remove?
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
		FetchContent_MakeAvailable(imgui)

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
		target_compile_definitions(imgui PUBLIC IMGUI_USE_WCHAR32)
	endif()
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

