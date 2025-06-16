cmake_minimum_required(VERSION 3.10)
project(Security4-dump1090-Phase2 C CXX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")

# 1. 헤더 경로 추가 (TLSsample 폴더 등)
include_directories(
    ${CMAKE_SOURCE_DIR}/TLSsample
    ${CMAKE_SOURCE_DIR}
)

# 2. pkg-config로 rtlsdr 관련 플래그 가져오기
find_package(PkgConfig REQUIRED)
pkg_check_modules(RTLSDR REQUIRED librtlsdr)
include_directories(${RTLSDR_INCLUDE_DIRS})

# 3. 소스코드 OBJECT 라이브러리화
add_library(dump1090_obj OBJECT dump1090.c)
add_library(sqlog_obj OBJECT sqlog.c)
add_library(anet_obj OBJECT anet.c)
add_library(tserver_obj OBJECT TLSsample/tserver.c)
add_library(sbs_network_reader_obj OBJECT sbs_network_reader.c)
# 필요시 추가 소스도 같은 방식으로 등록


target_compile_definitions(dump1090_obj PUBLIC UNIT_TEST LG_SECURITY_USE_EXT_SBS_THREAD)
target_compile_definitions(tserver_obj PUBLIC UNIT_TEST)
target_compile_definitions(sbs_network_reader_obj PUBLIC LG_SECURITY_USE_EXT_SBS_THREAD)
# 4. 테스트 디렉토리 포함
add_subdirectory(tests)