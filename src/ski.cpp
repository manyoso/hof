#include "ski.h"

class SkiTerm {
public:
    SkiTerm() { }
    SkiTerm(const QChar& c) { ch = c; }
    virtual ~SkiTerm() { }
    virtual QString toHof() const { return QString(ch); }
    virtual bool isWellFormed() const { return !ch.isNull(); }
    QChar ch;
};

class SkiSubTerm : public SkiTerm {
public:
    SkiSubTerm()
        : SkiTerm('A'),
        m_subTerm(0),
        m_closed(false) { }

    virtual ~SkiSubTerm()
    {
        qDeleteAll(m_terms);
        delete m_subTerm;
        m_subTerm = 0;
    }

    virtual QString toHof() const
    {
        QString a('A');
        QTextStream stream(&a);
        stream << QString(m_terms.count() - 2, 'A');
        foreach (SkiTerm* t, m_terms)
            stream << t->toHof();
        stream.flush();
        return a;
    }

    virtual bool isWellFormed() const
    {
        return m_closed && m_terms.count() >= 2 && !m_subTerm;
    }

    bool isClosed() const
    {
        return m_closed;
    }

    void addTerm(SkiTerm* term)
    {
        if (m_subTerm)
            m_subTerm->addTerm(term);
        else if (term->ch == 'A')
            m_subTerm = static_cast<SkiSubTerm*>(term);
        else
            m_terms.append(term);
    }

    void close()
    {
        if (m_subTerm)
            m_subTerm->close();
        else
            m_closed = true;

        if (m_subTerm && m_subTerm->m_closed) {
            m_terms.append(m_subTerm);
            m_subTerm = 0;
        }
    }

private:
    QList<SkiTerm*> m_terms;
    SkiSubTerm* m_subTerm;
    bool m_closed;
};

QString Ski::fromSki(const QString& string)
{
    SkiSubTerm* subTerm = 0;
    QList<SkiTerm*> terms;
    for (int x = 0; x < string.length(); x++) {
        SkiTerm* term = 0;
        QChar ch = string.at(x);
        switch (ch.unicode()) {
        case '(': term = new SkiSubTerm; break;
        case ')':
            {
                subTerm->close();
                if (subTerm->isClosed()) {
                    terms.append(subTerm);
                    subTerm = 0;
                }
                continue;
            }
        case 'S':
        case 's': term = new SkiTerm('S'); break;
        case 'K':
        case 'k': term = new SkiTerm('K'); break;
        case 'I':
        case 'i': term = new SkiTerm('I'); break;
        default: term = new SkiTerm(ch); break;
        };

        if (subTerm)
            subTerm->addTerm(term);
        else if (term->ch == 'A')
            subTerm = static_cast<SkiSubTerm*>(term);
        else
            terms.append(term);
    }

    if (subTerm)
        terms.append(subTerm); // wasn't closed properly

    bool error = false;
    QString hof;
    QTextStream stream(&hof);
    foreach (SkiTerm* t, terms) {
        Q_ASSERT(t);
        error = !t->isWellFormed() ? true : error;
        stream << t->toHof();
    }
    stream.flush();

    if (error) {
        QString error = QString("Error: from ski to hof: program is not well formed! program=`%1`").arg(hof);
        Q_ASSERT_X(false, "translate", qPrintable(error));
        return error;
    }

    qDeleteAll(terms); // cleanup

    return hof;
}
