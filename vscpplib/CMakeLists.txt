add_library(vscpplib STATIC vscpplib.cpp vscpplib.hpp)
set_target_properties(vscpplib PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
)

target_link_libraries(vscpplib vsclib)