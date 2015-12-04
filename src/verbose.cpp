#include "verbose.h"
#include "colors.h"

Verbose::Verbose()
{
    m_stream = 0;
    m_format = OutputFormat::Bash;
    m_cacheHits = 0;
    m_cacheMisses = 0;
    m_depthAchieved = 0;
    m_longestEvalLine = 0;
}

void Verbose::generateProgramString(const QString& string)
{
    if (!isVerbose())
        return;
    m_program = string;
    *m_stream << PURPLE(m_format) << m_program << "\n" << RESET(m_format);
    print();
}

void Verbose::generateProgramEnd()
{
    if (!isVerbose())
        return;
    *m_stream << PURPLE(m_format) << "end\n" << RESET(m_format);
    *m_stream << RED(m_format)
              << "\tcacheHits:" << m_cacheHits << "\n"
              << "\tcacheMiss:" << m_cacheMisses << "\n"
              << "\t>depth:" << m_depthAchieved << "\n"
              << "\t>line:" << m_longestEvalLine << "\n"
              << RESET(m_format);
    print();
}

void Verbose::generateOutputString()
{
    if (!isVerbose())
        return;
    *m_stream << PURPLE(m_format) << "output: ";
    print();
}

void Verbose::generateOutputStringEnd()
{
    if (!isVerbose())
        return;
    *m_stream << "\n" << RESET(m_format);
    print();
}

QString Verbose::prefix() const
{
    return m_prefix.join("");
}

QString Verbose::postfix() const
{
    return m_postfix.join("");
}

void Verbose::generateEvalString(const CombinatorPtr& term1, const CombinatorPtr& term2, int evaluationDepth, bool cached)
{
    if (!isVerbose())
        return;

    QString apply = term1->toStringApply(term2, m_format);
    if (apply.isEmpty())
        return;

    Q_UNUSED(evaluationDepth);
    Q_UNUSED(cached);

    if (cached)
        m_cacheHits++;
    else
        m_cacheMisses++;

    if (evaluationDepth > m_depthAchieved)
        m_depthAchieved = evaluationDepth;

    if (apply.length() > m_longestEvalLine)
        m_longestEvalLine = apply.length();

    *m_stream << "  "
        << prefix()
        << apply
        << postfix()
#if 0
        << "\t" << "depth: " << evaluationDepth
        << ", " << (cached ? "cached: true" : "cached: false")
#endif
        << "\n";

    print();
}

void Verbose::generateReturnString(const CombinatorPtr& r)
{
    if (!isVerbose() || r.isNull())
        return;

    QString ret = r->toStringApply(CombinatorPtr(), m_format);
    if (ret.isEmpty())
        return;

    QString program = "return type: " + r->typeToString() + "";
    Verbose::instance()->generateProgramString(program);

    *m_stream << "  "
        << prefix()
        << ret
        << postfix()
        << "\n";
    print();
}

void Verbose::generateInputString(const CombinatorPtr& input)
{
    if (!isVerbose() || input.isNull())
        return;

    // Whatever is left in the evaluation list is input
    Verbose::instance()->generateProgramString("input");
    *m_stream << "  ";
    *m_stream << input->toString();
    *m_stream << "\n";
    print();
}

void Verbose::generateReplacementString(const CombinatorPtr& term1, const CombinatorPtr& term2)
{
    if (!isVerbose())
      return;

    QString from = term1->toString();
    QString to = term2->toString();
    if (from.isEmpty() || to.isEmpty())
        return;

    *m_stream << "  "
        << prefix()
        << YELLOW()
        << from
        << "->"
        << to
        << RESET()
        << postfix()
        << "\n";
    print();
}
