find_package(Python3 COMPONENTS Interpreter)

if(NOT Python3_Interpreter_FOUND)
  message(WARNING "Skipping DolphinTool golden tests: Python 3 interpreter not available.")
  return()
endif()

set(DOLPHIN_TOOL_TEST_ARCHIVE "$ENV{DOLPHIN_TOOL_TEST_ARCHIVE}")
if(DOLPHIN_TOOL_TEST_ARCHIVE STREQUAL "")
  set(DOLPHIN_TOOL_TEST_ARCHIVE "/mnt/Data_Storage/ROMS/Wii/Balloon Pop (USA).zip")
endif()

if(NOT EXISTS "${DOLPHIN_TOOL_TEST_ARCHIVE}")
  message(WARNING "Skipping DolphinTool golden tests: archive '${DOLPHIN_TOOL_TEST_ARCHIVE}' not found. Set DOLPHIN_TOOL_TEST_ARCHIVE to override.")
  return()
endif()

set(TEST_WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/artifacts")
set(TEST_USER_DIR "${CMAKE_CURRENT_BINARY_DIR}/user")
set(TEST_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/run_dolphin_tool_tests.py")
set(EXPECTED_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/expected_header.txt")
set(EXPECTED_VERIFY "${CMAKE_CURRENT_SOURCE_DIR}/expected_verify.txt")

add_test(
  NAME dolphin_tool_header
  COMMAND ${Python3_EXECUTABLE} ${TEST_SCRIPT}
          --dolphin-tool $<TARGET_FILE:dolphin-tool>
          --archive ${DOLPHIN_TOOL_TEST_ARCHIVE}
          --work-dir ${TEST_WORK_DIR}
          --user-dir ${TEST_USER_DIR}
          --expected ${EXPECTED_HEADER}
          header
)

add_test(
  NAME dolphin_tool_verify
  COMMAND ${Python3_EXECUTABLE} ${TEST_SCRIPT}
          --dolphin-tool $<TARGET_FILE:dolphin-tool>
          --archive ${DOLPHIN_TOOL_TEST_ARCHIVE}
          --work-dir ${TEST_WORK_DIR}
          --user-dir ${TEST_USER_DIR}
          --expected ${EXPECTED_VERIFY}
          verify
)

add_test(
  NAME dolphin_tool_convert_roundtrip
  COMMAND ${Python3_EXECUTABLE} ${TEST_SCRIPT}
          --dolphin-tool $<TARGET_FILE:dolphin-tool>
          --archive ${DOLPHIN_TOOL_TEST_ARCHIVE}
          --work-dir ${TEST_WORK_DIR}
          --user-dir ${TEST_USER_DIR}
          --expected ${EXPECTED_VERIFY}
          convert-roundtrip
)
