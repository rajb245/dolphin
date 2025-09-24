add_library(DiscImageKit STATIC
  src/fs.cpp
  src/Core/IOS/ES/Formats.cpp
  src/Common/CPUDetect.cpp
  src/Common/Crypto.cpp
  src/Common/ColorUtil.cpp
  src/Common/FileUtil.cpp
  src/Common/Hash.cpp
  src/Common/IOFile.cpp
  src/Common/Log.cpp
  src/Common/MsgHandler.cpp
  src/Common/NandPaths.cpp
  src/Common/StringUtil.cpp
  src/DiscIO/Blob.cpp
  src/DiscIO/CISOBlob.cpp
  src/DiscIO/CompressedBlob.cpp
  src/DiscIO/DirectoryBlob.cpp
  src/DiscIO/DiscExtractor.cpp
  src/DiscIO/DiscScrubber.cpp
  src/DiscIO/DiscUtils.cpp
  src/DiscIO/Enums.cpp
  src/DiscIO/FileBlob.cpp
  src/DiscIO/FileSystemGCWii.cpp
  src/DiscIO/Filesystem.cpp
  src/DiscIO/GameModDescriptor.cpp
  src/DiscIO/LaggedFibonacciGenerator.cpp
  src/DiscIO/NANDImporter.cpp
  src/DiscIO/NFSBlob.cpp
  src/DiscIO/RiivolutionParser.cpp
  src/DiscIO/RiivolutionPatcher.cpp
  src/DiscIO/ScrubbedBlob.cpp
  src/DiscIO/SplitFileBlob.cpp
  src/DiscIO/TGCBlob.cpp
  src/DiscIO/Volume.cpp
  src/DiscIO/VolumeDisc.cpp
  src/DiscIO/VolumeFileBlobReader.cpp
  src/DiscIO/VolumeGC.cpp
  src/DiscIO/VolumeVerifier.cpp
  src/DiscIO/VolumeWad.cpp
  src/DiscIO/VolumeWii.cpp
  src/DiscIO/WbfsBlob.cpp
  src/DiscIO/WIABlob.cpp
  src/DiscIO/WIACompression.cpp
  src/DiscIO/WiiEncryptionCache.cpp
  src/DiscIO/WiiSaveBanner.cpp
)

set_target_properties(DiscImageKit PROPERTIES
  OUTPUT_NAME "discimagekit"
)

target_include_directories(DiscImageKit
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../Core
)

target_include_directories(DiscImageKit
  BEFORE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_SOURCE_DIR}/Externals/mbedtls/include
)

target_compile_features(DiscImageKit PUBLIC cxx_std_17)

target_compile_definitions(DiscImageKit PUBLIC DIK_STANDALONE=1)

# TODO: Trim dependencies in later phases.
target_link_libraries(DiscImageKit
  PUBLIC
    fmt::fmt
    zstd::zstd
    ZLIB::ZLIB
    BZip2::BZip2
    LibLZMA::LibLZMA
    minizip-ng::minizip-ng
    pugixml
    mbedcrypto
    mbedtls
    mbedx509
)
