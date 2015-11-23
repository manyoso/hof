#ifndef colors_h
#define colors_h

#include "combinators.h"

#include <QtCore>

QString CYAN(OutputFormat f = OutputFormat::Bash);
QString PURPLE(OutputFormat f = OutputFormat::Bash);
QString BLUE(OutputFormat f = OutputFormat::Bash);
QString YELLOW(OutputFormat f = OutputFormat::Bash);
QString GREEN(OutputFormat f = OutputFormat::Bash);
QString RED(OutputFormat f = OutputFormat::Bash);
QString RESET(OutputFormat f = OutputFormat::Bash);

#endif // colors_h
