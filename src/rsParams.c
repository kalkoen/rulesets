//
// Created by 20193736 on 10/03/2026.
//

#include "rsParams.h"
#include "typedef.h"

#define DEFAULT_MODEL_TYPE          MODEL_THRESHOLD
#define DEFAULT_METHOD              METHOD_CG
#define DEFAULT_ITERATIVE_ENABLED   FALSE
#define DEFAULT_EXACT_ENABLED       TRUE
#define DEFAULT_HEURISTIC_ENABLED   TRUE
#define DEFAULT_BEAM_WIDTHS         "50,20,20,10"
#define DEFAULT_DIVERSITY           2
#define DEFAULT_MAX_CANDIDATES      10
#define DEFAULT_D                   INT_MAX
#define DEFAULT_EXACT_TIMELIMIT     INT_MAX
#define DEFAULT_CG_TIMELIMIT        INT_MAX
#define DEFAULT_QUANTILES           "9"
#define DEFAULT_NEGATE              NEGATE_NO_MIXING
#define DEFAULT_DEEP                FALSE
#define DEFAULT_COMPLEXITY          2

#define DEFAULT_NONADDITIVE         FALSE
#define DEFAULT_COVER_POSITIVE      FALSE
#define DEFAULT_BINARY_PRICING_CONSTRAINT_TYPE      BINARY_PRICING_CONSTRAINT_TYPE_SINGLE

extern
SCIP_RETCODE setSCIPParameters(
    SCIP* scip /**< SCIP data structure */
)
{
    assert(scip != NULL);

    return SCIP_OKAY;
}
SCIP_RETCODE addRSParameters(
    SCIP* scip /**< SCIP data structure */
)
{
    assert(scip != NULL);

    SCIP_CALL(SCIPaddIntParam(scip, "rs/type",
        "Type of rule set / data (0: boolean rule sets / boolean data, 1: threshold rule sets / continuous data)",
        NULL, TRUE, DEFAULT_MODEL_TYPE, 0, 1, NULL, NULL));

    SCIP_CALL(SCIPaddIntParam(scip, "rs/method",
        "Method used to find the rule set (0: full IP, 1: column generation with heuristic and exact pricer)",
        NULL, TRUE, DEFAULT_METHOD, 0, 1, NULL, NULL));

    SCIP_CALL(SCIPaddIntParam(scip, "rs/rule_complexity_type",
        "Definition of complexity (0: c = 1, 1: c = |S|, 2: c = 1 + |S|)",
        NULL, TRUE, DEFAULT_COMPLEXITY, 0, 2, NULL, NULL));

    SCIP_CALL(SCIPaddBoolParam(scip, "rs/cover_positive",
        "Whether to force coverage of all positive points in a solution",
        NULL, TRUE, DEFAULT_COVER_POSITIVE, NULL, NULL));

    SCIP_CALL(SCIPaddIntParam(scip, "rs/full/D", "Maximum rule size for full method",
        NULL, TRUE, DEFAULT_D, 0, INT_MAX, NULL, NULL));

    SCIP_CALL(SCIPaddBoolParam(scip, "rs/iterative/enabled", "Whether to enable iterative pricer",
        NULL, TRUE, DEFAULT_ITERATIVE_ENABLED, NULL, NULL));

    SCIP_CALL(SCIPaddBoolParam(scip, "rs/heuristic/enabled", "Whether to enable heuristic pricer",
        NULL, TRUE, DEFAULT_HEURISTIC_ENABLED, NULL, NULL));

    SCIP_CALL(SCIPaddBoolParam(scip, "rs/exact/enabled", "Whether to enable exact pricer",
        NULL, TRUE, DEFAULT_EXACT_ENABLED, NULL, NULL));

    SCIP_CALL(SCIPaddIntParam(scip, "rs/iterative/D", "Maximum rule size for iterative pricer",
        NULL, TRUE, DEFAULT_D, 0, INT_MAX, NULL, NULL));

    SCIP_CALL(SCIPaddIntParam(scip, "rs/iterative/max_candidates",
        "Per-round maximum number of rules added by the iterative pricer",
        NULL, TRUE, DEFAULT_MAX_CANDIDATES, 0, INT_MAX, NULL, NULL));

    SCIP_CALL(SCIPaddStringParam(scip, "rs/heuristic/beam_widths",
        "Comma-separated beam widths per depth for the heuristic pricer",
        NULL, TRUE, DEFAULT_BEAM_WIDTHS, NULL, NULL));

    SCIP_CALL(SCIPaddIntParam(scip, "rs/heuristic/diversity",
        "Diversity maximum used by the heuristic pricer",
        NULL, TRUE, DEFAULT_DIVERSITY, 0, INT_MAX, NULL, NULL));

    SCIP_CALL(SCIPaddIntParam(scip, "rs/heuristic/max_candidates",
        "Per-round maximum number of rules added by the heuristic pricer",
        NULL, TRUE, DEFAULT_MAX_CANDIDATES, 0, INT_MAX, NULL, NULL));

    SCIP_CALL(SCIPaddBoolParam(scip, "rs/heuristic/deep", "Whether to enable deep heuristic pricer",
        NULL, TRUE, DEFAULT_DEEP, NULL, NULL));

    SCIP_CALL(SCIPaddIntParam(scip, "rs/exact/D", "Maximum rule size for exact pricer",
        NULL, TRUE, DEFAULT_D, 0, INT_MAX, NULL, NULL));

    SCIP_CALL(SCIPaddRealParam(scip, "rs/exact/timelimit", "Time limit per exact pricer call",
        NULL, TRUE, DEFAULT_EXACT_TIMELIMIT, 0.0, INT_MAX, NULL, NULL));

    SCIP_CALL(SCIPaddRealParam(scip, "rs/cg_timelimit",
        "Time limit for column generation as a whole; must be less than limits/time (if set), "
        "otherwise the final MIP cannot be solved",
        NULL, TRUE, DEFAULT_CG_TIMELIMIT, 0.0, INT_MAX, NULL, NULL));

    SCIP_CALL(SCIPaddStringParam(scip, "rs/threshold/quantiles",
        "Number of quantiles to consider for threshold model",
        NULL, TRUE, DEFAULT_QUANTILES, NULL, NULL));

    SCIP_CALL(SCIPaddIntParam(scip, "rs/threshold/negation",
        "Type of threshold negation (0: no negation, 1: negation without mixing, 2: negation with mixing)",
        NULL, TRUE, DEFAULT_NEGATE, 0, 2, NULL, NULL));

    SCIP_CALL(SCIPaddBoolParam(scip, "rs/threshold/nonadditive", "Whether to use the non-additive approach",
        NULL, TRUE, DEFAULT_NONADDITIVE, NULL, NULL));

    SCIP_CALL(SCIPaddIntParam(scip, "rs/exact/binary_pricing_constraint_type",
        "Constraint type in binary pricing problem "
        "(0: one constraint per datapoint, 1: one constraint per datapoint and non-satisfied feature)",
        NULL, TRUE, DEFAULT_BINARY_PRICING_CONSTRAINT_TYPE, 0, 1, NULL, NULL));

    return SCIP_OKAY;
}