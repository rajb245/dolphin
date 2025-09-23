add_executable(dolphin-tool
  ToolHeadlessPlatform.cpp
  ExtractCommand.cpp
  ExtractCommand.h
  ConvertCommand.cpp
  ConvertCommand.h
  VerifyCommand.cpp
  VerifyCommand.h
  HeaderCommand.cpp
  HeaderCommand.h
  ToolMain.cpp
)

set_target_properties(dolphin-tool PROPERTIES OUTPUT_NAME dolphin-tool)

if(DOLPHIN_TOOL_STANDALONE)
  target_compile_definitions(dolphin-tool PRIVATE DIK_STANDALONE=1)
  target_sources(dolphin-tool PRIVATE StandaloneUICommon.cpp)
  target_link_libraries(dolphin-tool
  PRIVATE
    DiscImageKit
    cpp-optparse
    fmt::fmt
  )
else()
  target_link_libraries(dolphin-tool
  PRIVATE
    discio
    uicommon
    cpp-optparse
    fmt::fmt
  )
endif()

if(MSVC)
  # Add precompiled header
  target_link_libraries(dolphin-tool PRIVATE use_pch)
endif()

set(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} dolphin-tool)
install(TARGETS dolphin-tool RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(ENABLE_TESTS)
  add_subdirectory(tests)
endif()
