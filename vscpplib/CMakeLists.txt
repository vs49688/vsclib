add_library(vscpplib STATIC vscpplib.cpp include/vscpplib.hpp)
set_target_properties(vscpplib PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
)

target_include_directories(vscpplib PUBLIC include)
target_link_libraries(vscpplib vsclib)