
<<<<<<< HEAD
INCLUDE_DIRECTORIES( "${PROJECT_SOURCE_DIR}/third_party/Expat/lib")
INCLUDE_DIRECTORIES( "${PROJECT_SOURCE_DIR}/third_party/")
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/third_party/zlib )

SET(SRC
=======
include_directories("${PROJECT_SOURCE_DIR}/third_party/Expat/lib")
include_directories("${PROJECT_SOURCE_DIR}/third_party/")
include_directories("${PROJECT_SOURCE_DIR}/third_party/zlib")

set(SRC
>>>>>>> nrel/develop
  parser.h
  parser.c
  fmumini.c
  fmumini.h
  stack.c
  stack.h
  util.c
  util.h
  xml_parser_cosim.c
  xml_parser_cosim.h
  fmiFunctions.h
  fmiModelTypes.h
)

<<<<<<< HEAD
#ADD_LIBRARY( parser SHARED ${SRC} )

IF (UNIX)
  ADD_DEFINITIONS("-fPIC")
ENDIF()

ADD_EXECUTABLE( parser ${SRC} )

TARGET_LINK_LIBRARIES( parser epexpat miniziplib )

INSTALL( TARGETS parser
=======
#add_library(parser SHARED ${SRC})

if(UNIX)
  add_definitions("-fPIC")
endif()

add_executable(parser ${SRC})

target_link_libraries(parser PRIVATE epexpat miniziplib)

install(TARGETS parser
>>>>>>> nrel/develop
  RUNTIME DESTINATION PreProcess/FMUParser
  LIBRARY DESTINATION ./
  ARCHIVE DESTINATION ./
)

<<<<<<< HEAD
if( UNIX )
set_target_properties(
  parser
  PROPERTIES
  INSTALL_NAME_DIR "@executable_path"
)
=======
if(UNIX)
  set_target_properties(
    parser
    PROPERTIES
    INSTALL_NAME_DIR "@executable_path"
  )
>>>>>>> nrel/develop
endif()

if(BUILD_TESTING)
  # Avoid creating some artifacts (ShadingController/modelDescription.xml and ShadingController/binaries inside "${PROJECT_SOURCE_DIR}/datasets/FMUs/"
  configure_file("${PROJECT_SOURCE_DIR}/datasets/FMUs/ShadingController.fmu" "${CMAKE_CURRENT_BINARY_DIR}/ShadingController.fmu" COPYONLY)
  add_test(NAME "FMUParser"
    COMMAND parser "--printidf" "${CMAKE_CURRENT_BINARY_DIR}/ShadingController.fmu"
  )
endif()
