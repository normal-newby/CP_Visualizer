# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\sql_practice_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\sql_practice_autogen.dir\\ParseCache.txt"
  "sql_practice_autogen"
  )
endif()
