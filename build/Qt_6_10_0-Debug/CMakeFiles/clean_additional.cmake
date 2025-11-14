# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/Spacecraft_Telemetry_Vizualizer_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/Spacecraft_Telemetry_Vizualizer_autogen.dir/ParseCache.txt"
  "Spacecraft_Telemetry_Vizualizer_autogen"
  )
endif()
