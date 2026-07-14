set(GAMELINK_EXAMPLE_NAMES
    authenticate
    voting
    state
    broadcast
    transaction
)

set(GAMELINK_EXAMPLE_DIRECTORIES
    01-authenticate
    02-voting
    03-state
    04-broadcast
    05-transaction
)

list(LENGTH GAMELINK_EXAMPLE_NAMES GAMELINK_EXAMPLE_COUNT)
math(EXPR GAMELINK_EXAMPLE_LAST "${GAMELINK_EXAMPLE_COUNT} - 1")

foreach(index RANGE ${GAMELINK_EXAMPLE_LAST})
    list(GET GAMELINK_EXAMPLE_NAMES ${index} target)
    list(GET GAMELINK_EXAMPLE_DIRECTORIES ${index} directory)
    add_executable(${target}
        "${directory}/main.cpp"
        "${CMAKE_SOURCE_DIR}/websocket_network/websocket.cpp"
    )
    add_dependencies(${target} amalgam)
    target_include_directories(${target} PRIVATE
        "${AMALGAM_GENERATED_DIR}"
        "${CMAKE_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/third_party"
        "${CMAKE_CURRENT_SOURCE_DIR}/shared"
    )
    target_link_libraries(${target} PRIVATE CURL::libcurl Threads::Threads)
endforeach()

add_custom_target(examples)
add_dependencies(examples ${GAMELINK_EXAMPLE_NAMES})
