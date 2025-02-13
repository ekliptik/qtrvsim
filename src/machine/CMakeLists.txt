project(machine
        DESCRIPTION "The actual simulator as a library. Link with an UI of your choice.")

set(CMAKE_AUTOMOC ON)

set(machine_SOURCES
        execute/alu.cpp
        cop0state.cpp
        core.cpp
        instruction.cpp
        machine.cpp
        machineconfig.cpp
        memory/backend/lcddisplay.cpp
        memory/backend/memory.cpp
        memory/backend/peripheral.cpp
        memory/backend/peripspiled.cpp
        memory/backend/serialport.cpp
        memory/cache/cache.cpp
        memory/cache/cache_policy.cpp
        memory/frontend_memory.cpp
        memory/memory_bus.cpp
        programloader.cpp
        registers.cpp
        simulator_exception.cpp
        symboltable.cpp
        )

set(machine_HEADERS
        execute/alu.h
        cop0state.h
        core.h
        core/core_state.h
        instruction.h
        machine.h
        machineconfig.h
        machinedefs.h
        memory/address.h
        memory/backend/backend_memory.h
        memory/backend/lcddisplay.h
        memory/backend/memory.h
        memory/backend/peripheral.h
        memory/backend/peripspiled.h
        memory/backend/serialport.h
        memory/cache/cache.h
        memory/cache/cache_policy.h
        memory/cache/cache_types.h
        memory/frontend_memory.h
        memory/memory_bus.h
        memory/memory_utils.h
        programloader.h
		predictor.h
		pipeline.h
		registers.h
        register_value.h
        simulator_exception.h
        symboltable.h
        utils.h
        machine_global.h
        execute/alu_op.h
        execute/mul_op.h
        )
set(machine_TESTS
        tests/data/cache_test_performance_data.h
        tests/tst_machine.h
        tests/utils/integer_decomposition.h
        tests/testcache.cpp
        tests/testcore.cpp
        tests/testinstruction.cpp
        tests/testmemory.cpp
        tests/testprogramloader.cpp
        tests/testregisters.cpp
        tests/tst_machine.cpp
        )


# Object library is preferred, because the library archive is never really
# needed. This option skips the archive creation and links directly .o files.
add_library(machine STATIC
        ${machine_SOURCES}
        ${machine_HEADERS})
target_link_libraries(machine
		PRIVATE ${QtLib}::Core
		PUBLIC libelf)

if (NOT ${WASM})
    # Machine tests (not available on WASM)
#    add_executable(machine_unit_tests ${machine_TESTS})
#    target_link_libraries(machine_unit_tests
    #            PRIVATE machine ${QtLib}::Core ${QtLib}::Test)
#
#    add_test(NAME machine_unit_tests
#            COMMAND machine_unit_tests)

    add_executable(alu_test
            execute/alu.test.cpp
            execute/alu.test.h
            execute/alu.cpp
            execute/alu.h
            )
    target_link_libraries(alu_test
		    PRIVATE ${QtLib}::Core ${QtLib}::Test)
    add_test(NAME alu COMMAND alu_test)
endif ()
