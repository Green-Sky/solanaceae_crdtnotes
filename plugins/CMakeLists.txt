cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

add_library(plugin_crdtnotes SHARED
	./plugin_crdtnotes.cpp
)
target_compile_features(plugin_crdtnotes PUBLIC cxx_std_17)
target_link_libraries(plugin_crdtnotes PUBLIC
	solanaceae_crdtnotes
	solanaceae_plugin
)

########################################

add_library(plugin_crdtnotes_imgui SHARED
	./plugin_crdtnotes_imgui.cpp
)
target_compile_features(plugin_crdtnotes_imgui PUBLIC cxx_std_17)
target_link_libraries(plugin_crdtnotes_imgui PUBLIC
	solanaceae_crdtnotes_imgui
	solanaceae_plugin
)

########################################

if (TARGET solanaceae_crdtnotes_toxsync)
	add_library(plugin_crdtnotes_toxsync SHARED
		./plugin_crdtnotes_toxsync.cpp
	)
	target_compile_features(plugin_crdtnotes_toxsync PUBLIC cxx_std_17)
	target_link_libraries(plugin_crdtnotes_toxsync PUBLIC
		solanaceae_crdtnotes_toxsync
		solanaceae_plugin
	)
endif()

########################################
