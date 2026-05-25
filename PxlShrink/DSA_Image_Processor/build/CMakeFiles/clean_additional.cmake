# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\DSA_Image_Project_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\DSA_Image_Project_autogen.dir\\ParseCache.txt"
  "DSA_Image_Project_autogen"
  )
endif()
