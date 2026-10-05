//
// Created by 20193736 on 10/03/2026.
//

#ifndef CODE_BRSPARAMS_H
#define CODE_BRSPARAMS_H

// SCIP include
#include <scip/scip.h>

#ifdef __cplusplus
extern "C" {
#endif

    /** Set basic SCIP parameters that are relevant for computing RC */
    extern
    SCIP_RETCODE setSCIPParameters(
       SCIP*                 scip                /**< SCIP data structure */
       );

    /** Introduce parameters that are relevant for computing RC */
    extern
    SCIP_RETCODE addRSParameters(
       SCIP*                 scip                /**< SCIP data structure */
       );

#ifdef __cplusplus
}
#endif

#endif //CODE_BRSPARAMS_H