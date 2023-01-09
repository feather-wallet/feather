find_path(QRENCODE_INCLUDE_DIR qrencode.h)
message(STATUS "QRENCODE PATH ${QRENCODE_INCLUDE_DIR}")

find_library(QRENCODE_LIBRARY qrencode)
message(STATUS "QRENCODE LIBRARY ${QRENCODE_LIBRARY}")

mark_as_advanced(QRENCODE_LIBRARY QRENCODE_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QREncode DEFAULT_MSG QRENCODE_LIBRARY QRENCODE_INCLUDE_DIR)
