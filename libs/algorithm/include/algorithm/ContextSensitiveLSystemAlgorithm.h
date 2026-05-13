#pragma once

// Context-sensitive rules are encoded in LSystemGrammar::Rule::leftContext /
// rightContext and are already handled by LSystemGrammar::derive().
// No separate derivation logic is needed; this alias exists for clarity.

#include "algorithm/D0LSystemAlgorithm.h"

namespace D {
using ContextSensitiveLSystemAlgorithm = D0LSystemAlgorithm;
}  // namespace D
