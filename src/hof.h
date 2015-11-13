#ifndef hof_h
#define hof_h

#include <QtCore>

class Hof {
public:
    Hof(QTextStream* outputStream, QTextStream* verboseStream = 0);
    ~Hof();

    void run(const QString& string);
};

#endif // hof_h
