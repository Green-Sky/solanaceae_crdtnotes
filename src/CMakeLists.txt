cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

add_library(solanaceae_crdtnotes
	./solanaceae/crdtnotes/crdtnotes.hpp
	./solanaceae/crdtnotes/crdtnotes.cpp
)
target_include_directories(solanaceae_crdtnotes PUBLIC .)
target_compile_features(solanaceae_crdtnotes PUBLIC cxx_std_17)
target_link_libraries(solanaceae_crdtnotes PUBLIC
	crdt_version3
	#solanaceae_util
)

########################################

add_library(solanaceae_crdtnotes_imgui
	./solanaceae/crdtnotes_imgui/crdtnotes_imgui.hpp
	./solanaceae/crdtnotes_imgui/crdtnotes_imgui.cpp
)
target_include_directories(solanaceae_crdtnotes_imgui PUBLIC .)
target_compile_features(solanaceae_crdtnotes_imgui PUBLIC cxx_std_17)
target_link_libraries(solanaceae_crdtnotes_imgui PUBLIC
	solanaceae_crdtnotes
	#imgui_interface
	imgui
	#solanaceae_util
	solanaceae_contact
)

########################################
