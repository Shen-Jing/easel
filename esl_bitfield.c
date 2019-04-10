
#include "esl_config.h"

#include <string.h>

#include "easel.h"
#include "esl_bitfield.h"


ESL_BITFIELD *
esl_bitfield_Create(int nb)
{
  ESL_BITFIELD *b = NULL;
  int nu = (nb + 63) / 64;
  int status;

  ESL_DASSERT1(( nb >= 1 ));

  ESL_ALLOC(b, sizeof(uint64_t) * nu);
  memset((void *) b, 0, sizeof(uint64_t) * nu);
  return b;
  
 ERROR: // also normal exit
  esl_bitfield_Destroy(b);
  return NULL;
}

void
esl_bitfield_Destroy(ESL_BITFIELD *b)
{
  free(b);
}


/*****************************************************************
 * 2. Unit tests
 *****************************************************************/
#ifdef eslBITFIELD_TESTDRIVE

#include "esl_random.h"
#include "esl_vectorops.h"

static void
utest_randpattern(ESL_RANDOMNESS *rng)
{
  char msg[]      = "bitfield randpattern utest failed";
  int  nb         = 1 + esl_rnd_Roll(rng, 192); // 1..192; 1-3 uint64_t's. Keep it small to exercise edge cases frequently.
  int  nu         = (nb + 63) / 64;
  int  nset       = esl_rnd_Roll(rng, nb+1);
  int *deal       = NULL;                       // sample of <nset> elements out of <nb> that are initially set TRUE
  int *bigflags   = NULL;                       // bigflags[] are 1/0 for set/unset bits 
  ESL_BITFIELD *b = esl_bitfield_Create(nb);
  int  i;
  int  status;

  ESL_ALLOC(deal, sizeof(int) * nset);
  ESL_ALLOC(bigflags, sizeof(int) * nb);
  esl_vec_ISet(bigflags, nb, FALSE);
  
  esl_rnd_Deal(rng, nset, nb, deal);
  for (i = 0; i < nset; i++) bigflags[deal[i]] = TRUE;
  for (i = 0; i < nset; i++) esl_bitfield_Set(b, deal[i]);

  for (i = 0; i < nb;   i++) if (bigflags[i] != esl_bitfield_IsSet(b, i)) esl_fatal(msg);
  for (i = 0; i < nb;   i++) esl_bitfield_Toggle(b, i);
  for (i = 0; i < nb;   i++) if (bigflags[i] == esl_bitfield_IsSet(b, i)) esl_fatal(msg);
  for (i = 0; i < nb;   i++) esl_bitfield_Clear(b, i);   // do all bits, to test that clearing 0 bits leaves them 0

  for (i = 0; i < nu;   i++) if (b[i]) esl_fatal(msg);  // note <nu>. this reaches inside the "opaque" ESL_BITFIELD and can break if that structure changes.
  
  free(deal);
  free(bigflags);
  esl_bitfield_Destroy(b);
  return;

 ERROR:
  esl_fatal(msg);
}
#endif // eslBITFIELD_TESTDRIVE

/*****************************************************************
 * 2. Unit tests
 *****************************************************************/
#ifdef eslBITFIELD_TESTDRIVE

#include "esl_getopts.h"

static ESL_OPTIONS options[] = {
  /* name           type      default  env  range toggles reqs incomp  help                             docgroup*/
  { "-h",  eslARG_NONE,   FALSE,  NULL, NULL,  NULL,  NULL, NULL, "show brief help on version and usage",    0 },
  { "-s",  eslARG_INT,      "0",  NULL, NULL,  NULL,  NULL, NULL, "set random number seed to <n>",           0 },
  {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};
static char usage[]  = "[-options]";
static char banner[] = "test driver for bitfield module";

int
main(int argc, char **argv)
{
  ESL_GETOPTS    *go   = esl_getopts_CreateDefaultApp(options, 0, argc, argv, banner, usage);
  ESL_RANDOMNESS *rng  = esl_randomness_Create(esl_opt_GetInteger(go, "-s"));

  fprintf(stderr, "## %s\n", argv[0]);
  fprintf(stderr, "#  rng seed = %" PRIu32 "\n", esl_randomness_GetSeed(rng));

  utest_randpattern(rng);

  fprintf(stderr, "#  status = ok\n");
 
  esl_randomness_Destroy(rng);
  esl_getopts_Destroy(go);
  return eslOK;
}
#endif // eslBITFIELD_TESTDRIVE