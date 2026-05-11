#pragma once

#include "structure/StringStructure.h"
#include "structure/TreeGraph.h"

namespace D {

using PlantStructure = std::variant<StringStructure, TreeGraph>;

}  // namespace D
