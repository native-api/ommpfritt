#include <QCommandLineParser>
#include "logging.h"
#include <iostream>

namespace omm
{

class SubcommandLineParser : public QCommandLineParser
{
public:
  explicit SubcommandLineParser(int argc, char* argv[]);
  QString command() const { return m_command; }

  template<typename T> T get(const QString& name) const;
  template<typename T> T get(const QString& name, const T& default_value) const
  {
    if (const QString value = this->value(name); value.isEmpty()) {
      return default_value;
    } else {
      return get<T>(name);
    }
  }

  static constexpr auto FRAMENUMBER_PLACEHOLDER = '#';

private:
  const QString m_appname;
  const QString m_command;
  void print_help() const;
};

}  // namespace omm