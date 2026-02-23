# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/eventcam_GUI_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/eventcam_GUI_autogen.dir/ParseCache.txt"
  "eventcam_GUI_autogen"
  )
endif()
