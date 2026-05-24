#pragma once

#include <QWidget>

class QPlainTextEdit;

namespace D {

class LogWidget final : public QWidget {
  Q_OBJECT

 public:
  explicit LogWidget(QWidget* parent = nullptr);

 public slots:
  // Safe to call from the main thread only (via QueuedConnection from QtLogSink).
  // Discards the message if this widget is not currently visible.
  void appendMessage(const QString& msg);

 private:
  QPlainTextEdit* m_log = nullptr;
};

}  // namespace D
