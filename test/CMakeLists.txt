project(clapptest)

add_executable(${PROJECT_NAME}
    test_main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE
    clapp)

add_test(NAME "Tests" COMMAND ${PROJECT_NAME})