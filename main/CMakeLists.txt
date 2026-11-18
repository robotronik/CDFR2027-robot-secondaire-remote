set(LIB ${CMAKE_CURRENT_LIST_DIR}/../../librairie-commune)
# Same version macros as the asserv platformio.ini, so both sides hash the same commit.
execute_process(COMMAND git -C ${LIB} rev-parse --short HEAD
                OUTPUT_VARIABLE SHA OUTPUT_STRIP_TRAILING_WHITESPACE)
string(SUBSTRING "${SHA}" 0 2 SHA1)
string(SUBSTRING "${SHA}" 2 2 SHA2)
string(SUBSTRING "${SHA}" 4 2 SHA3)
string(SUBSTRING "${SHA}" 6 2 SHA4)

idf_component_register(
    SRCS main.cpp ../../librairie-commune/master/src/asservissement_interface.cpp
    INCLUDE_DIRS . ../../librairie-commune/master/include ../../librairie-commune/common/include
    REQUIRES esp_driver_i2c esp_wifi esp_netif esp_event nvs_flash esp_http_server esp_timer
    EMBED_TXTFILES index.html)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    GIT_COMMIT_SHA_PART1=0x${SHA1} GIT_COMMIT_SHA_PART2=0x${SHA2}
    GIT_COMMIT_SHA_PART3=0x${SHA3} GIT_COMMIT_SHA_PART4=0x${SHA4})
