# bitmap
add_library(bitmap INTERFACE)
target_include_directories(bitmap INTERFACE bitmap/)

# libbmpread
add_library(libbmpread STATIC libbmpread/libbmpread/bmpread.c)
set_target_properties(libbmpread PROPERTIES LINKER_LANGUAGE C)
target_include_directories(libbmpread INTERFACE libbmpread/)

# threadpool
add_library(threadpool INTERFACE)
target_include_directories(threadpool INTERFACE threadpool/)