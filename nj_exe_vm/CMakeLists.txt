file(GLOB SOURCE_FILES *.c)
file(GLOB HEADER_FILES *.h)
add_library(nj_exe_vm ${SOURCE_FILES} ${HEADER_FILES})
