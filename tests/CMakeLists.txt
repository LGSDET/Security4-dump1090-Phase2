# 테스트 파일 목록 (필요시 여기에 추가)
set(TEST_SOURCES
    test_dump1090.cpp
    test_sqlog.cpp
    test_anet.cpp
    test_tserver.cpp
    test_sbs_network_reader.cpp
)

foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})

    # target_link_libraries(${test_name}
    #     dump1090_obj
    #     sqlog_obj
    #     anet_obj
    #     tserver_obj
    #     sbs_network_reader_obj
    #     ${RTLSDR_LIBRARIES}
    #     gtest gtest_main pthread ssl crypto m

    if(${test_name} STREQUAL "test_sbs_network_reader")
        target_link_libraries(${test_name}
            sbs_network_reader_obj
            dump1090_obj
            sqlog_obj
            anet_obj
            tserver_obj
            ${RTLSDR_LIBRARIES}
            gtest gtest_main pthread ssl crypto m
        )
    else()
        target_link_libraries(${test_name}
            dump1090_obj
            sqlog_obj
            anet_obj
            tserver_obj
            ${RTLSDR_LIBRARIES}
            gtest gtest_main pthread ssl crypto m
        )
    endif()

    # include 경로 (루트에서 이미 추가되어 있으면 생략 가능)
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/TLSsample
        ${CMAKE_SOURCE_DIR}
        ${RTLSDR_INCLUDE_DIRS}
    )

endforeach()