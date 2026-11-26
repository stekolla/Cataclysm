#pragma once

#include "artifact.h"

#include <string>
#include <vector>

namespace cata::detail {
auto artifact_effect_names(const std::vector<art_effect_passive>& effects)
    -> std::vector<std::string>;
auto artifact_effect_names(const std::vector<art_effect_active>& effects)
    -> std::vector<std::string>;
auto artifact_effect_descriptions(const std::vector<art_effect_passive>& effects)
    -> std::vector<std::string>;
auto artifact_effect_descriptions(const std::vector<art_effect_active>& effects)
    -> std::vector<std::string>;
auto artifact_charge_description(const art_charge charge) -> std::string;
auto artifact_charge_req_description(const art_charge_req charge_req) -> std::string;
} // namespace cata::detail
