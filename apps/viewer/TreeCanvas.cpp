// glad MUST precede all Qt/system OpenGL headers (QOpenGLWidget pulls in <GL/gl.h>)
#include <glad/gl.h>

#include "TreeCanvas.h"

#include <QOpenGLContext>
#include <glm/glm.hpp>

#include "examples.h"
#include "structure/StringStructure.h"

// GLADloadfunc = GLADapiproc(*)(const char*), GLADapiproc = void(*)().
// QFunctionPointer = void(*)() — identical type, no cast needed.
static GLADapiproc loadGLFunc(const char* name) {
  return QOpenGLContext::currentContext()->getProcAddress(name);
}

namespace D {

// ── Static helpers ────────────────────────────────────────────────────────────

QColor TreeCanvas::toQColor(glm::vec4 c) {
  return QColor::fromRgbF(
      static_cast<float>(c.r),
      static_cast<float>(c.g),
      static_cast<float>(c.b));
}

glm::vec4 TreeCanvas::toGlm(QColor c) {
  return {static_cast<float>(c.redF()),
          static_cast<float>(c.greenF()),
          static_cast<float>(c.blueF()),
          1.f};
}

std::string TreeCanvas::wordToString(const Word& word) {
  std::string s;
  s.reserve(word.size());
  for (const auto& sym : word) s += sym.letter;
  return s;
}

std::string TreeCanvas::wordToParametricString(const Word& word) {
  std::string s;
  for (const auto& sym : word) {
    s += sym.letter;
    if (!sym.params.empty()) {
      s += '(';
      for (size_t i = 0; i < sym.params.size(); ++i) {
        if (i > 0) s += ',';
        float v = std::visit([](auto x) { return static_cast<float>(x); }, sym.params[i]);
        s += std::to_string(v);
      }
      s += ')';
    }
  }
  return s;
}

Word TreeCanvas::stringToWord(std::string_view s) {
  Word w;
  w.reserve(s.size());
  for (char c : s) w.emplace_back(c);
  return w;
}

TreeCanvas::RuleEdit TreeCanvas::ruleToEdit(const Rule& rule) {
  RuleEdit re;
  re.predecessor[0] = rule.predecessor;
  re.probability    = rule.probability;
  if (rule.leftContext)  re.leftContext[0]  = *rule.leftContext;
  if (rule.rightContext) re.rightContext[0] = *rule.rightContext;
  auto str  = wordToString(rule.successor({}));
  auto slen = std::min(str.size(), sizeof(re.successor) - 1);
  std::copy_n(str.begin(), slen, re.successor);
  return re;
}

// ── Construction ──────────────────────────────────────────────────────────────

TreeCanvas::TreeCanvas(QWidget* parent) : QOpenGLWidget(parent), m_turtle(25.f) {
  initLSystem();

  QSurfaceFormat fmt;
  fmt.setVersion(4, 6);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  setFormat(fmt);
}

void TreeCanvas::initLSystem() {
  m_grammar     = examples::binaryTree();
  m_algoType    = AlgoType::D0L;
  m_angleOverride = m_grammar.angle;
  m_stepLen     = m_turtle.step();
  m_turtle.setAngle(m_angleOverride);
  m_algo        = std::make_unique<D0LSystemAlgorithm>(m_grammar);
  m_mesh        = buildMesh(m_turtle, m_algo->getStructure());
  populateGrammarBuffers();
}

void TreeCanvas::populateGrammarBuffers() {
  auto axiomStr = wordToString(m_grammar.axiom);
  auto len = std::min(axiomStr.size(), sizeof(m_axiomBuf) - 1);
  std::fill(std::begin(m_axiomBuf), std::end(m_axiomBuf), '\0');
  std::copy_n(axiomStr.begin(), len, m_axiomBuf);

  m_ruleEdits.clear();
  for (const auto& rule : m_grammar.rules)
    m_ruleEdits.push_back(ruleToEdit(rule));
}

// ── QOpenGLWidget ─────────────────────────────────────────────────────────────

void TreeCanvas::initializeGL() {
  gladLoadGL(loadGLFunc);
  m_renderer = std::make_unique<OpenGLRenderer>(
      static_cast<uint32_t>(width()),
      static_cast<uint32_t>(height()));
}

void TreeCanvas::resizeGL(int w, int h) {
  if (m_renderer)
    m_renderer->resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
}

void TreeCanvas::paintGL() {
  if (!m_renderer) return;
  m_renderer->setClearColor(m_bgColor);
  m_renderer->beginFrame();
  m_renderer->submit({&m_mesh,       glm::mat4(1.f), m_lineColor});
  m_renderer->submit({&m_flowerMesh, glm::mat4(1.f), m_flowerColor});
  m_renderer->endFrame();
}

// ── Accessors ─────────────────────────────────────────────────────────────────

int TreeCanvas::generation() const {
  return m_algo ? m_algo->generation() : 0;
}

int TreeCanvas::symbolCount() const {
  if (!m_algo) return 0;
  return static_cast<int>(
      std::get<StringStructure>(m_algo->getStructure()).derivation.size());
}

QColor TreeCanvas::lineColor()   const { return toQColor(m_lineColor); }
QColor TreeCanvas::flowerColor() const { return toQColor(m_flowerColor); }
QColor TreeCanvas::bgColor()     const { return toQColor(m_bgColor); }

// ── Mesh rebuild ──────────────────────────────────────────────────────────────

void TreeCanvas::rebuildMesh() {
  m_turtle.setAngle(m_angleOverride);
  m_turtle.setStep(m_stepLen);
  m_turtle.setFlowerRadius(m_flowerRadius);
  m_mesh      = buildMesh(m_turtle, m_algo->getStructure());
  m_flowerMesh = m_turtle.lastFlowerMesh();
  if (m_renderer) {
    m_renderer->setZoom(m_zoom);
    m_renderer->setPan(m_panX, m_panY);
  }
  emit stateChanged(generation(), symbolCount());
  update();
}

// ── Slots — generation control ────────────────────────────────────────────────

void TreeCanvas::stepGeneration() {
  m_algo->step();
  rebuildMesh();
}

void TreeCanvas::resetGeneration() {
  m_algo->reset();
  rebuildMesh();
}

void TreeCanvas::switchAlgo(int typeInt) {
  m_algoType = static_cast<AlgoType>(typeInt);

  switch (m_algoType) {
    case AlgoType::D0L:
      m_grammar = examples::binaryTree();
      m_algo    = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::Stochastic:
      m_grammar = examples::stochasticPlant();
      m_algo    = std::make_unique<StochasticLSystemAlgorithm>(
          m_grammar, static_cast<uint32_t>(m_seed));
      break;
    case AlgoType::ContextSensitive:
      m_grammar = examples::contextSensitivePlant();
      m_algo    = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::ContextSensitive2L:
      m_grammar = examples::contextSensitive2LPlant();
      m_algo    = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::ContextSensitiveFlower:
      m_grammar = examples::contextSensitiveFlower();
      m_algo    = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::Parametric: {
      auto pa       = examples::parametricTree();
      m_angleOverride = pa.angle();
      m_turtle.setAngle(m_angleOverride);

      auto axiomStr = wordToParametricString(pa.axiomWord());
      auto len = std::min(axiomStr.size(), sizeof(m_paramAxiomBuf) - 1);
      std::fill(std::begin(m_paramAxiomBuf), std::end(m_paramAxiomBuf), '\0');
      std::copy_n(axiomStr.begin(), len, m_paramAxiomBuf);

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
      rebuildMesh();
      emit algoSwitched(typeInt);
      return;
    }
  }

  m_angleOverride = m_grammar.angle;
  m_turtle.setAngle(m_angleOverride);
  populateGrammarBuffers();
  rebuildMesh();
  emit algoSwitched(typeInt);
}

// ── Slots — visual params ─────────────────────────────────────────────────────

void TreeCanvas::setAngle(double deg) {
  m_angleOverride = static_cast<float>(deg);
  rebuildMesh();
}

void TreeCanvas::setStepLen(double len) {
  m_stepLen = static_cast<float>(len);
  rebuildMesh();
}

void TreeCanvas::setZoom(double z) {
  m_zoom = static_cast<float>(z);
  if (m_renderer) m_renderer->setZoom(m_zoom);
  update();
}

void TreeCanvas::setPanX(double x) {
  m_panX = static_cast<float>(x);
  if (m_renderer) m_renderer->setPan(m_panX, m_panY);
  update();
}

void TreeCanvas::setPanY(double y) {
  m_panY = static_cast<float>(y);
  if (m_renderer) m_renderer->setPan(m_panX, m_panY);
  update();
}

void TreeCanvas::setSeed(int seed) {
  m_seed = seed;
  if (m_algoType == AlgoType::Stochastic)
    static_cast<StochasticLSystemAlgorithm*>(m_algo.get())
        ->setSeed(static_cast<uint32_t>(m_seed));
  rebuildMesh();
}

void TreeCanvas::setLineColor(QColor c) {
  m_lineColor = toGlm(c);
  update();
}

void TreeCanvas::setBgColor(QColor c) {
  m_bgColor = toGlm(c);
  update();
}

void TreeCanvas::setFlowerColor(QColor c) {
  m_flowerColor = toGlm(c);
  update();
}

void TreeCanvas::setFlowerRadius(double r) {
  m_flowerRadius = static_cast<float>(r);
  rebuildMesh();
}

// ── Slots — grammar apply ─────────────────────────────────────────────────────

void TreeCanvas::applyGrammar(const std::string& axiom,
                               const std::vector<RuleEdit>& rules) {
  m_grammar.axiom = stringToWord(axiom);
  m_grammar.angle = m_angleOverride;
  m_grammar.rules.clear();
  for (const auto& re : rules) {
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

  if (m_algoType == AlgoType::Stochastic)
    m_algo = std::make_unique<StochasticLSystemAlgorithm>(
        m_grammar, static_cast<uint32_t>(m_seed));
  else
    m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);

  rebuildMesh();
}

void TreeCanvas::applyParametricGrammar(const std::string& axiom,
                                         const std::vector<ParametricEdit>& rules,
                                         const std::vector<ParamDef>& params) {
  using PRule = ParametricLSystemAlgorithm::PRule;

  Word pAxiom = detail::parseParametricWord(axiom, {});
  std::vector<PRule> prules;
  for (const auto& pe : rules) {
    if (pe.predecessor[0] == '\0') continue;
    PRule r;
    r.predecessor = pe.predecessor[0];
    std::string names = pe.paramNames;
    std::string cur;
    for (char c : names) {
      if (c == ',') { if (!cur.empty()) { r.paramNames.push_back(cur); cur.clear(); } }
      else if (c != ' ') cur += c;
    }
    if (!cur.empty()) r.paramNames.push_back(cur);
    r.successorExpr = pe.successorExpr;
    prules.push_back(std::move(r));
  }

  auto palgo = std::make_unique<ParametricLSystemAlgorithm>(
      std::move(pAxiom), std::move(prules), m_angleOverride);

  std::map<std::string, float> globals;
  for (const auto& pd : params)
    if (pd.name[0] != '\0') globals[pd.name] = pd.value;
  palgo->setGlobalParams(std::move(globals));

  m_algo = std::move(palgo);
  rebuildMesh();
}

}  // namespace D
