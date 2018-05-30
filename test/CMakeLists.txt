set(TEST_NAMES
    lex_boolean_test
    lex_integer_test
    lex_floating_test
    lex_string_test
    lex_datetime_test
    lex_key_comment_test
    parse_boolean_test
    parse_datetime_test
    parse_floating_test
    parse_integer_test
    parse_string_test
    parse_inline_table_test
    parse_key_test
    parse_table_key_test
    get_test
    result_type_test
    parse_file_test
    )

foreach(TEST_NAME ${TEST_NAMES})
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME}
             WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/test")
endforeach(TEST_NAME)
