#pragma once

#include <optional>
#include <string>
#include <vector>

#include <QString>

namespace D {

class TreeCanvas;

// A self-contained snapshot of a plant's STRUCTURE: algorithm + grammar +
// the geometric parameters that are part of the L-system definition (angle,
// step, seed). Appearance (colors, flower radius, turtle symbol remapping) is
// deliberately NOT stored here — that lives in user Preferences (QSettings),
// not in the document. Serialized as JSON so the schema can grow over time.
//
// Kept free of TreeCanvas nested types so this header does not depend on
// TreeCanvas.h (TreeCanvas.h includes this one for loadPlant()).
struct PlantDoc {
  int algoType = 0;  // TreeCanvas::AlgoType
  double angle = 25.0;
  double step = 1.0;
  int seed = 42;

  // Normal (non-parametric) grammar.
  std::string axiom;
  struct Rule {
    char pred = '\0';
    std::string left;
    std::string right;
    std::string succ;
    float prob = 1.f;
  };
  std::vector<Rule> rules;
  std::string ignore;
  bool hasPush = false;
  bool hasPop = false;
  char push = '[';
  char pop = ']';
  bool includeSiblings = false;
  bool strict = false;

  // Parametric grammar.
  std::string paramAxiom;
  struct PRule {
    char pred = '\0';
    std::string params;
    std::string expr;
  };
  std::vector<PRule> paramRules;
  struct PDef {
    std::string name;
    float value = 0.f;
  };
  std::vector<PDef> paramDefs;
};

// Serialize the current canvas structure to the .dt JSON format.
QString serializePlant(const TreeCanvas& canvas);

// Parse .dt JSON into a PlantDoc. Returns nullopt on invalid JSON; unknown keys
// are ignored and missing fields keep their defaults.
std::optional<PlantDoc> parsePlant(const QString& text, QString* error);

// Convenience file wrappers around serialize/parse.
bool savePlantFile(const TreeCanvas& canvas, const QString& path, QString* error);
bool loadPlantFile(TreeCanvas& canvas, const QString& path, QString* error);

}  // namespace D
