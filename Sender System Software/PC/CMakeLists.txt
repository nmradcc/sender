cmake_minimum_required(VERSION 3.20)

project(SenderV4System LANGUAGES C CXX)

option(BUILD_TEST_TOOLS "Build and register unit test tools" OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SRC_DIR ${ROOT_DIR})
set(SEND_DIR ${SRC_DIR}/SRC/SEND)
set(INC_DIR ${SRC_DIR}/INCLUDE)
set(LIB_DIR ${SRC_DIR}/LIB)

# PC software version. This is the single definition of the version; it is
# passed to the compiler below and is required by SRC/SEND/SEND.H.
# PREFIX is the release level: D-debug, X-experimental, B-beta, R-released.
set(PC_VERSION_PREFIX B)
set(PC_VERSION_MAJOR 7)
set(PC_VERSION_MINOR 0)
set(PC_VERSION_PATCH 0)
set(PC_VERSION_BUILD 0)

set(_pc_ver_rel_D DEB)
set(_pc_ver_rel_X EXP)
set(_pc_ver_rel_B BETA)
set(_pc_ver_rel_R REL)
if (NOT DEFINED _pc_ver_rel_${PC_VERSION_PREFIX})
  message(FATAL_ERROR "PC_VERSION_PREFIX must be one of D, X, B or R (got '${PC_VERSION_PREFIX}')")
endif()
message("PC software version: ${PC_VERSION_PREFIX}.${PC_VERSION_MAJOR}.${PC_VERSION_MINOR}.${PC_VERSION_PATCH}.${PC_VERSION_BUILD}")

add_compile_definitions(
  PC_VER_REL=VER_${_pc_ver_rel_${PC_VERSION_PREFIX}}
  PC_VER_MAJOR=${PC_VERSION_MAJOR}u
  PC_VER_MINOR=${PC_VERSION_MINOR}u
  PC_VER_PATCH=${PC_VERSION_PATCH}u
  PC_VER_BUILD=${PC_VERSION_BUILD}u
)

add_library(nmra_lib
  ${LIB_DIR}/BITS.CPP
  ${LIB_DIR}/ZLOG.CPP
  ${LIB_DIR}/cksum.cpp
)

target_include_directories(nmra_lib PUBLIC ${INC_DIR})

if (MSVC)
  target_compile_definitions(nmra_lib PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
endif()

if (BUILD_TEST_TOOLS)
  enable_testing()

  add_executable(bit_test
    ${LIB_DIR}/BIT_TEST.CPP
  )
  target_link_libraries(bit_test PRIVATE nmra_lib)
  add_test(NAME bit_test COMMAND bit_test)

  add_executable(zlog_test
    ${LIB_DIR}/ZLOG_TEST.CPP
  )
  target_link_libraries(zlog_test PRIVATE nmra_lib)
  add_test(NAME zlog_test COMMAND zlog_test)

  set_tests_properties(bit_test zlog_test PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()

add_executable(send
  ${SEND_DIR}/ARGS.CPP
  ${SEND_DIR}/DEC_TST.CPP
  ${SEND_DIR}/SEND.CPP
  ${SEND_DIR}/SENDER_HW_MCU_USB.CPP
  ${SEND_DIR}/SEND_REG.CPP
  ${SEND_DIR}/SR_CORE.CPP
)

target_include_directories(send PRIVATE ${INC_DIR} ${SEND_DIR})
target_link_libraries(send PRIVATE nmra_lib)

if (MSVC)
  target_compile_definitions(send PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
endif()

add_executable(mcu_smoke_test
  ${SEND_DIR}/mcu_smoke_test.cpp
  ${SEND_DIR}/SENDER_HW_MCU_USB.CPP
)
target_include_directories(mcu_smoke_test PRIVATE ${INC_DIR} ${SEND_DIR})
target_link_libraries(mcu_smoke_test PRIVATE nmra_lib)
if (MSVC)
  target_compile_definitions(mcu_smoke_test PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
endif()

add_executable(migrateINI
  ${SEND_DIR}/migrateINI.cpp
)
if (MSVC)
  target_compile_definitions(migrateINI PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
endif()

if (WIN32)
  add_executable(sendgui WIN32
    ${SEND_DIR}/sendgui.cpp
  )
  target_include_directories(sendgui PRIVATE ${INC_DIR})
  target_link_libraries(sendgui PRIVATE comdlg32 comctl32)
  if (MSVC)
    target_compile_definitions(sendgui PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
  endif()
endif()
