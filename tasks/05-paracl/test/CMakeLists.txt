function(add_pass_test TEST_NAME FOLDER_PATH)

  add_test(
    NAME ${TEST_NAME}
    COMMAND
      ${BASH_PROGRAM} ${SCRIPTS_DIR}/test_compare.sh "$<TARGET_FILE:pclc>"
      ${CMAKE_CURRENT_SOURCE_DIR}/${FOLDER_PATH} "$<TARGET_FILE:pclvm>")

endfunction()

add_pass_test(test.paracl.external external)
add_pass_test(test.paracl.basic basic)
add_pass_test(test.paracl.blocks blocks)
add_pass_test(test.paracl.functions functions)
add_pass_test(test.paracl.morefunctions morefunctions)
add_pass_test(test.paracl.globals globals)

add_test(NAME test.paracl.fail
         COMMAND ${BASH_PROGRAM} ${SCRIPTS_DIR}/test_fail.sh
                 "$<TARGET_FILE:pclc>" ${CMAKE_CURRENT_SOURCE_DIR}/errors)
