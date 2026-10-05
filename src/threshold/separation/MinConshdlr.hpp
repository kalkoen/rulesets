#pragma once

#include "../ExactPricerThreshold.hpp"
#include "objscip/objscip.h"
#include <cstdint>

class MinConshdlr : public scip::ObjConshdlr
{
public:
    explicit MinConshdlr(ExactPricerThreshold* pricer, SCIP* subscip);
    ~MinConshdlr() override = default;

    static SCIP_RETCODE createCons(
    const ExactPricerThreshold* pricer,
    SCIP* subscip,
    SCIP_CONS** cons,
    const char* name,
    uint32_t datapoint,
    bool initial = true,
    bool separate = true,
    bool enforce = true,
    bool check = true,
    bool propagate = true,
    bool local = false,
    bool modifiable = false,
    bool dynamic = false,
    bool removable = false, bool stickingatnode = false);
    SCIP_RETCODE scip_init(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS** conss, int nconss);

    SCIP_DECL_CONSINITSOL(scip_initsol) override;
    SCIP_RETCODE scip_exit(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS** conss, int nconss);
    SCIP_RETCODE scip_exitsol(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_CONS** conss, int nconss);
    SCIP_DECL_CONSEXITSOL(scip_exitsol) override;

    SCIP_DECL_CONSCHECK  (scip_check)   override;
    bool MinConsSatisfiedInBinarySolution(SCIP* scip, const SolutionValues& sv, uint32_t datapoint) const;
    bool MinConsSatisfied(SCIP* scip, uint32_t datapoint, double gamma_i, const std::vector<double>& z_vals) const;
    double MinSelectedGamma(SCIP* scip, const std::vector<double>& z_vals, uint32_t datapoint) const;
    SCIP_DECL_CONSSEPALP(scip_sepalp) override;
    SCIP_DECL_CONSSEPASOL(scip_sepasol) override;
    SCIP_DECL_CONSPROP   (scip_prop)    override;
    SCIP_DECL_CONSENFOLP (scip_enfolp)  override;
    std::vector<double> GetLocalLbZ() const;
    std::vector<double> GetLocalUbZ() const;
    SCIP_DECL_CONSENFOPS(scip_enfops) override;
    SCIP_DECL_CONSLOCK   (scip_lock)    override;
    SCIP_DECL_CONSCOPY(scip_copy) override;
    SCIP_DECL_CONSDELETE (scip_delete)  override;
    SCIP_DECL_CONSTRANS  (scip_trans)   override;
    SCIP_DECL_CONSPRINT  (scip_print)   override;

    SolutionValues ExtractSolutionValues(SCIP* scip, SCIP_SOL* sol) const;



private:
    ExactPricerThreshold* pricer_;

    std::vector<SCIP_VAR*> g_trans_;
    std::vector<SCIP_VAR*> z_trans_;

    SCIP_RETCODE separate(
        SCIP*        scip,
        SCIP_CONS**  conss,
        int          nconss,
        SCIP_SOL*    sol,
        SCIP_RESULT* result) const;
    SCIP_RETCODE AddCut(SCIP* scip, SCIP_CONSHDLR* conshdlr, SCIP_SOL* sol, Cut& cut, double lb, double ub) const;

    static inline uint32_t getDatapoint(SCIP_CONS* cons)
    {
        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(SCIPconsGetData(cons)));
    }
};
