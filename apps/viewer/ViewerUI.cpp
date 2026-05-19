#include "ViewerUI.h"

#include <imgui.h>

#include "examples.h"

namespace D {

// ── Helpers ──────────────────────────────────────────────────────────────────

static std::string wordToString(const Word& word) {
  std::string s;
  s.reserve(word.size());
  for (const auto& sym : word) s += sym.letter;
  return s;
}

static std::string wordToParametricString(const Word& word) {
  std::string s;
  for (const auto& sym : word) {
    s += sym.letter;
    if (!sym.params.empty()) {
      s += '(';
      for (size_t i = 0; i < sym.params.size(); ++i) {
        if (i > 0) s += ',';
        float v = std::visit([](auto x){ return static_cast<float>(x); }, sym.params[i]);
        s += std::to_string(v);
      }
      s += ')';
    }
  }
  return s;
}

static Word stringToWord(std::string_view s) {
  Word w;
  w.reserve(s.size());
  for (char c : s) w.emplace_back(c);
  return w;
}

// ── ViewerUI static helpers ───────────────────────────────────────────────────

ViewerUI::RuleEdit ViewerUI::ruleToEdit(const Rule& rule) {
  ViewerUI::RuleEdit re;
  re.predecessor[0] = rule.predecessor;
  re.probability    = rule.probability;
  if (rule.leftContext)  re.leftContext[0]  = *rule.leftContext;
  if (rule.rightContext) re.rightContext[0] = *rule.rightContext;
  auto str  = wordToString(rule.successor({}));
  auto slen = std::min(str.size(), sizeof(re.successor) - 1);
  std::copy_n(str.begin(), slen, re.successor);
  return re;
}

// ── ViewerUI ─────────────────────────────────────────────────────────────────

ViewerUI::ViewerUI(OpenGLRenderer& renderer)
    : m_renderer(renderer),
      m_grammar(examples::binaryTree()),
      m_turtle(m_grammar.angle),
      m_angleOverride(m_grammar.angle),
      m_stepLen(m_turtle.step()) {
  m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
  m_mesh = buildMesh(m_turtle, m_algo->getStructure());

  auto axiomStr = wordToString(m_grammar.axiom);
  auto len = std::min(axiomStr.size(), sizeof(m_axiomBuf) - 1);
  std::copy_n(axiomStr.begin(), len, m_axiomBuf);

  m_ruleEdits.reserve(m_grammar.rules.size());
  for (const auto& rule : m_grammar.rules)
    m_ruleEdits.push_back(ruleToEdit(rule));
}

void ViewerUI::draw() {
  m_renderer.setClearColor(m_bgColor);

  float nextY = 10.f;
  drawControlPanel(nextY);
  drawGrammarPanel(nextY);
  drawSettingsPanel(nextY);
}

void ViewerUI::applyParametricGrammar() {
  using PRule = ParametricLSystemAlgorithm::PRule;

  Word axiom = detail::parseParametricWord(m_paramAxiomBuf, {});
  std::vector<PRule> rules;
  for (const auto& pe : m_paramRuleEdits) {
    if (pe.predecessor[0] == '\0') continue;
    PRule r;
    r.predecessor = pe.predecessor[0];
    // split param names by comma
    std::string names = pe.paramNames;
    std::string cur;
    for (char c : names) {
      if (c == ',') { if (!cur.empty()) { r.paramNames.push_back(cur); cur.clear(); } }
      else if (c != ' ') cur += c;
    }
    if (!cur.empty()) r.paramNames.push_back(cur);
    r.successorExpr = pe.successorExpr;
    rules.push_back(std::move(r));
  }
  auto palgo = std::make_unique<ParametricLSystemAlgorithm>(
      std::move(axiom), std::move(rules), m_angleOverride);
  std::map<std::string, float> globals;
  for (const auto& pd : m_paramDefs)
    if (pd.name[0] != '\0') globals[pd.name] = pd.value;
  palgo->setGlobalParams(std::move(globals));
  m_algo = std::move(palgo);
  rebuildMesh();
}

void ViewerUI::rebuildMesh() {
  m_turtle.setAngle(m_angleOverride);
  m_turtle.setStep(m_stepLen);
  m_turtle.setFlowerRadius(m_flowerRadius);
  m_mesh = buildMesh(m_turtle, m_algo->getStructure());
  m_flowerMesh = m_turtle.lastFlowerMesh();
}

void ViewerUI::switchAlgo(AlgoType type) {
  m_algoType = type;

  switch (type) {
    case AlgoType::D0L:
      m_grammar = examples::binaryTree();
      m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::Stochastic:
      m_grammar = examples::stochasticPlant();
      m_algo = std::make_unique<StochasticLSystemAlgorithm>(m_grammar, static_cast<uint32_t>(m_seed));
      break;
    case AlgoType::ContextSensitive:
      m_grammar = examples::contextSensitivePlant();
      m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::ContextSensitive2L:
      m_grammar = examples::contextSensitive2LPlant();
      m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::ContextSensitiveFlower:
      m_grammar = examples::contextSensitiveFlower();
      m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::Parametric: {
      auto pa = examples::parametricTree();
      m_angleOverride = pa.angle();
      m_turtle.setAngle(m_angleOverride);
      // populate parametric UI state
      {
        auto axiomStr = wordToParametricString(pa.axiomWord());
        auto len = std::min(axiomStr.size(), sizeof(m_paramAxiomBuf) - 1);
        std::fill(std::begin(m_paramAxiomBuf), std::end(m_paramAxiomBuf), '\0');
        std::copy_n(axiomStr.begin(), len, m_paramAxiomBuf);
      }
      m_paramRuleEdits.clear();
      for (const auto& r : pa.prules()) {
        ParametricEdit pe;
        pe.predecessor[0] = r.predecessor;
        std::string names;
        for (size_t k = 0; k < r.paramNames.size(); ++k) {
          if (k) names += ',';
          names += r.paramNames[k];
        }
        auto nlen = std::min(names.size(), sizeof(pe.paramNames) - 1);
        std::copy_n(names.begin(), nlen, pe.paramNames);
        auto slen = std::min(r.successorExpr.size(), sizeof(pe.successorExpr) - 1);
        std::copy_n(r.successorExpr.begin(), slen, pe.successorExpr);
        m_paramRuleEdits.push_back(pe);
      }
      m_paramDefs.clear();
      for (const auto& [name, val] : pa.globalParams()) {
        ParamDef pd;
        auto nlen = std::min(name.size(), sizeof(pd.name) - 1);
        std::copy_n(name.begin(), nlen, pd.name);
        pd.value = val;
        m_paramDefs.push_back(pd);
      }
      m_algo = std::make_unique<ParametricLSystemAlgorithm>(pa);
      // skip the generic grammar rebuild below
      rebuildMesh();
      return;
    }
  }

  m_angleOverride = m_grammar.angle;
  m_turtle.setAngle(m_angleOverride);

  auto axiomStr = wordToString(m_grammar.axiom);
  auto len = std::min(axiomStr.size(), sizeof(m_axiomBuf) - 1);
  std::fill(std::begin(m_axiomBuf), std::end(m_axiomBuf), '\0');
  std::copy_n(axiomStr.begin(), len, m_axiomBuf);

  m_ruleEdits.clear();
  for (const auto& rule : m_grammar.rules)
    m_ruleEdits.push_back(ruleToEdit(rule));

  rebuildMesh();
}

void ViewerUI::applyGrammar() {
  m_grammar.axiom = stringToWord(m_axiomBuf);
  m_grammar.angle = m_angleOverride;
  m_grammar.rules.clear();
  for (const auto& re : m_ruleEdits) {
    if (re.predecessor[0] == '\0') continue;
    Rule rule;
    rule.predecessor = re.predecessor[0];
    rule.probability = re.probability;
    if (re.leftContext[0]  != '\0') rule.leftContext  = re.leftContext[0];
    if (re.rightContext[0] != '\0') rule.rightContext = re.rightContext[0];
    std::string succStr = re.successor;
    rule.successor = [succStr](std::span<const ParamValue>) -> Word {
      return stringToWord(succStr);
    };
    m_grammar.rules.push_back(std::move(rule));
  }

  if (m_algoType == AlgoType::Parametric) {
    applyParametricGrammar();
    return;
  }

  switch (m_algoType) {
    case AlgoType::Stochastic:
      m_algo = std::make_unique<StochasticLSystemAlgorithm>(m_grammar, static_cast<uint32_t>(m_seed));
      break;
    default:
      m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
  }

  rebuildMesh();
}

void ViewerUI::drawControlPanel(float& nextY) {
  constexpr float kW = 340.f;
  ImGui::SetNextWindowPos({10, nextY}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({kW, 0}, ImGuiCond_Always);
  ImGui::Begin("L-System", nullptr, ImGuiWindowFlags_NoResize);

  // Algorithm type combo
  static const char* kAlgoNames[] = {
      "D0L (deterministic)", "Stochastic",
      "Context-sensitive (1L)", "Context-sensitive (2L)",
      "Parametric", "Context-sensitive (flower K)"};
  int currentItem = static_cast<int>(m_algoType);
  ImGui::SetNextItemWidth(-1);
  if (ImGui::Combo("##algo", &currentItem, kAlgoNames, 6)) {
    switchAlgo(static_cast<AlgoType>(currentItem));
  }

  ImGui::Separator();

  ImGui::Text("Generation: %d", m_algo->generation());
  ImGui::Text("Symbols:    %zu",
              std::get<StringStructure>(m_algo->getStructure()).derivation.size());

  ImGui::Separator();

  if (m_algoType == AlgoType::Stochastic) {
    if (ImGui::InputInt("Seed", &m_seed)) {
      if (m_seed < 0) m_seed = 0;
      static_cast<StochasticLSystemAlgorithm*>(m_algo.get())
          ->setSeed(static_cast<uint32_t>(m_seed));
      rebuildMesh();
    }
  }

  if (ImGui::SliderFloat("Angle",    &m_angleOverride, 5.f,    90.f))   rebuildMesh();
  if (ImGui::SliderFloat("Step len", &m_stepLen,       0.1f,   10.f))   rebuildMesh();
  if (ImGui::SliderFloat("Zoom",     &m_zoom,          0.1f,   20.f))   m_renderer.setZoom(m_zoom);
  if (ImGui::SliderFloat("Pan X",    &m_panX,         -100.f, 100.f))   m_renderer.setPan(m_panX, m_panY);
  if (ImGui::SliderFloat("Pan Y",    &m_panY,         -100.f, 100.f))   m_renderer.setPan(m_panX, m_panY);

  ImGui::Separator();
  if (ImGui::Button("Step")) { m_algo->step(); rebuildMesh(); }
  ImGui::SameLine();
  if (ImGui::Button("Reset")) { m_algo->reset(); rebuildMesh(); }

  nextY += ImGui::GetWindowHeight() + 5.f;
  ImGui::End();
}

void ViewerUI::drawGrammarPanel(float& nextY) {
  constexpr float kW = 340.f;
  ImGui::SetNextWindowPos({10, nextY}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({kW, 0}, ImGuiCond_Always);
  ImGui::Begin("Grammar", nullptr, ImGuiWindowFlags_NoResize);

  ImGui::SetNextItemWidth(-1);
  ImGui::InputText("##axiom", m_axiomBuf, sizeof(m_axiomBuf));
  ImGui::SameLine(0, 0);
  ImGui::Text(" Axiom");

  ImGui::Separator();

  const bool isStochastic  = (m_algoType == AlgoType::Stochastic);
  const bool isContext     = (m_algoType == AlgoType::ContextSensitive ||
                              m_algoType == AlgoType::ContextSensitive2L ||
                              m_algoType == AlgoType::ContextSensitiveFlower);
  const bool isParametric  = (m_algoType == AlgoType::Parametric);

  if (isParametric) {
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##paxiom", m_paramAxiomBuf, sizeof(m_paramAxiomBuf));
    ImGui::SameLine(0,0); ImGui::Text(" Axiom");
    ImGui::Separator();
    for (int i = 0; i < static_cast<int>(m_paramRuleEdits.size()); ++i) {
      ImGui::PushID(i);
      ImGui::SetNextItemWidth(18); ImGui::InputText("##pp", m_paramRuleEdits[i].predecessor, 2);
      ImGui::SameLine(); ImGui::TextUnformatted("(");
      ImGui::SameLine(); ImGui::SetNextItemWidth(50);
      ImGui::InputText("##pn", m_paramRuleEdits[i].paramNames, sizeof(m_paramRuleEdits[i].paramNames));
      ImGui::SameLine(); ImGui::TextUnformatted(") ->");
      ImGui::SameLine(); ImGui::SetNextItemWidth(170);
      ImGui::InputText("##pe", m_paramRuleEdits[i].successorExpr, sizeof(m_paramRuleEdits[i].successorExpr));
      ImGui::SameLine();
      if (ImGui::Button("X")) { m_paramRuleEdits.erase(m_paramRuleEdits.begin() + i); --i; }
      ImGui::PopID();
    }
    if (ImGui::Button("+ Rule")) m_paramRuleEdits.emplace_back();
    ImGui::Separator();
    ImGui::TextUnformatted("Global params:");
    for (int i = 0; i < static_cast<int>(m_paramDefs.size()); ++i) {
      ImGui::PushID(100 + i);
      ImGui::SetNextItemWidth(40);
      ImGui::InputText("##pname", m_paramDefs[i].name, sizeof(m_paramDefs[i].name));
      ImGui::SameLine();
      ImGui::SetNextItemWidth(180);
      if (ImGui::SliderFloat("##pval", &m_paramDefs[i].value, 0.01f, 2.f)) {
        auto* pa = static_cast<ParametricLSystemAlgorithm*>(m_algo.get());
        std::map<std::string, float> globals;
        for (const auto& pd : m_paramDefs)
          if (pd.name[0] != '\0') globals[pd.name] = pd.value;
        pa->setGlobalParams(std::move(globals));
        rebuildMesh();
      }
      ImGui::SameLine();
      if (ImGui::Button("X")) { m_paramDefs.erase(m_paramDefs.begin() + i); --i; }
      ImGui::PopID();
    }
    if (ImGui::Button("+ Param")) m_paramDefs.emplace_back();
    ImGui::Separator();
    if (ImGui::Button("Apply")) applyGrammar();
    nextY += ImGui::GetWindowHeight() + 5.f;
    ImGui::End();
    return;
  }

  for (int i = 0; i < static_cast<int>(m_ruleEdits.size()); ++i) {
    ImGui::PushID(i);
    if (isContext) {
      ImGui::SetNextItemWidth(22); ImGui::InputText("##lc", m_ruleEdits[i].leftContext, 2);
      ImGui::SameLine(); ImGui::TextUnformatted("<");
      ImGui::SameLine();
    }
    ImGui::SetNextItemWidth(22);
    ImGui::InputText("##pred", m_ruleEdits[i].predecessor, 2);
    if (isContext) {
      ImGui::SameLine(); ImGui::TextUnformatted(">");
      ImGui::SameLine(); ImGui::SetNextItemWidth(22);
      ImGui::InputText("##rc", m_ruleEdits[i].rightContext, 2);
    }
    ImGui::SameLine(); ImGui::TextUnformatted("->");
    ImGui::SameLine();
    float succW = isStochastic ? 140.f : (isContext ? 160.f : 230.f);
    ImGui::SetNextItemWidth(succW);
    ImGui::InputText("##succ", m_ruleEdits[i].successor, sizeof(m_ruleEdits[i].successor));
    if (isStochastic) {
      ImGui::SameLine();
      ImGui::SetNextItemWidth(44.f);
      ImGui::InputFloat("##prob", &m_ruleEdits[i].probability, 0.f, 0.f, "%.2f");
    }
    ImGui::SameLine();
    if (ImGui::Button("X")) { m_ruleEdits.erase(m_ruleEdits.begin() + i); --i; }
    ImGui::PopID();
  }

  if (ImGui::Button("+ Rule")) m_ruleEdits.emplace_back();
  ImGui::Separator();
  if (ImGui::Button("Apply")) applyGrammar();

  nextY += ImGui::GetWindowHeight() + 5.f;
  ImGui::End();
}

void ViewerUI::drawSettingsPanel(float& nextY) {
  constexpr float kW = 340.f;
  ImGui::SetNextWindowPos({10, nextY}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({kW, 0}, ImGuiCond_Always);
  ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

  ImGui::ColorEdit3("Line color",       reinterpret_cast<float*>(&m_lineColor));
  ImGui::ColorEdit3("Background color", reinterpret_cast<float*>(&m_bgColor));
  ImGui::ColorEdit3("Flower color",     reinterpret_cast<float*>(&m_flowerColor));
  if (ImGui::SliderFloat("Flower radius", &m_flowerRadius, 0.05f, 3.f)) rebuildMesh();

  nextY += ImGui::GetWindowHeight() + 5.f;
  ImGui::End();
}

}  // namespace D
