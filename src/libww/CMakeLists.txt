project(libww)
add_library(libww codebook.cpp codebook.h crc.c crc.h wwriff.cpp wwriff.h)
target_link_libraries(libww LINK_PUBLIC)