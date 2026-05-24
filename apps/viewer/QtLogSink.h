#pragma once

#include <mutex>

#include <spdlog/sinks/base_sink.h>

#include <QMetaObject>

#include "LogWidget.h"

namespace D {

// spdlog sink that forwards formatted messages to LogWidget on the main thread.
// If the widget is not visible the message is discarded (checked inside appendMessage).
class QtLogSink final : public spdlog::sinks::base_sink<std::mutex> {
 public:
  explicit QtLogSink(LogWidget* widget) : m_widget(widget) {}

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    if (!m_widget) return;
    spdlog::memory_buf_t buf;
    formatter_->format(msg, buf);
    QString text = QString::fromUtf8(buf.data(), static_cast<int>(buf.size())).trimmed();
    // QueuedConnection: safe to call from any thread; runs on main thread.
    QMetaObject::invokeMethod(
        m_widget, [widget = m_widget, text]() { widget->appendMessage(text); },
        Qt::QueuedConnection);
  }

  void flush_() override {}

 private:
  LogWidget* m_widget;
};

}  // namespace D
