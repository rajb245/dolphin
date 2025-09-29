add_library(DiscImageKit STATIC
  src/fs.cpp
  src/Core/IOS/ES/Formats.cpp
  src/common/crypto.cpp
  src/color_utils.cpp
  src/hash_utils.cpp
  src/io_file.cpp
  src/common/msg_handler.cpp
  src/common/nand_paths.cpp
  src/fs_utils.cpp
  src/string_utils.cpp
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

option(DIK_USE_ZLIB "Enable zlib support for GCZ/WIA and CRC helpers" ON)
option(DIK_USE_ZSTD "Enable zstd support for RVZ/WIA compression" ON)
option(DIK_USE_BZIP2 "Enable bzip2 support for WIA compression" ON)
option(DIK_USE_LZMA "Enable lzma support for WIA compression" ON)

set_target_properties(DiscImageKit PROPERTIES
  OUTPUT_NAME "discimagekit"
)

target_include_directories(DiscImageKit
  BEFORE PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(DiscImageKit
  BEFORE PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_SOURCE_DIR}/Externals/mbedtls/include
)

target_compile_features(DiscImageKit PUBLIC cxx_std_17)

target_compile_definitions(DiscImageKit
  PUBLIC
    DIK_STANDALONE=1
    DIK_HAVE_ZLIB=$<BOOL:${DIK_USE_ZLIB}>
    DIK_HAVE_ZSTD=$<BOOL:${DIK_USE_ZSTD}>
    DIK_HAVE_BZIP2=$<BOOL:${DIK_USE_BZIP2}>
    DIK_HAVE_LZMA=$<BOOL:${DIK_USE_LZMA}>
)

set(_dik_public_libs
  fmt::fmt
  minizip-ng::minizip-ng
  pugixml
  mbedcrypto
  mbedtls
  mbedx509
)

set(_dik_private_libs)

if(DIK_USE_ZLIB)
  list(APPEND _dik_private_libs ZLIB::ZLIB)
endif()

if(DIK_USE_ZSTD)
  list(APPEND _dik_private_libs zstd::zstd)
endif()

if(DIK_USE_BZIP2)
  list(APPEND _dik_private_libs BZip2::BZip2)
endif()

if(DIK_USE_LZMA)
  list(APPEND _dik_private_libs LibLZMA::LibLZMA)
endif()

target_link_libraries(DiscImageKit
  PUBLIC
    ${_dik_public_libs}
  PRIVATE
    ${_dik_private_libs}
)
