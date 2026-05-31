// clang-format off
#include <glad/gl.h>  // Must precede all Qt/system OpenGL headers (QOpenGLWidget pulls in <GL/gl.h>)
// clang-format on

#include "TreeCanvas.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QWheelEvent>

#include <algorithm>

#include <spdlog/spdlog.h>

#include "examples.h"

namespace D {

namespace {

// GLADloadfunc = GLADapiproc(*)(const char*), GLADapiproc = void(*)().
// QFunctionPointer = void(*)() — identical type, no cast needed.
static GLADapiproc loadGLFunc(const char* name) {
  return QOpenGLContext::currentContext()->getProcAddress(name);
}

QColor toQColor(Vec4 c) {
  return QColor::fromRgbF(static_cast<float>(c.r), static_cast<float>(c.g),
                          static_cast<float>(c.b));
}

Vec4 toGlm(QColor c) {
  return {static_cast<float>(c.redF()), static_cast<float>(c.greenF()),
          static_cast<float>(c.blueF()), 1.f};
}

std::string wordToParametricString(const Word& word) {
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

TreeCanvas::RuleEdit ruleToEdit(const Rule& rule) {
  TreeCanvas::RuleEdit re;
  re.predecessor[0] = rule.predecessor;
  re.probability = rule.probability;
  if (rule.leftContext) re.leftContext[0] = *rule.leftContext;
  if (rule.rightContext) re.rightContext[0] = *rule.rightContext;
  auto str = D::str(rule.successor({}));
  auto slen = std::min(str.size(), sizeof(re.successor) - 1);
  std::copy_n(str.begin(), slen, re.successor);
  return re;
}

}  // namespace

// ── Construction ──────────────────────────────────────────────────────────────

TreeCanvas::TreeCanvas(QWidget* parent /* = nullptr */, Qt::WindowFlags f /* = Qt::WindowFlags() */)
    : QOpenGLWidget(parent, f), m_turtle(25.f) {
  setFocusPolicy(Qt::StrongFocus);
  initLSystem();

  QSurfaceFormat fmt;
  fmt.setVersion(4, 6);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  setFormat(fmt);
}

void TreeCanvas::initLSystem() {
  m_grammar = examples::binaryTree();
  m_algoType = AlgoType::D0L;
  m_turtle.setAngle(m_angle);
  m_turtle.setStep(m_stepLen);
  m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
  m_mesh = m_turtle.build(m_algo->current());
  populateGrammarBuffers();
}

void TreeCanvas::populateGrammarBuffers() {
  auto axiomStr = D::str(m_grammar.axiom);
  auto len = std::min(axiomStr.size(), sizeof(m_axiomBuf) - 1);
  std::fill(std::begin(m_axiomBuf), std::end(m_axiomBuf), '\0');
  std::copy_n(axiomStr.begin(), len, m_axiomBuf);

  m_ruleEdits.clear();
  for (const auto& rule : m_grammar.rules) m_ruleEdits.push_back(ruleToEdit(rule));
}

// ── QOpenGLWidget ─────────────────────────────────────────────────────────────

void TreeCanvas::initializeGL() {
  gladLoadGL(loadGLFunc);
  m_renderer = std::make_unique<OpenGLRenderer>(static_cast<uint32_t>(width()),
                                                static_cast<uint32_t>(height()));
}

void TreeCanvas::resizeGL(int w, int h) {
  if (m_renderer) m_renderer->resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
}

void TreeCanvas::paintGL() {
  if (!m_renderer) return;
  m_renderer->setClearColor(m_bgColor);
  m_renderer->beginFrame();
  m_renderer->submit({&m_mesh, Mat4{1.f}, m_lineColor});
  m_renderer->submit({&m_flowerMesh, Mat4{1.f}, m_flowerColor});
  m_renderer->endFrame();
}

// ── Viewport navigation ───────────────────────────────────────────────────────

void TreeCanvas::wheelEvent(QWheelEvent* e) {
  const float factor = e->angleDelta().y() > 0 ? 1.1f : (1.f / 1.1f);
  m_zoom = std::clamp(m_zoom * factor, 0.001f, 50.f);
  if (m_renderer) m_renderer->setZoom(m_zoom);
  emit viewChanged(m_zoom, m_panX, m_panY);
  update();
  e->accept();
}

void TreeCanvas::mousePressEvent(QMouseEvent* e) {
  if (e->button() == Qt::MiddleButton) {
    m_panning = true;
    m_lastMousePos = e->pos();
    setCursor(Qt::SizeAllCursor);
    e->accept();
    return;
  }
  QOpenGLWidget::mousePressEvent(e);
}

void TreeCanvas::mouseMoveEvent(QMouseEvent* e) {
  if (m_panning) {
    const QPoint delta = e->pos() - m_lastMousePos;
    m_lastMousePos = e->pos();
    const float worldPerPixel = 20.f / (m_zoom * static_cast<float>(height()));
    m_panX -= static_cast<float>(delta.x()) * worldPerPixel;
    m_panY += static_cast<float>(delta.y()) * worldPerPixel;
    if (m_renderer) m_renderer->setPan(m_panX, m_panY);
    emit viewChanged(m_zoom, m_panX, m_panY);
    update();
    e->accept();
    return;
  }
  QOpenGLWidget::mouseMoveEvent(e);
}

void TreeCanvas::mouseReleaseEvent(QMouseEvent* e) {
  if (e->button() == Qt::MiddleButton) {
    m_panning = false;
    setCursor(Qt::ArrowCursor);
    e->accept();
    return;
  }
  QOpenGLWidget::mouseReleaseEvent(e);
}

void TreeCanvas::keyPressEvent(QKeyEvent* e) {
  if (e->key() == Qt::Key_F || e->key() == Qt::Key_Home) {
    m_zoom = 1.f;
    m_panX = 0.f;
    m_panY = 0.f;
    if (m_renderer) {
      m_renderer->setZoom(m_zoom);
      m_renderer->setPan(m_panX, m_panY);
    }
    emit viewChanged(m_zoom, m_panX, m_panY);
    update();
    e->accept();
    return;
  }
  QOpenGLWidget::keyPressEvent(e);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

int TreeCanvas::generation() const { return m_algo ? m_algo->generation() : 0; }

TreeCanvas::ContextEdit TreeCanvas::contextEdit() const {
  ContextEdit ce;
  auto ignLen = std::min(m_grammar.ignore.size(), sizeof(ce.ignore) - 1);
  std::copy_n(m_grammar.ignore.begin(), ignLen, ce.ignore);
  if (m_grammar.push) ce.push[0] = *m_grammar.push;
  if (m_grammar.pop) ce.pop[0] = *m_grammar.pop;
  ce.includeSiblings = m_grammar.includeSiblings;
  ce.strictMode = (m_grammar.contextMode == ContextMode::Strict);
  return ce;
}

int TreeCanvas::symbolCount() const {
  if (!m_algo) return 0;
  return static_cast<int>(m_algo->current().size());
}

QColor TreeCanvas::lineColor() const { return toQColor(m_lineColor); }
QColor TreeCanvas::flowerColor() const { return toQColor(m_flowerColor); }
QColor TreeCanvas::bgColor() const { return toQColor(m_bgColor); }

// ── Mesh rebuild ──────────────────────────────────────────────────────────────

void TreeCanvas::rebuildMesh() {
  m_turtle.setAngle(m_angle);
  m_turtle.setStep(m_stepLen);
  m_turtle.setFlowerRadius(m_flowerRadius);
  m_mesh = m_turtle.build(m_algo->current());
  m_flowerMesh = m_turtle.lastFlowerMesh();
  if (m_renderer) {
    m_renderer->setZoom(m_zoom);
    m_renderer->setPan(m_panX, m_panY);
  }
  spdlog::debug("[Mesh] {} vertices, {} segments",
               m_mesh.positions.size(), m_mesh.indices.size() / 2);
  if (spdlog::should_log(spdlog::level::debug)) {
    spdlog::debug("[L-System] String: {}", D::str(m_algo->current()));
  }
  emit stateChanged(generation(), symbolCount());
  update();
}

// ── Slots — generation control ────────────────────────────────────────────────

void TreeCanvas::stepGeneration() {
  m_algo->step();
  rebuildMesh();
  spdlog::info("[L-System] Step -> gen {}, {} symbols", generation(), symbolCount());
}

void TreeCanvas::resetGeneration() {
  m_algo->reset();
  rebuildMesh();
  spdlog::info("[L-System] Reset -> gen {}, {} symbols", generation(), symbolCount());
}

void TreeCanvas::switchAlgo(int typeInt) {
  m_algoType = static_cast<AlgoType>(typeInt);

  switch (m_algoType) {
    case AlgoType::D0L:
      m_grammar = examples::binaryTree();
      m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::Stochastic:
      m_grammar = examples::stochasticPlant();
      m_algo =
          std::make_unique<StochasticLSystemAlgorithm>(m_grammar, static_cast<uint32_t>(m_seed));
      break;
    case AlgoType::ContextSensitive:
      m_grammar = examples::contextSensitivePlant();
      m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::ContextSensitive2L:
      m_grammar = examples::contextSensitive2LPlant();
      m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::ContextFlower:
      m_grammar = examples::contextFlower();
      m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);
      break;
    case AlgoType::Parametric: {
      auto pa = examples::parametricTree();
      m_angle = pa.angle();

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

  m_angle = 25.f;
  populateGrammarBuffers();
  rebuildMesh();
  emit algoSwitched(typeInt);

  static constexpr std::array<const char*, 6> kAlgoNames = {
      "D0L", "Stochastic", "Context-Sensitive 1L",
      "Context-Sensitive 2L", "Parametric", "Context-Sensitive Flower"};
  spdlog::info("[Algo] Switched to {}", kAlgoNames[typeInt]);
}

// ── Slots — visual params ─────────────────────────────────────────────────────

void TreeCanvas::setAngle(double deg) {
  m_angle = static_cast<float>(deg);
  rebuildMesh();
}

void TreeCanvas::setStepLen(double len) {
  m_stepLen = static_cast<float>(len);
  rebuildMesh();
}

void TreeCanvas::setZoom(double z) {
  m_zoom = static_cast<float>(z);
  if (m_renderer) m_renderer->setZoom(m_zoom);
  emit viewChanged(m_zoom, m_panX, m_panY);
  update();
}

void TreeCanvas::setPanX(double x) {
  m_panX = static_cast<float>(x);
  if (m_renderer) m_renderer->setPan(m_panX, m_panY);
  emit viewChanged(m_zoom, m_panX, m_panY);
  update();
}

void TreeCanvas::setPanY(double y) {
  m_panY = static_cast<float>(y);
  if (m_renderer) m_renderer->setPan(m_panX, m_panY);
  emit viewChanged(m_zoom, m_panX, m_panY);
  update();
}

void TreeCanvas::setSeed(int seed) {
  m_seed = seed;
  if (m_algoType == AlgoType::Stochastic)
    static_cast<StochasticLSystemAlgorithm*>(m_algo.get())->setSeed(static_cast<uint32_t>(m_seed));
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

void TreeCanvas::setSymbols(TurtleSymbols s) {
  m_turtle.setSymbols(s);
  rebuildMesh();
}

// ── Slots — grammar apply ─────────────────────────────────────────────────────

void TreeCanvas::applyGrammar(const std::string& axiom, const std::vector<RuleEdit>& rules,
                              const ContextEdit& ctx) {
  m_grammar.axiom = D::w(axiom);
  m_grammar.rules.clear();
  for (const auto& re : rules) {
    if (re.predecessor[0] == '\0') continue;
    Rule rule;
    rule.predecessor = re.predecessor[0];
    rule.probability = re.probability;
    if (re.leftContext[0] != '\0') rule.leftContext = re.leftContext[0];
    if (re.rightContext[0] != '\0') rule.rightContext = re.rightContext[0];
    std::string succStr = re.successor;
    rule.successor = [succStr](std::span<const ParamValue>) -> Word {
      return D::w(succStr);
    };
    m_grammar.rules.push_back(std::move(rule));
  }

  m_grammar.ignore = ctx.ignore;
  m_grammar.push = ctx.push[0] ? std::optional<char>(ctx.push[0]) : std::nullopt;
  m_grammar.pop  = ctx.pop[0]  ? std::optional<char>(ctx.pop[0])  : std::nullopt;
  m_grammar.includeSiblings = ctx.includeSiblings;
  m_grammar.contextMode = ctx.strictMode ? ContextMode::Strict : ContextMode::Biological;

  if (m_algoType == AlgoType::Stochastic)
    m_algo = std::make_unique<StochasticLSystemAlgorithm>(m_grammar, static_cast<uint32_t>(m_seed));
  else
    m_algo = std::make_unique<D0LSystemAlgorithm>(m_grammar);

  rebuildMesh();
  spdlog::info("[Grammar] Applied: axiom='{}', {} rules", axiom, rules.size());
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
      if (c == ',') {
        if (!cur.empty()) {
          r.paramNames.push_back(cur);
          cur.clear();
        }
      } else if (c != ' ')
        cur += c;
    }
    if (!cur.empty()) r.paramNames.push_back(cur);
    r.successorExpr = pe.successorExpr;
    prules.push_back(std::move(r));
  }

  auto palgo = std::make_unique<ParametricLSystemAlgorithm>(std::move(pAxiom), std::move(prules),
                                                            m_angle);

  std::map<std::string, float> globals;
  for (const auto& pd : params)
    if (pd.name[0] != '\0') globals[pd.name] = pd.value;
  palgo->setGlobalParams(std::move(globals));

  m_algo = std::move(palgo);
  rebuildMesh();
  spdlog::info("[Grammar] Applied parametric: axiom='{}', {} rules, {} params",
               axiom, rules.size(), params.size());
}

}  // namespace D
