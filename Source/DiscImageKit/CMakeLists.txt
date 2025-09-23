add_library(DiscImageKit STATIC
  src/fs.cpp
  ../Core/DiscIO/Blob.cpp
  ../Core/DiscIO/CISOBlob.cpp
  ../Core/DiscIO/CompressedBlob.cpp
  ../Core/DiscIO/DirectoryBlob.cpp
  ../Core/DiscIO/DiscExtractor.cpp
  ../Core/DiscIO/DiscScrubber.cpp
  ../Core/DiscIO/DiscUtils.cpp
  ../Core/DiscIO/Enums.cpp
  ../Core/DiscIO/FileBlob.cpp
  ../Core/DiscIO/FileSystemGCWii.cpp
  ../Core/DiscIO/Filesystem.cpp
  ../Core/DiscIO/GameModDescriptor.cpp
  ../Core/DiscIO/LaggedFibonacciGenerator.cpp
  ../Core/DiscIO/NANDImporter.cpp
  ../Core/DiscIO/NFSBlob.cpp
  ../Core/DiscIO/RiivolutionParser.cpp
  ../Core/DiscIO/RiivolutionPatcher.cpp
  ../Core/DiscIO/ScrubbedBlob.cpp
  ../Core/DiscIO/SplitFileBlob.cpp
  ../Core/DiscIO/TGCBlob.cpp
  ../Core/DiscIO/Volume.cpp
  ../Core/DiscIO/VolumeDisc.cpp
  ../Core/DiscIO/VolumeFileBlobReader.cpp
  ../Core/DiscIO/VolumeGC.cpp
  ../Core/DiscIO/VolumeVerifier.cpp
  ../Core/DiscIO/VolumeWad.cpp
  ../Core/DiscIO/VolumeWii.cpp
  ../Core/DiscIO/WbfsBlob.cpp
  ../Core/DiscIO/WIABlob.cpp
  ../Core/DiscIO/WIACompression.cpp
  ../Core/DiscIO/WiiEncryptionCache.cpp
  ../Core/DiscIO/WiiSaveBanner.cpp
)

set_target_properties(DiscImageKit PROPERTIES
  OUTPUT_NAME "discimagekit"
)

target_include_directories(DiscImageKit
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Core
)

target_compile_features(DiscImageKit PUBLIC cxx_std_17)

# TODO: Trim dependencies in later phases.
target_link_libraries(DiscImageKit
  PUBLIC
    core
    fmt::fmt
    zstd::zstd
    ZLIB::ZLIB
    BZip2::BZip2
    LibLZMA::LibLZMA
    minizip-ng::minizip-ng
    pugixml
)
