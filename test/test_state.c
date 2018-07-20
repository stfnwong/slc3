/* TEST_STATE
 * Unit test for LC-3 state object
 *
 * Stefan Wong 2018
 */

#include <stdlib.h>
#include <check.h>
#include <stdio.h>
// Module under test
#include "isa.h"
#include "util.h"

// Init test
START_TEST(test_init_state)
{
    uint32_t mem_size = 4096;
    uint32_t trace_size = 512;

    LC3State* state = init_state(mem_size, trace_size);
    // Check memory size was assigned correctly 
    ck_assert_int_eq(mem_size, state->mem_size);
    ck_assert_int_eq(trace_size, state->pc_trace->size);

    // Ensure that the memory is initalized correctly 
    for(int i = 0; i < mem_size; i++)
        ck_assert_int_eq(0, state->memory[i]);
    // Ensure the trace is inintialized correcly
    for(int i = 0; i < trace_size; i++)
        ck_assert_int_eq(0, state->pc_trace->trace[i]);

    // Ensure that state is initialized correctly
    for(int i = 0; i < 8; i++)
        ck_assert_int_eq(0, state->gpr[i]);
    ck_assert_int_eq(0, state->cc.n);
    ck_assert_int_eq(0, state->cc.z);
    ck_assert_int_eq(0, state->cc.p);

    destroy_state(state);

} END_TEST

//START_TEST(test_gpr_mapping)
//{
//    uint32_t mem_size = 4096;
//    uint32_t trace_size = 512;
//
//    LC3State* state = init_state(mem_size, trace_size);
//    // Check memory size was assigned correctly 
//    ck_assert_int_eq(mem_size, state->mem_size);
//    ck_assert_int_eq(trace_size, state->pc_trace->size);
//
//    // Look inside gpr
//
//    destroy_state(state);
//} END_TEST

START_TEST(test_memory_load)
{
    uint32_t mem_size = 4096;
    uint32_t trace_size = 512;

    LC3State* state = init_state(mem_size, trace_size);
    // Check memory size was assigned correctly 
    ck_assert_int_eq(mem_size, state->mem_size);
    ck_assert_int_eq(trace_size, state->pc_trace->size);
    // Ensure that the memory is initalized correctly 
    for(int i = 0; i < mem_size; i++)
        ck_assert_int_eq(0, state->memory[i]);

    fprintf(stdout, "Generating random memory contents... ");
    uint8_t *mem_contents;
    mem_contents = malloc(sizeof(uint8_t) * mem_size);
    if(!mem_contents)
    {
        fprintf(stderr, "Failed to allocate memory for random memory test\n");
        exit(-1);
    }
    for(int i = 0; i < mem_size; i++)
        mem_contents[i] = rand_interval(0, 255);

    const char* mem_filename = "data/mem_test.dat";
    FILE *fp;
    fp = fopen(mem_filename, "wb");
    if(!fp)
    {
        fprintf(stderr, "Failed to open memory test file [%s] for writing\n",
                mem_filename);
        exit(-1);
    }
    fwrite(mem_contents, 1, mem_size, fp);
    fclose(fp);

    // Now try to read the contents into the state memory and check. 
    // The original contents are still in heap memory
    int ret = load_memory(state, mem_filename, 0);
    if(ret != 0)
        exit(-1);

    // Check the memory contents
    for(int i = 0; i < state->mem_size; i++)
        ck_assert_int_eq(mem_contents[i], state->memory[i]);

    free(mem_contents);
    fprintf(stdout, "done\n");

} END_TEST

Suite* stateSuite(void)
{
    Suite *s;
    TCase *tc_test_state_init;
    TCase *tc_test_memory_load;

    s = suite_create("state");
    // See if we can init and destroy the state object correctly
    tc_test_state_init = tcase_create("Test State Init");
    tcase_add_test(tc_test_state_init, test_init_state);
    suite_add_tcase(s, tc_test_state_init);
    // Test the GPR mapping 

    // Test memory load and read
    tc_test_memory_load = tcase_create("Test Memory Load");
    tcase_add_test(tc_test_memory_load, test_memory_load);
    suite_add_tcase(s, tc_test_memory_load);

    return s;
}

int main(void)
{
    int num_failed;
    Suite *s;
    SRunner *sr;

    s = stateSuite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    num_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
