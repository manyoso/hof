#ifndef verbose_h
#define verbose_h

#include "combinators.h"

#include <QtCore>

class Verbose {
public:
    static Verbose* instance()
    {
        static Verbose* s_instance = 0;
        if (!s_instance)
            s_instance = new Verbose;
        return s_instance;
    }

    void print()
    {
        m_stream->flush();
    }

    bool isVerbose() const
    { return m_stream; }

    void setStream(QTextStream* stream)
    { m_stream = stream; }

    int prefixCount() const { return m_prefix.count(); }
    QString prefix() const;
    int addPrefix(const QString& prefix)
    {
        m_prefix.append(prefix);
        return m_prefix.count() - 1;
    }

    void replacePrefix(int i, const QString& prefix)
    {
        m_prefix.replace(i, prefix);
    }

    void removePrefix(int i)
    {
        m_prefix.removeAt(i);
    }

    int postfixCount() const { return m_postfix.count(); }
    QString postfix() const;
    int addPostfix(const QString& postfix)
    {
        m_postfix.prepend(postfix);
        return 0;
    }

    void removePostfix(int i)
    {
        m_postfix.removeAt(i);
    }

    void generateProgramString(const QString& string, bool replace = false);
    void generateProgramEnd();
    void generateOutputString();
    void generateOutputStringEnd();
    void generateEvalString(const CombinatorPtr& term1, const CombinatorPtr& term2, int evaluationDepth, bool cached);
    void generateReturnString(const CombinatorPtr& r);
    void generateInputString(const CombinatorPtr& input);
    void generateReplacementString(const CombinatorPtr& term1, const CombinatorPtr& term2);

private:
    Verbose()
    {
        m_stream = 0;
        m_format = OutputFormat::Bash;
        m_cacheHits = 0;
        m_cacheMisses = 0;
        m_depthAchieved = 0;
        m_longestEvalLine = 0;
    }

    QString m_program;
    QStringList m_prefix;
    QStringList m_postfix;
    QTextStream* m_stream;
    OutputFormat m_format;
    int m_cacheHits;
    int m_cacheMisses;
    int m_depthAchieved;
    int m_longestEvalLine;
};

#endif // verbose_h
