//
// Created by 20193736 on 15/01/2026.
//

#include "ExactPricerBinary.hpp"
#include "MasterProblemBinary.hpp"

#include <algorithm>

#include "scip/scipdefplugins.h"
#include <iostream>
#include <cstdio>

#include "../typedef.h"

using MasterProblemB = MasterProblem<BooleanClause>;

SCIP_RETCODE ExactPricerBinary::scip_init(SCIP* scip, SCIP_PRICER* pricer)
{
   const MasterProblemB& mp = *mp_;

   d.resize(mp.NumberOfDatapoints());
   z.resize(mp.NumberOfClauses());

   SCIP_CALL(SCIPcreate(&subscip));
   SCIP_CALL(SCIPincludeDefaultPlugins(subscip));
   SCIPsetMessagehdlrQuiet(subscip, false);
   SCIP_CALL(SCIPcreateProbBasic(subscip, "PP"));

   SCIP_CALL(SCIPsetEmphasis(scip, SCIP_PARAMEMPHASIS_OPTIMALITY, true));

   SCIP_CALL(SCIPgetIntParam(scip, "rs/exact/D", &D));
   SCIP_CALL(SCIPgetIntParam(scip, "rs/exact/binary_pricing_constraint_type", &constraint_type));

   for (size_t i = 0; i < mp.NumberOfDatapoints(); i++)
   {
      SCIP_VAR* var = nullptr;
      SCIP_CALL(SCIPcreateVarBasic(subscip, &var, ("d_" + std::to_string(i)).c_str(),
         0.0, 1.0, 0.0, SCIP_VARTYPE_BINARY));
      SCIP_CALL(SCIPaddVar(subscip, var));
      d[i] = var;
   }

   for (size_t j = 0; j < mp.NumberOfClauses(); j++)
   {
      SCIP_VAR* var = nullptr;
      SCIP_CALL(SCIPcreateVarBasic(subscip, &var, ("z_" + std::to_string(j)).c_str(),
         0.0, 1.0, 0.0, SCIP_VARTYPE_BINARY));
      SCIP_CALL(SCIPaddVar(subscip, var));
      z[j] = var;
   }

   for (size_t i = 0; i < mp.NumberOfDatapoints(); ++i)
   {

      if (constraint_type == BINARY_PRICING_CONSTRAINT_TYPE_SINGLE)
      {
         std::string consName = "con_d_" + std::to_string(i);
         std::vector<SCIP_VAR*> vars;
         std::vector<SCIP_Real> coefs;
         vars.reserve(mp.NumberOfClauses());
         coefs.reserve(mp.NumberOfClauses());
         vars.push_back(d[i]);
         coefs.push_back(D);

         for (size_t j = 0; j < mp.NumberOfClauses(); ++j)
         {
            if (not mp.HasFeature(i, j))
            {
               vars.push_back(z[j]);
               coefs.push_back(1.0);
            }
         }

         SCIP_CONS* consD = nullptr;
         SCIP_CALL(SCIPcreateConsBasicLinear(subscip, &consD,
            ("con_d_" + std::to_string(i)).c_str(),
            static_cast<int>(vars.size()),
            vars.data(),
            coefs.data(),
            -SCIPinfinity(subscip),
            D));
         SCIP_CALL(SCIPaddCons(subscip, consD));
         SCIP_CALL(SCIPreleaseCons(subscip, &consD));
      }

      else if (constraint_type == BINARY_PRICING_CONSTRAINT_TYPE_SEVERAL)
      {
         for (int j = 0; j < mp.NumberOfClauses(); ++j)
         {
            if (not mp.HasFeature(i, j))
            {
               SCIP_CONS* consUB = nullptr;
               std::vector<SCIP_VAR*> two_vars = {d[i], z[j]};
               std::vector<SCIP_Real> two_coefs = {1.0, 1.0};
               SCIP_CALL(SCIPcreateConsBasicLinear(subscip, &consUB,
                  ("con_ub_" + std::to_string(i) + "_" + std::to_string(j)).c_str(),
                  2, // number of vars
                  two_vars.data(), // array of vars
                  two_coefs.data(), // array of coefs
                  -SCIPinfinity(subscip), // lhs
                  1.0)); // rhs
               SCIP_CALL(SCIPaddCons(subscip, consUB));
               SCIP_CALL(SCIPreleaseCons(subscip, &consUB));
            }
         }
      }



      // Collect variables and coefficients
      std::vector<SCIP_VAR*> vars;
      std::vector<SCIP_Real> coefs;

      vars.reserve(mp.NumberOfClauses()+1);
      coefs.reserve(mp.NumberOfClauses()+1);

      vars.push_back(d[i]);
      coefs.push_back(1.0);

      for (size_t j = 0; j < mp.NumberOfClauses(); ++j)
      {
         if (not mp.HasFeature(i, j))
         {
            vars.push_back(z[j]);
            coefs.push_back(1.0);
         }
      }

      SCIP_CONS* consLB = nullptr;
      SCIP_CALL(SCIPcreateConsBasicLinear(subscip, &consLB,
         ("con_lb_" + std::to_string(i)).c_str(),
         static_cast<int>(vars.size()), // number of vars
         vars.data(), // array of vars
         coefs.data(), // array of coefs
         1, // lhs
         SCIPinfinity(subscip)));

      SCIP_CALL(SCIPaddCons(subscip, consLB));
      SCIP_CALL(SCIPreleaseCons(subscip, &consLB));
   }


   // Create constraint: sum_j z[j] <= D
   SCIP_CONS* cons = nullptr;
   std::vector<SCIP_Real> coefs(z.size(), 1.0);
   SCIP_CALL(SCIPcreateConsBasicLinear(subscip, &cons,
      "sum_z_leq_D", // constraint name
      static_cast<int>(z.size()), // number of vars
      z.data(), // array of vars
      coefs.data(), // all coefs = 1.0
      -SCIPinfinity(subscip), // lhs
      D)); // rhs

   SCIP_CALL(SCIPaddCons(subscip, cons));
   SCIP_CALL(SCIPreleaseCons(subscip, &cons));

   for (int i = 0; i < d.size(); ++i)
   {
      if (!mp.IsPositive(i))
      {
         SCIPchgVarObj(subscip, d[i], 1.0);
      }
   }

   return SCIP_OKAY;
}

/** Reduced cost pricing method of the pricer */
SCIP_RETCODE ExactPricerBinary::scip_redcost(
   SCIP* scip,
   SCIP_PRICER* pricer,
   SCIP_Real* lowerbound,
   SCIP_Bool* stopearly,
   SCIP_RESULT* result
)
{
   MasterProblemB& mp = *mp_;

   std::cout << "Exact Pricer Called.";

   ShadowPrices shadow_prices;
   SCIP_CALL(mp.ExtractShadowPrices(scip, shadow_prices));
   const auto& lambda = shadow_prices.lambda;

   SCIP_CALL(UpdatePricingProblem(mp, shadow_prices));

   double time_limit = MasterProblemB::ComputeCGTimeLimit(scip, "rs/exact/timelimit");
   SCIPsetRealParam(subscip, "limits/time", time_limit);

   SCIP_CALL(SCIPsolve(subscip));

   *result = ::SCIP_DIDNOTRUN;
   // read solution
   if (SCIP_SOL* sol = SCIPgetBestSol(subscip); sol != nullptr)
   {
      double soltime = SCIPgetSolvingTime(subscip);
      const SCIP_Real obj = SCIPgetSolOrigObj(subscip, sol);

      double real_obj = obj;
      if (mp_->RuleComplexityType() == COMPLEXITY_ONE or mp_->RuleComplexityType() == COMPLEXITY_ONE_PLUS_CLAUSES)
      {
         real_obj += lambda;
      }

      if (SCIPisDualfeasNegative(scip, real_obj))
      {
         BooleanRule rule;
         std::cout << "Rule found: ";
         for (int j = 0; j < z.size(); ++j)
         {
            if (const SCIP_Real val = SCIPgetSolVal(subscip, sol, z[j]); SCIPisEQ(subscip, val, 1))
            {
               std::cout << j;
               std::cout << ",";
               rule.clauses.insert(BooleanClause(j));
            }
         }
         std::cout << std::endl;
         if (!rule.empty())
         {
            auto red_cost = mp.ReducedCost(rule, shadow_prices);
            std::cout << "+ Rule: " << rule << "\t Reduced cost: " << red_cost << std::endl;

            uint32_t rule_idx{}; 
            SCIP_CALL(mp.AddRule(scip, rule, true, false, rule_idx));
            *result = SCIP_SUCCESS;

            mp.RecordPricingStep(scip, PRICER_EXACT, red_cost);
         }
         else
         {
            std::cout << "Empty rule!" << std::endl;
         }

         std::cout << ">Sol time " + std::to_string(soltime) + "\n";
      }
      else
      {
         std::cout << "Rule found, but no reduced cost \n";
      }
   }
   else
   {
      std::cout << "No solution found.";
   }

   return SCIP_OKAY;
}

/** Farkas pricing method (for feasibility) */
SCIP_RETCODE ExactPricerBinary::scip_farkas(
   SCIP* scip,
   SCIP_PRICER* pricer,
   SCIP_RESULT* result
)
{
   *result = ::SCIP_DIDNOTRUN;
   return SCIP_OKAY;
}


SCIP_RETCODE ExactPricerBinary::scip_exit(SCIP* scip, SCIP_PRICER* pricer)
{
   SCIP_CALL(SCIPfreeTransform(subscip));

   for (auto& v : d)
   {
      SCIP_CALL(SCIPreleaseVar(subscip, &v));
   }

   for (auto& v : z)
   {
      SCIP_CALL(SCIPreleaseVar(subscip, &v));
   }

   SCIP_CALL(SCIPfree(&subscip));

   return SCIP_OKAY;
}


SCIP_RETCODE ExactPricerBinary::UpdatePricingProblem(const MasterProblemB& mp, const ShadowPrices& shadow_prices) const
{
   SCIPfreeTransform(subscip);

   for (const auto i : mp.data.y)
   {
      SCIP_CALL(SCIPchgVarObj(subscip, d[i], -shadow_prices.mu[i]));
   }

   if (mp_->RuleComplexityType() == COMPLEXITY_CLAUSES or mp_->RuleComplexityType() == COMPLEXITY_ONE_PLUS_CLAUSES)
   {
      for (auto j : z)
      {
         SCIP_CALL(SCIPchgVarObj(subscip, j, shadow_prices.lambda));
      }

   }
   return SCIP_OKAY;
}
