#include "CUnit/Basic.h"
#include "CUnit/Automated.h"

extern  CU_SuiteInfo EEP_Suites[];
extern  CU_SuiteInfo SBP_Suites[];
extern  CU_SuiteInfo SYST_Suites[];

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main(int argc, char *argv[])
{
    int console   = 1;
    int automated = 0;
   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();
   if (CUE_SUCCESS != CU_register_nsuites(3, SYST_Suites, EEP_Suites, SBP_Suites))
   {
       CU_cleanup_registry();
       return CU_get_error();
   }

   while(argc--)
   {
       if      (  (strcmp(*argv,  "-n") == 0)
               || (strcmp(*argv, "--no-console") == 0)
               ) console   = 0;
       else if (  (strcmp(*argv,  "-a") == 0)
               || (strcmp(*argv, "--automated") == 0)
               ) automated   = 1;
       argv++;
   }
   if (console)
   {
       /* Run all tests using the CUnit Basic interface */
       CU_basic_set_mode(CU_BRM_VERBOSE);
       CU_basic_run_tests();
   }

   if (automated)
   {
       CU_set_output_filename("sblib");
       CU_list_tests_to_file();
       CU_set_output_filename("sblib-results");
       CU_automated_run_tests();
       printf("Test result XML files created\n");
   }

   CU_cleanup_registry();
   return CU_get_error();
}

