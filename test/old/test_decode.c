/* TEST_DECODE
 * Test the instruction decode loop
 *
 * Stefan Wong 2018
 */

#include <stdlib.h>
#include <check.h>
#include <stdio.h>
// Module under test
#include "decode.h"
#include "isa.h"
#include "util.h"


START_TEST(test_arith_decode)
{
    uint32_t mem_size = 4096;
    uint32_t trace_size = 512;
    LC3State *state = init_state(mem_size, trace_size);

    // TODO: Need an assembler to turn *asm into *.o

    destroy_state(state);

} END_TEST

Suite* decodeSuite(void)
{
    Suite *s;
    TCase *tc_test_arith_decode;

    s = suite_create("state");
    // See if we can init and destroy the state object correctly
    tc_test_arith_decode = tcase_create("Test State Init");
    tcase_add_test(tc_test_arith_decode, test_arith_decode);
    suite_add_tcase(s, tc_test_arith_decode);

    return s;
}

int main(void)
{
    int num_failed;
    Suite *s;
    SRunner *sr;

    s = decodeSuite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    num_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
