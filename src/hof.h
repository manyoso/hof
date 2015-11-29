#ifndef hof_h
#define hof_h

#include <QtCore>

class Hof {
public:
    Hof(QTextStream* outputStream);
    ~Hof();

    void run(const QString& string);
};

#endif // hof_h
