//
// Created by 20193736 on 11/03/2026.
//

#include "objscip/objscip.h"
#include "scip/scip.h"
#include "scip/scipdefplugins.h"
#include <iostream>
#include <vector>
#include <map>
#include <cstdio>

#include "rsParams.h"
#include "Utils.hpp"
#include "rapidcsv.h"

#include "typedef.h"
#include "binary/MasterProblemBinary.hpp"
#include "threshold/MasterProblemThreshold.hpp"

SCIP_RETCODE computeRs(
    const std::string& filenameX,
    const std::string& filenameY,
    const int C,
    const std::optional<std::string>& filenameOut,
    const std::optional<std::string>& settings,
    const std::optional<double>& marginWidth)
{
    SCIP* scip;
    SCIP_CALL(SCIPcreate(&scip));

    SCIP_CALL(SCIPincludeDefaultPlugins(scip));

    SCIP_CALL(addRSParameters(scip));

    if (settings.has_value())
    {
        auto settings_c = settings->c_str();
        if (!SCIPfileExists(settings_c))
        {
            SCIPerrorMessage("Setting file <%s> does not exist.\n\n", settings_c);
        }
        else
        {
            SCIPinfoMessage(scip, nullptr, "Reading parameters from <%s>.\n\n", settings_c);
            SCIP_CALL(SCIPreadParams(scip, settings_c));
        }
    }

    SCIPinfoMessage(scip, nullptr, "Changed settings:\n");
    SCIP_CALL(SCIPwriteParams(scip, nullptr, false, true));
    SCIPinfoMessage(scip, nullptr, "\n");

    int model_type;
    SCIP_CALL(SCIPgetIntParam(scip, "rs/type", &model_type));

    nlohmann::json result;
    if (model_type == MODEL_BOOLEAN)
    {
        auto data = MasterProblemData<BooleanClause>::load_csv(filenameX, filenameY);
        auto mp_boolean = MasterProblem<BooleanClause>(std::move(data), C);
        mp_boolean.CreateRSModel(scip);
        mp_boolean.Solve(scip);
        result = mp_boolean.ToJson(scip);
        mp_boolean.Free(scip);
    }
    else
    {
        int negation_mode;
        SCIPgetIntParam(scip, "rs/threshold/negation", &negation_mode);

        auto mw = marginWidth.value_or(0.0);
        auto data = MasterProblemData<ThresholdClause>::load_csv(filenameX, filenameY, negation_mode);
        auto mp_threshold = MasterProblem<ThresholdClause>(std::move(data), C, mw);
        mp_threshold.CreateRSModel(scip);
        mp_threshold.Solve(scip);
        result = mp_threshold.ToJson(scip);
        mp_threshold.Free(scip);
    }

    auto result_dump = result.dump(4);
    std::cout << result_dump;

    if (filenameOut)
    {
        std::ofstream(filenameOut.value()) << result_dump;
    }

    return SCIP_OKAY;
};


int main(int argc, char* argv[])
{
    std::cout << "Program name: " << argv[0] << std::endl;

    auto args = Utils::parse_args(argc, argv,
                                  {"X", "y", "C", "O", "S", "b"});

    if (!args.contains("X"))
        throw std::invalid_argument("Must provide an X file.");
    if (!args.contains("y"))
        throw std::invalid_argument("Must provide a y file.");
    if (!args.contains("C"))
        throw std::invalid_argument("Must provide C.");
    if (!Utils::is_integer(args["C"]))
        throw std::invalid_argument("C must be an integer.");

    const int C = Utils::integer(args["C"]);

    std::cout << "'X' file: " << args.at("X") << std::endl;
    std::cout << "'y' file: " << args.at("y") << std::endl;
    std::cout << "'C': " << args.at("C") << std::endl;
    if (args.contains("O"))
    {
        std::cout << "Output file: " << args.at("O") << std::endl;
    }
    if (args.contains("S"))
    {
        std::cout << "Settings file: " << args.at("S") << std::endl;
    }
    if (args.contains("b"))
    {
        if (!Utils::is_decimal(args["b"]) || Utils::decimal(args["b"]) < 0)
            throw std::invalid_argument("b must be a non-negative number.");
        std::cout << "'b': " << args.at("b") << std::endl;
    }

    computeRs(args.at("X"),
               args.at("y"),
               C,
               args.contains("O") ? std::optional{args.at("O")} : std::nullopt,
               args.contains("S") ? std::optional{args.at("S")} : std::nullopt,
               args.contains("b") ? std::optional{Utils::decimal(args.at("b"))} : std::nullopt
    );
}
