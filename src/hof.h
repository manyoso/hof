#ifndef hof_h
#define hof_h

#include <QtCore>

class Hof {
public:
    Hof(bool verbose);
    ~Hof();

    QString run(const QString& string);

private:
    bool m_verbose;
};

#endif // hof_h
