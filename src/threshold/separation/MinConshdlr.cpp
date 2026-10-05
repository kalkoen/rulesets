//
// Created by 20193736 on 03/06/2026.
//

#include "MinConshdlr.hpp"

#include "LowerBoundOracle.hpp"
#include "UpperBoundOracle.hpp"
#include "../ExactPricerThreshold.hpp"

static const auto HANDLER_NAME = "MinConstrhdlr";
static const auto HANDLER_DESC = "gamma <= or >= min_{j : z_j = 1}(gamma_j) constraint handler";
static constexpr int HANDLER_SEPAPRIORITY = 1000; // separation priority
static constexpr int HANDLER_ENFOPRIORITY = -1000; // enforcement priority (negative = after integrality cons handlr)
static constexpr int HANDLER_CHECKPRIORITY = -1000;
static constexpr int HANDLER_SEPAFREQ = 1; // separate every node
static constexpr int HANDLER_PROPFREQ = 1; // propagate every node
static constexpr int HANDLER_EAGERFREQ = 100;
static constexpr int HANDLER_MAXPREROUNDS = -1; // unlimited presolve rounds
static constexpr SCIP_Bool HANDLER_DELAYSEPA = FALSE;
static constexpr SCIP_Bool HANDLER_DELAYPROP = FALSE;
static constexpr SCIP_Bool HANDLER_NEEDSCONS = TRUE;
static const auto HANDLER_PROPTIMING = SCIP_PROPTIMING_BEFORELP;
static const auto HANDLER_PRESOLTIMING = SCIP_PRESOLTIMING_FAST;

MinConshdlr::MinConshdlr(ExactPricerThreshold* pricer, SCIP* subscip)
    : ObjConshdlr(subscip,
                  HANDLER_NAME,
                  HANDLER_DESC,
                  HANDLER_SEPAPRIORITY,
                  HANDLER_ENFOPRIORITY,
                  HANDLER_CHECKPRIORITY,
                  HANDLER_SEPAFREQ,
                  HANDLER_PROPFREQ,
                  HANDLER_EAGERFREQ,
                  HANDLER_MAXPREROUNDS,
                  HANDLER_DELAYSEPA,
                  HANDLER_DELAYPROP,
                  HANDLER_NEEDSCONS,
                  HANDLER_PROPTIMING,
                  HANDLER_PRESOLTIMING), pricer_(pricer)
{
}

SCIP_RETCODE MinConshdlr::createCons(
    const ExactPricerThreshold* pricer,
    SCIP* subscip,
    SCIP_CONS** cons,
    const char* name,
    const uint32_t datapoint,
    const bool initial,
    const bool separate,
    const bool enforce,
    const bool check,
    const bool propagate,
    const bool local,
    const bool modifiable,
    const bool dynamic,
    const bool removable,
    const bool stickingatnode)
{
    assert(subscip != nullptr && cons != nullptr);
    // 1. Locate the handler that was previously included via SCIPincludeObjConshdlr.
    SCIP_CONSHDLR* hdlr = SCIPfindConshdlr(subscip, HANDLER_NAME);
    if (hdlr == nullptr)
    {
        SCIPerrorMessage("Constraint handler <%s> not found\n", HANDLER_NAME);
        return SCIP_PLUGINNOTFOUND;
    }

    SCIP_CALL(SCIPcreateCons(subscip, cons, name, hdlr,
        reinterpret_cast<SCIP_CONSDATA*>(static_cast<uintptr_t>(datapoint)),
        initial, separate, enforce, check, propagate,
        local, modifiable, dynamic, removable,
        stickingatnode));

    return SCIP_OKAY;
}

SCIP_DECL_CONSINIT(MinConshdlr::scip_init)
{
    const auto ng = pricer_->g.size();
    const auto nz = pricer_->z.size();
    g_trans_.resize(ng);
    z_trans_.resize(nz);
    SCIP_CALL(SCIPgetTransformedVars(scip, ng, pricer_->g.data(), g_trans_.data()));
    SCIP_CALL(SCIPgetTransformedVars(scip, nz, pricer_->z.data(), z_trans_.data()));
    return SCIP_OKAY;
}

SCIP_DECL_CONSINITSOL(MinConshdlr::scip_initsol)
{
    return SCIP_OKAY;
}

SCIP_DECL_CONSEXIT(MinConshdlr::scip_exit)
{
    g_trans_.clear();
    z_trans_.clear();
    return SCIP_OKAY;
}


SCIP_DECL_CONSEXITSOL(MinConshdlr::scip_exitsol)
{
    return SCIP_OKAY;
}

SCIP_DECL_CONSCHECK(MinConshdlr::scip_check)
{
    // assert(!z_trans_.empty() && "scip_check cfalled before scip_initsol");

    *result = SCIP_FEASIBLE;
    const SolutionValues sv = ExtractSolutionValues(scip, sol);

    // If z is fractional, this isn't a MIP solution candidate — not our job
    for (const auto z_val : sv.z)
        if (!SCIPisFeasIntegral(scip, z_val))
            return SCIP_OKAY;

    for (int c = 0; c < nconss; ++c)
    {
        if (const uint32_t datapoint = getDatapoint(conss[c]);
            !MinConsSatisfiedInBinarySolution(scip, sv, datapoint))
        {
            // const auto& pd = pricer_->point_data[datapoint];
            // SCIPinfoMessage(scip, nullptr, "[CHECK FAIL] datapoint=%u type=%s g_i=%.15g\n",
            //                 datapoint, pd.type == MinType::UPPER_BOUND ? "UB" : "LB", sv.g[datapoint]);
            // for (int l = 0; l < sv.z.size(); ++l)
            //     SCIPinfoMessage(scip, nullptr, "  z[%d]=%.15g gamma=%.15g\n",
            //                     l, sv.z[l], pd.gammas_unsorted[l]);

            *result = SCIP_INFEASIBLE;
            return SCIP_OKAY;
        }
    }

    return SCIP_OKAY;
}

bool MinConshdlr::MinConsSatisfiedInBinarySolution(SCIP* scip, const SolutionValues& sv,
                                                   const uint32_t datapoint) const
{
    const auto g_i = sv.g[datapoint];
    return MinConsSatisfied(scip, datapoint, g_i, sv.z);
}

bool MinConshdlr::MinConsSatisfied(
    SCIP* scip,
    const uint32_t datapoint,
    const double gamma_i,
    const std::vector<double>& z_vals) const
{
    const auto& pd = pricer_->point_data[datapoint];
    if (pd.type == MinType::UPPER_BOUND)
    {
        int n_pos = 0;
        for (int l = 0; l < z_vals.size(); ++l)
        {
            const auto z_l = z_vals[l];
            const auto gamma = pd.gammas_unsorted[l];
            if (SCIPisFeasGT(scip, z_l, 0))
            {
                if (SCIPisFeasGT(scip, gamma_i, gamma))
                {
                    return false;
                }
                n_pos++;
            }
        }
        if (n_pos == 0 and SCIPisFeasGT(scip, gamma_i, std::ranges::max(pd.gammas_unsorted)))
        {
            return false;
        }
    }
    else
    {
        int n_pos = 0;
        if (const auto min_selected_gamma =
            MinSelectedGamma(scip, z_vals, datapoint); SCIPisFeasLT(scip, gamma_i, min_selected_gamma))
        {
            return false;
        }
    }
    return true;
}

double MinConshdlr::MinSelectedGamma(SCIP* scip, const std::vector<double>& z_vals, const uint32_t datapoint) const
{
    const auto& pd = pricer_->point_data[datapoint];
    double min_val = 2.0;
    for (int l = 0; l < z_vals.size(); ++l)
    {
        const auto z_l = z_vals[l];
        const auto gamma = pd.gammas_unsorted[l];
        if (SCIPisFeasGT(scip, z_l, 0))
        {
            min_val = std::min(min_val, gamma);
        }
    }
    return min_val;
}

SCIP_DECL_CONSSEPALP(MinConshdlr::scip_sepalp)
{
    *result = SCIP_DIDNOTFIND;
    SCIP_CALL(separate(scip, conss, nconss, nullptr, result));
    return SCIP_OKAY;
}

SCIP_DECL_CONSSEPASOL(MinConshdlr::scip_sepasol)
{
    *result = SCIP_DIDNOTFIND;
    SCIP_CALL(separate(scip, conss, nconss, sol, result));
    return SCIP_OKAY;
}

SCIP_DECL_CONSPROP(MinConshdlr::scip_prop)
{
    *result = SCIP_DIDNOTFIND;
    int total_tightened = 0;


    for (int c = 0; c < nconss; ++c)
    {
        const uint32_t datapoint = getDatapoint(conss[c]);
        const auto var_g = g_trans_[datapoint];
        const auto& pd = pricer_->point_data[datapoint];

        SCIP_Bool infeasible, tightened;

        if (pd.type == MinType::LOWER_BOUND)
        {
            const auto lowest_lb = MinSelectedGamma(
                scip,
                GetLocalUbZ(),
                datapoint);

            // std::cout << "Propagating " << datapoint << " at least " << lowest_lb << std::endl;

            SCIP_CALL(SCIPtightenVarLb(scip, var_g, lowest_lb, false, &infeasible, &tightened));
        }
        else
        {
            const auto highest_ub = MinSelectedGamma(
                scip,
                GetLocalLbZ(),
                datapoint);

            // std::cout << "g[" << datapoint << "] lb: "
            //     << SCIPvarGetLbLocal(var_g) << " -> " << highest_ub << "\n";
            // std::cout << "Propagating " << datapoint << " at most " << highest_ub << std::endl;

            SCIP_CALL(SCIPtightenVarUb(scip, var_g, highest_ub, false, &infeasible, &tightened));
        }

        if (infeasible)
        {
            *result = SCIP_CUTOFF;
            return SCIP_OKAY;
        }
        if (tightened)
        {
            *result = SCIP_REDUCEDDOM;
            ++total_tightened;
        }
    }
    // std::cout << "Propagator: " << total_tightened << "/" << nconss << " bounds tightened\n";

    return SCIP_OKAY;
}

SCIP_DECL_CONSENFOLP(MinConshdlr::scip_enfolp)
{
    *result = SCIP_FEASIBLE;

    SCIP_CALL(separate(scip, conss, nconss, nullptr, result));
    SCIPdebugMessage("[enfolp] result=%d (FEASIBLE=1, SEPARATED=3, CUTOFF=4)\n", *result);
    return SCIP_OKAY;
}

std::vector<double> MinConshdlr::GetLocalLbZ() const
{
    std::vector<double> z_lb;
    z_lb.resize(z_trans_.size());
    for (size_t l = 0; l < z_trans_.size(); ++l)
    {
        auto& v = z_trans_[l];
        z_lb[l] = SCIPvarGetLbLocal(v);
    }
    return z_lb;
}

std::vector<double> MinConshdlr::GetLocalUbZ() const
{
    std::vector<double> z_ub;
    z_ub.resize(z_trans_.size());
    for (size_t l = 0; l < z_trans_.size(); ++l)
    {
        auto& v = z_trans_[l];
        z_ub[l] = SCIPvarGetUbLocal(v);
    }
    return z_ub;
}


SCIP_DECL_CONSENFOPS(MinConshdlr::scip_enfops)
{
    *result = SCIP_FEASIBLE;

    const SolutionValues sv = ExtractSolutionValues(scip, nullptr);

    for (int c = 0; c < nconss; ++c)
    {
        if (const uint32_t datapoint = getDatapoint(conss[c]);
            !MinConsSatisfiedInBinarySolution(scip, sv, datapoint))
        {
            *result = SCIP_SOLVELP;
            return SCIP_OKAY;
        }
    }
    return SCIP_OKAY;
}

SCIP_DECL_CONSLOCK(MinConshdlr::scip_lock)
{
    // Conservative locking. TODO impprove.
    for (SCIP_VAR* v : pricer_->z)
    {
        SCIP_CALL(SCIPaddVarLocksType(scip, v, locktype,
            nlockspos,
            nlockspos));
    }

    for (int i = 0; i < pricer_->g.size(); ++i)
    {
        PointData& pd = pricer_->point_data[i];
        auto v = pricer_->g[i];
        if (pd.type == MinType::LOWER_BOUND)
        {
            SCIP_CALL(SCIPaddVarLocksType(scip, v, locktype,
                nlockspos,
                0));
        }
        else
        {
            SCIP_CALL(SCIPaddVarLocksType(scip, v, locktype,
                0,
                nlockspos));
        }
    }

    return SCIP_OKAY;
}

SCIP_DECL_CONSCOPY(MinConshdlr::scip_copy)
{
    // uint32_t idx = getDatapoint(sourcecons);
    // const char* consname = (name != nullptr) ? name : SCIPconsGetName(sourcecons);
    //
    // SCIP_CALL(createCons(pricer_, scip, cons, consname, idx,
    //     initial, separate, enforce, check, propagate,
    //     local, modifiable, dynamic, removable, stickingatnode));
    *valid = false;
    return SCIP_OKAY;
}


SCIP_DECL_CONSDELETE(MinConshdlr::scip_delete)
{
    *consdata = nullptr;
    return SCIP_OKAY;
}

SCIP_DECL_CONSTRANS(MinConshdlr::scip_trans)
{
    const auto datapoint = getDatapoint(sourcecons);

    SCIP_CALL(SCIPcreateCons(scip, targetcons,
        SCIPconsGetName(sourcecons),
        conshdlr,
        reinterpret_cast<SCIP_CONSDATA*>(static_cast<uintptr_t>(datapoint)),
        SCIPconsIsInitial(sourcecons),
        SCIPconsIsSeparated(sourcecons),
        SCIPconsIsEnforced(sourcecons),
        SCIPconsIsChecked(sourcecons),
        SCIPconsIsPropagated(sourcecons),
        SCIPconsIsLocal(sourcecons),
        SCIPconsIsModifiable(sourcecons),
        SCIPconsIsDynamic(sourcecons),
        SCIPconsIsRemovable(sourcecons),
        SCIPconsIsStickingAtNode(sourcecons)));
    return SCIP_OKAY;
}


SCIP_DECL_CONSPRINT(MinConshdlr::scip_print)
{
    SCIPinfoMessage(scip, file, "idx=%" PRIu32 "\n", getDatapoint(cons));
    return SCIP_OKAY;
}

SCIP_RETCODE MinConshdlr::separate(
    SCIP* scip,
    SCIP_CONS** conss,
    int nconss,
    SCIP_SOL* sol,
    SCIP_RESULT* result) const
{
    assert(scip != nullptr);
    SCIP_CONSHDLR* hdlr = SCIPfindConshdlr(scip, HANDLER_NAME);
    if (hdlr == nullptr)
    {
        SCIPerrorMessage("Constraint handler <%s> not found\n", HANDLER_NAME);
        return SCIP_PLUGINNOTFOUND;
    }

    // std::cout << "Separating..." <<std::endl;

    const SolutionValues sv = ExtractSolutionValues(scip, sol);

    std::vector<Cut> cuts;

    // std::cout << sv << std::endl;
    for (int c = 0; c < nconss; ++c)
    {
        uint32_t datapoint = getDatapoint(conss[c]);
        const auto& pd = pricer_->point_data[datapoint];

        if (pd.type == MinType::LOWER_BOUND)
        {
            const auto cut = LowerBoundOracle::FindCutForDatapoint(scip, sv, datapoint, pd, 0.0);
            if (cut.has_value())
            {
                // std::cout << "Cut found: " << cut.value() << std::endl;
                cuts.push_back(cut.value());
            }
            else
            {
                // std::cout << "No cut for lowerbound found " << std::endl;
                // if (! MinConstrSatisfiedInBinarySolution(scip, sv, datapoint))
                // {
                //     std::cout << "No violated LOWER bound found, but the min constraint is not satisfied! ";
                //     std::cout << "Datapoint: " << datapoint << std::endl;
                // }
                //
                // std::cout << std::endl;
            }
        }
        else
        {
            const auto cut = UpperBoundOracle::FindCutForDatapoint(scip, sv, datapoint, pd);
            if (cut.has_value())
            {
                // std::cout << "Cut found: " << cut.value() << std::endl;
                cuts.push_back(cut.value());
            }
            // if (cut_vector.empty())
            // {
            //     if (! MinConstrSatisfiedInBinarySolution(scip, sv, datapoint))
            //     {
            //         std::cout << "No violated UPPER bound found, but the min constraint is not satisfied! ";
            //         std::cout << "Datapoint: " << datapoint << std::endl;
            //     }
            // }
        }
    }

    std::ranges::sort(cuts, std::greater{});
    cuts.resize(std::min<size_t>(10, cuts.size()));

    if (!cuts.empty())
    {
        *result = SCIP_SEPARATED;

        for (auto& cut : cuts)
        {
            // std::cout << "Adding cut " << cut << std::endl;

            const auto& pd = pricer_->point_data[cut.datapoint];
            if (pd.type == MinType::LOWER_BOUND)
            {
                AddCut(scip, hdlr, sol, cut, cut.bound, SCIPinfinity(scip));
            }
            else
            {
                AddCut(scip, hdlr, sol, cut, -SCIPinfinity(scip), cut.bound);
            }
        }
    }

    return SCIP_OKAY;
}


SCIP_RETCODE MinConshdlr::AddCut(
    SCIP* scip,
    SCIP_CONSHDLR* conshdlr,
    SCIP_SOL* sol,
    Cut& cut,
    const double lb,
    const double ub
) const
{
    SCIP_ROW* row;
    SCIP_CALL(SCIPcreateEmptyRowConshdlr(scip, &row, conshdlr, "threshold_cut",
        lb, ub, false, false, true));
    SCIP_CALL(SCIPcacheRowExtensions(scip, row));

    // const auto& g_var = g_trans.empty() ? pricer_->g[cut.datapoint] : g_trans[cut.datapoint];
    const auto& g_var = g_trans_[cut.datapoint];
    SCIP_CALL(SCIPaddVarToRow(scip, row, g_var, 1.0));

    std::vector<SCIP_VAR*> selected_z;
    selected_z.reserve(cut.clause_idxs.size());
    // std::ranges::transform(cut.clause_idxs, std::back_inserter(selected_z),
    //     [&](int i) { return z_trans.empty() ? pricer_->z[i] : z_trans[i]; });
    std::ranges::transform(cut.clause_idxs, std::back_inserter(selected_z),
                           [&](int i) { return z_trans_[i]; });

    SCIP_CALL(SCIPaddVarsToRow(
        scip,
        row,
        selected_z.size(),
        selected_z.data(),
        cut.clause_coeffs.data()));
    SCIP_CALL(SCIPflushRowExtensions(scip, row));

    SCIP_Bool infeasible = FALSE;
    SCIP_CALL(SCIPaddRow(scip, row, FALSE, &infeasible));
    SCIP_CALL(SCIPreleaseRow(scip, &row));

    return SCIP_OKAY;
}

SolutionValues MinConshdlr::ExtractSolutionValues(SCIP* scip, SCIP_SOL* sol) const
{
    const auto& z_vars = z_trans_.empty() ? pricer_->z : z_trans_;
    const auto& g_vars = g_trans_.empty() ? pricer_->g : g_trans_;

    // const auto& z_vars = z_trans_;
    // const auto& g_vars = g_trans_;
    SolutionValues sol_vals;
    sol_vals.g.resize(g_vars.size());
    sol_vals.z.resize(z_vars.size());
    for (int i = 0; i < sol_vals.g.size(); ++i)
        sol_vals.g[i] = SCIPgetSolVal(scip, sol, g_vars[i]);
    for (int l = 0; l < sol_vals.z.size(); ++l)
        sol_vals.z[l] = SCIPgetSolVal(scip, sol, z_vars[l]);
    return sol_vals;
}
