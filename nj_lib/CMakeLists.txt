file(GLOB SOURCE_FILES *.c)
file(GLOB HEADER_FILES *.h)
add_library(nj_lib ${SOURCE_FILES} ${HEADER_FILES})
