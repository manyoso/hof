#include "colors.h"

QString CYAN(OutputFormat f)
{
    if (f == OutputFormat::None)
        return QString();
    return "\033[96m";
}

QString PURPLE(OutputFormat f)
{
    if (f == OutputFormat::None)
        return QString();
    return "\033[95m";
}

QString BLUE(OutputFormat f)
{
    if (f == OutputFormat::None)
        return QString();
    return "\033[94m";
}

QString YELLOW(OutputFormat f)
{
    if (f == OutputFormat::None)
        return QString();
    return "\033[93m";
}

QString GREEN(OutputFormat f)
{
    if (f == OutputFormat::None)
        return QString();
    return "\033[92m";
}

QString RED(OutputFormat f)
{
    if (f == OutputFormat::None)
        return QString();
    return "\033[91m";
}

QString RESET(OutputFormat f)
{
    if (f == OutputFormat::None)
        return QString();
    return "\033[0m";
}
