AUX_SOURCE_DIRECTORY(. SOURCES)
AUX_SOURCE_DIRECTORY(./algebra SOURCES)
AUX_SOURCE_DIRECTORY(./common SOURCES)
AUX_SOURCE_DIRECTORY(./combinatorics SOURCES)
AUX_SOURCE_DIRECTORY(./combinatorics/exact SOURCES)

SET(CMAKE_CXX_FLAGS "-fpermissive -frounding-math")
add_library(cvt SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(cvt
    gmp
)

install(DIRECTORY ./
  DESTINATION ${CMAKE_SOURCE_DIR}/include/CVT
  FILES_MATCHING PATTERN "*.h" REGEX ".svn.*" EXCLUDE)
