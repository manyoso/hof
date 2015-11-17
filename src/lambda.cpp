#include "lambda.h"

QString Lambda::fromLambda(const QString& string)
{
    for (int x = 0; x < string.length(); x++) {
        QChar ch = string.at(x);
        switch (ch.unicode()) {
        case 0x03BB: // lambda
        case 0x002E: // dot
        default:
            break;
        };
    }

    return string;
}