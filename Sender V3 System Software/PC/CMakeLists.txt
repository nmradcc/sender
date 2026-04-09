cmake_minimum_required(VERSION 3.20)

project(SenderV3System LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SRC_DIR ${ROOT_DIR})
set(SEND_DIR ${SRC_DIR}/SRC/SEND)
set(INC_DIR ${SRC_DIR}/INCLUDE)
set(LIB_DIR ${SRC_DIR}/LIB)

add_library(nmra_lib
  ${LIB_DIR}/BITS.CPP
  ${LIB_DIR}/ZLOG.CPP
  ${LIB_DIR}/cksum.cpp
)

target_include_directories(nmra_lib PUBLIC ${INC_DIR})

if (MSVC)
  target_compile_definitions(nmra_lib PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
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

option(BUILD_TEST_TOOLS "Build test utilities in PC/LIB" OFF)

if (BUILD_TEST_TOOLS)
  add_executable(cktest ${LIB_DIR}/cktest.cpp)
  target_include_directories(cktest PRIVATE ${INC_DIR})
  target_link_libraries(cktest PRIVATE nmra_lib)

  add_executable(zl_tst ${LIB_DIR}/ZL_TST.CPP)
  target_include_directories(zl_tst PRIVATE ${INC_DIR})
  target_link_libraries(zl_tst PRIVATE nmra_lib)

  add_executable(bit_test ${LIB_DIR}/BIT_TEST.CPP)
  target_include_directories(bit_test PRIVATE ${INC_DIR})
  target_link_libraries(bit_test PRIVATE nmra_lib)

  if (MSVC)
    target_compile_definitions(cktest PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
    target_compile_definitions(zl_tst PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
    target_compile_definitions(bit_test PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
  endif()
endif()
