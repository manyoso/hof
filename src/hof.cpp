#include "hof.h"

#include <QtCore>
#include <random>

#define LAZY_EVALUATION 1

class Term;
typedef QSharedPointer<Term> TermPtr;
typedef QList<TermPtr> EvaluationList;

enum Format {
  None,
  Bash
};

class Term {
public:
    enum Type { i_, k_, k1_, s_, s1_, s2_, v_, p_, r_, r1_, a_};

    Term() : m_type(Type(-1)) { }
    Term(Type t) : m_type(t) { }
    virtual ~Term() { }
    TermPtr apply(const TermPtr& term) const;
    Type type() const { return m_type; }
    QString toString() const;
    QString toStringApply(const TermPtr& arg, Format f = None) const;
    QString typeToString() const
    {
        switch (m_type) {
        case i_: return QStringLiteral("I");
        case k_: return QStringLiteral("K");
        case k1_: return QStringLiteral("K1");
        case s_: return QStringLiteral("S");
        case s1_: return QStringLiteral("S1");
        case s2_: return QStringLiteral("S2");
        case v_: return QStringLiteral("V");
        case p_: return QStringLiteral("P");
        case r_: return QStringLiteral("R");
        case r1_: return QStringLiteral("R1");
        case a_: return QStringLiteral("A");
        default:
            Q_ASSERT(false);
            return QString();
        }
    }

private:
    Type m_type;
};

struct I : Term {
    I() : Term(Term::i_) { }
    TermPtr apply(const TermPtr& x) const;
};

struct K : Term {
    K() : Term(Term::k_) { }
    struct K1 : Term {
        K1() : Term(Term::k1_) { }
        TermPtr apply(const TermPtr& /*y*/) const;
        TermPtr x;
    };
    TermPtr apply(const TermPtr& x) const;
};

struct S : Term {
    S() : Term(Term::s_) { }
    struct S1 : Term {
        S1() : Term(Term::s1_) { }
        struct S2 : Term {
            S2() : Term(Term::s2_) { }
            TermPtr apply(const TermPtr& z) const;
            TermPtr x;
            TermPtr y;
        };
        TermPtr x;
        TermPtr apply(const TermPtr& y) const;
    };
    TermPtr apply(const TermPtr& x) const;
};

struct V : Term {
    V() : Term(Term::v_) { }
    TermPtr apply(const TermPtr& x) const;
};

class P : public Term {
public:
    P() : Term(Term::p_) { m_stream = 0; }
    TermPtr apply(const TermPtr& x) const;
    void setStream(QTextStream* stream);
private:
    QTextStream* m_stream;
};

struct R : Term {
    R() : Term(Term::r_) { }
    struct R1 : Term {
        R1() : Term(Term::r1_) { }
        TermPtr apply(const TermPtr& y) const;
        TermPtr x;
    };
    TermPtr apply(const TermPtr& x) const;
};

struct A : Term {
    A() : Term(Term::a_), isThunk(false) { }
    TermPtr apply() const;
    TermPtr apply(const TermPtr& x) const;

    bool isFull() const;
    bool isWellFormed() const;
    bool doNotCache() const;
    void addTerm(const TermPtr&);
    TermPtr left;
    TermPtr right;
    bool isThunk;
};

static TermPtr i()
{
    static TermPtr s_instance;
    if (!s_instance)
        s_instance = TermPtr(new I);
    return s_instance;
}

static TermPtr k()
{
    static TermPtr s_instance;
    if (!s_instance)
        s_instance = TermPtr(new K);
    return s_instance;
}

static TermPtr s()
{
    static TermPtr s_instance;
    if (!s_instance)
        s_instance = TermPtr(new S);
    return s_instance;
}

static TermPtr v()
{
    static TermPtr s_instance;
    if (!s_instance)
        s_instance = TermPtr(new V);
    return s_instance;
}

static TermPtr p()
{
    static TermPtr s_instance;
    if (!s_instance)
        s_instance = TermPtr(new P);
    return s_instance;
}

static TermPtr r()
{
    static TermPtr s_instance;
    if (!s_instance)
        s_instance = TermPtr(new R);
    return s_instance;
}

QString Term::toString() const
{
    switch(m_type) {
    case i_:
        return QStringLiteral("I");
    case k_:
        return QStringLiteral("K");
    case k1_:
      {
          return QString("K%1").arg(static_cast<const K::K1*>(this)->x->toString());
      }
    case s_:
        return QStringLiteral("S");
    case s1_:
      {
          return QString("S%1").arg(static_cast<const S::S1*>(this)->x->toString());
      }
    case s2_:
      {
          const S::S1::S2* s2 = static_cast<const S::S1::S2*>(this);
          return QString("S%1%2").arg(s2->x->toString()).arg(s2->y->toString());
      }
    case v_:
        return QStringLiteral("V");
    case p_:
        return QStringLiteral("P");
    case r_:
        return QStringLiteral("R");
    case r1_:
          return QString("R%1").arg(static_cast<const R::R1*>(this)->x->toString());
    case a_:
      {
          const A* a = static_cast<const A*>(this);
          return QString("%1%2%3").arg(!a->isThunk ? "A" : QString())
                                  .arg(a->left ? a->left->toString() : QString())
                                  .arg(a->right ? a->right->toString() : QString());
      }
    default:
        Q_ASSERT(false);
        return QString();
    }
}

QString CYAN(Format f = Format::Bash)
{
    if (f == Format::None)
        return QString();
    return "\033[96m";
}

QString PURPLE(Format f = Format::Bash)
{
    if (f == Format::None)
        return QString();
    return "\033[95m";
}

QString BLUE(Format f = Format::Bash)
{
    if (f == Format::None)
        return QString();
    return "\033[94m";
}

QString YELLOW(Format f = Format::Bash)
{
    if (f == Format::None)
        return QString();
    return "\033[93m";
}

QString GREEN(Format f = Format::Bash)
{
    if (f == Format::None)
        return QString();
    return "\033[92m";
}

QString RED(Format f = Format::Bash)
{
    if (f == Format::None)
        return QString();
    return "\033[91m";
}

QString RESET(Format f = Format::Bash)
{
    if (f == Format::None)
        return QString();
    return "\033[0m";
}

QString Term::toStringApply(const TermPtr& arg, Format f) const
{
    switch(m_type) {
    case i_:
    case k_:
    case s_:
    case v_:
    case p_:
    case r_:
    case a_:
        return GREEN(f) + toString() + RED(f) + arg->toString() + RESET(f);
    case s1_:
    case k1_:
    case r1_:
      {
          QString s = toString();
          s.insert(1, "₁");
          return CYAN(f) + s + RED(f) + arg->toString() + RESET(f);
      }
    case s2_:
      {
          QString s = toString();
          s.insert(1, "₂");
          return CYAN(f) + s + RED(f) + arg->toString() + RESET(f);
      }
    default:
        Q_ASSERT(false);
        return QString();
    }
}

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
    void generateOutputString();
    void generateOutputStringEnd();
    void generateEvalString(const TermPtr& term1, const TermPtr& term2, int evaluationDepth);
    void generateReturnString(const TermPtr& r);
    void generateInputString(const EvaluationList& list);
    void generateReplacementString(const TermPtr& term1, const TermPtr& term2);

private:
    Verbose()
    {
        m_stream = 0;
        m_format = Format::Bash;
    }

    QString m_program;
    QStringList m_prefix;
    QStringList m_postfix;
    QTextStream* m_stream;
    Format m_format;
};

void Verbose::generateProgramString(const QString& string, bool replace)
{
    if (!isVerbose())
        return;
    if (replace)
        m_program = string;
    else
        m_program += string;
    *m_stream << PURPLE(m_format) << m_program << "\n" << RESET(m_format);
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

void Verbose::generateEvalString(const TermPtr& term1, const TermPtr& term2, int evaluationDepth)
{
    if (!isVerbose())
        return;

    QString apply = term1->toStringApply(term2, m_format);
    if (apply.isEmpty())
        return;

    Q_UNUSED(evaluationDepth);

    *m_stream << "  "
        << prefix()
        << apply
        << postfix()
#if 0
        << "  " << evaluationDepth
#endif
        << "\n";

    print();
}

void Verbose::generateReturnString(const TermPtr& r)
{
    if (!isVerbose())
        return;

    QString ret = r->toString();
    if (ret.isEmpty())
        return;

    QString program = "return type: " + r->typeToString() + "";
    Verbose::instance()->generateProgramString(program, true /*replace*/);

    *m_stream << "  "
        << prefix()
        << ret
        << postfix()
        << "\n";
    print();
}

void Verbose::generateInputString(const EvaluationList& list)
{
    if (!isVerbose() || list.isEmpty())
        return;

    // Whatever is left in the evaluation list is input
    Verbose::instance()->generateProgramString("input", true /*replace*/);
    EvaluationList::const_iterator it = list.begin();
    *m_stream << "  ";
    for (; it != list.end(); ++it) {
        TermPtr next = *it;
        *m_stream << next->toString();
    }
    *m_stream << "\n";
    print();
}

void Verbose::generateReplacementString(const TermPtr& term1, const TermPtr& term2)
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


class SubEval {
public:
    SubEval()
    {
        m_prefixNumber = -1;
        m_postfixNumber = -1;
    }

    SubEval(const SubEval& other)
    {
        m_prefixNumber = other.m_prefixNumber;
        m_postfixNumber = other.m_postfixNumber;
        other.m_prefixNumber = -1;
        other.m_postfixNumber = -1;
    }

    ~SubEval()
    {
        clear();
    }

    void clear()
    {
        if (m_prefixNumber != -1)
            Verbose::instance()->removePrefix(m_prefixNumber);
        if (m_postfixNumber != -1)
            Verbose::instance()->removePostfix(m_postfixNumber);
    }

    void replacePrefix(const QString& prefix)
    {
        Q_ASSERT(m_prefixNumber != -1);
        Verbose::instance()->replacePrefix(m_prefixNumber, prefix);
    }

    void addPrefix(const QString& prefix)
    {
        m_prefixNumber = Verbose::instance()->addPrefix(prefix);
        Q_ASSERT(m_prefixNumber != -1);
    }

    void addPostfix(const QString& postfix)
    {
        m_postfixNumber = Verbose::instance()->addPostfix(postfix);
        Q_ASSERT(m_postfixNumber != -1);
    }

private:
    mutable int m_prefixNumber;
    mutable int m_postfixNumber;
};

class EvaluationCache {
public:
    static EvaluationCache* instance()
    {
        static EvaluationCache* s_instance = 0;
        if (!s_instance)
            s_instance = new EvaluationCache;
        return s_instance;
    }

    void insert(const QString& key, const TermPtr& value);
    TermPtr result(const QString& key) const;

private:
    EvaluationCache()
    { }

    QHash<QString, TermPtr> m_cache;
};

void EvaluationCache::insert(const QString& key, const TermPtr& value)
{
    if (!m_cache.contains(key))
        m_cache.insert(key, value);
}

TermPtr EvaluationCache::result(const QString& key) const
{
    return m_cache.value(key, TermPtr());
}

TermPtr eval(const TermPtr& left, const TermPtr& right)
{
    static int evaluationDepth = 0;

    if (evaluationDepth >= 1000) {
        qDebug() << "Hof program has exceed maximum stack depth!";
        exit(2);
    }

    Verbose::instance()->generateEvalString(left, right, evaluationDepth);

    evaluationDepth++;

    TermPtr cached = EvaluationCache::instance()->result(left->toStringApply(right));
    if (!cached.isNull()) {
        evaluationDepth--;
        return cached;
    }

    TermPtr r;
    switch(left->type()) {
    case Term::i_:
        r = static_cast<const I*>(left.data())->apply(right); break;
    case Term::k_:
        r = static_cast<const K*>(left.data())->apply(right); break;
    case Term::k1_:
        r = static_cast<const K::K1*>(left.data())->apply(right); break;
    case Term::s_:
        r = static_cast<const S*>(left.data())->apply(right); break;
    case Term::s1_:
        r = static_cast<const S::S1*>(left.data())->apply(right); break;
    case Term::s2_:
        r = static_cast<const S::S1::S2*>(left.data())->apply(right); break;
    case Term::v_:
        r = static_cast<const V*>(left.data())->apply(right); break;
    case Term::p_:
        r = static_cast<const P*>(left.data())->apply(right); break;
    case Term::r_:
        r = static_cast<const R*>(left.data())->apply(right); break;
    case Term::r1_:
        r = static_cast<const R::R1*>(left.data())->apply(right); break;
    case Term::a_:
        r = static_cast<const A*>(left.data())->apply(right); break;
    default:
        {
            Q_ASSERT(false);
            r = i();
            break;
        }
    }

    evaluationDepth--;

#if LAZY_EVALUATION
    if (left->type() != Term::p_ && left->type() != Term::r_ &&
       (left->type() != Term::a_ || !static_cast<A*>(left.data())->doNotCache())) {
        EvaluationCache::instance()->insert(left->toStringApply(right), r);
    }
#endif

    return r;
}

TermPtr I::apply(const TermPtr& x) const
{
    return x;
}

TermPtr K::apply(const TermPtr& x) const
{
    TermPtr k1(new K1);
    k1.staticCast<K1>()->x = x;
    return k1;
}

TermPtr K::K1::apply(const TermPtr& /*y*/) const
{
    return x;
}

TermPtr S::apply(const TermPtr& x) const
{
    TermPtr s1(new S1);
    s1.staticCast<S1>()->x = x;
    return s1;
}

TermPtr S::S1::apply(const TermPtr& y) const
{
    if (x->type() == Term::k_) {
        TermPtr s2(new S2);
        s2.staticCast<S2>()->x = x;
        s2.staticCast<S2>()->y = y;
        Verbose::instance()->generateReplacementString(s2, i());
        return i();
    }

    TermPtr s2(new S2);
    s2.staticCast<S2>()->x = x;
    s2.staticCast<S2>()->y = y;
    return s2;
}

TermPtr S::S1::S2::apply(const TermPtr& z) const
{
#if LAZY_EVALUATION
    TermPtr first;
    if (Verbose::instance()->isVerbose()) {
        SubEval subEval;
        subEval.addPostfix(y->toString() + z->toString());
        first = eval(x, z);
    } else
        first = eval(x, z);

    TermPtr second = EvaluationCache::instance()->result(y->toStringApply(z));
    if (second.isNull()) {
        A* yz = new A;
        yz->left = y;
        yz->right = z;
        yz->isThunk = true;
        second = TermPtr(yz);
    }

    A* evaluate = new A;
    evaluate->left = first;
    evaluate->right = second;
    evaluate->isThunk = true;

    return TermPtr(evaluate);
#else
    TermPtr first;
    TermPtr second;
    if (Verbose::instance()->isVerbose()) {
        SubEval subEval;
        subEval.addPostfix(y->toString() + z->toString());
        first = eval(x, z);
    } else
        first = eval(x, z);

    if (Verbose::instance()->isVerbose()) {
        SubEval subEval;
        subEval.addPrefix(first->toString());
        second = eval(y, z);
    } else
        second = eval(y, z);

    return (eval(first, second));
#endif
}

TermPtr V::apply(const TermPtr& /*x*/) const
{
    return i();
}

TermPtr P::apply(const TermPtr& x) const
{
    TermPtr toPrint = x;
    while (toPrint->type() == Term::a_ && static_cast<A*>(toPrint.data())->isThunk) {
        SubEval subEval;
        subEval.addPrefix("P");
        A* a = static_cast<A*>(toPrint.data());
        toPrint = a->apply();
    }

    if (m_stream) {
        *m_stream << toPrint->toString();
        Verbose::instance()->generateOutputString();
        m_stream->flush();
        Verbose::instance()->generateOutputStringEnd();
    }
    return toPrint;
}

void P::setStream(QTextStream* stream)
{
    m_stream = stream;
}

TermPtr R::apply(const TermPtr& x) const
{
    TermPtr r1(new R1);
    r1.staticCast<R1>()->x = x;
    return r1;
}

class Random {
public:
    static Random* instance()
    {
        static Random* s_instance = 0;
        if (!s_instance)
            s_instance = new Random;
        return s_instance;
    }

    bool boolean() const
    {
        return (*m_dist)(*m_gen);
    }

private:
    Random()
    {
        std::random_device rd;
        m_gen = new std::mt19937(rd());
        m_dist = new std::bernoulli_distribution;
    }

    std::mt19937* m_gen;
    std::bernoulli_distribution* m_dist;
};

TermPtr R::R1::apply(const TermPtr& y) const
{
    return Random::instance()->boolean() ? x : y;
}

bool A::isFull() const
{
    return !left.isNull() && !right.isNull();
}

bool A::isWellFormed() const
{
    if (!isFull())
        return false;

    bool leftIsWellFormed = left->type() != Term::a_ || static_cast<const A*>(left.data())->isWellFormed();
    bool rightIsWellFormed = right->type() != Term::a_ || static_cast<const A*>(right.data())->isWellFormed();
    return leftIsWellFormed && rightIsWellFormed;
}

bool A::doNotCache() const
{
    Q_ASSERT(!left.isNull());
    return left->type() == Term::r_ || left->type() == Term::p_ ||
           (left->type() == Term::a_ && static_cast<const A*>(left.data())->doNotCache());
}

void A::addTerm(const TermPtr& term)
{
    Q_ASSERT(!isWellFormed());

    if (!left.isNull() && left->type() == Term::a_) {
        A* leftA = static_cast<A*>(left.data());
        if (!leftA->isWellFormed()) {
            leftA->addTerm(term);
            return;
        }
    }

    if (!right.isNull() && right->type() == Term::a_) {
        A* rightA = static_cast<A*>(right.data());
        if (!rightA->isWellFormed()) {
            rightA->addTerm(term);
            return;
        }
    }

    if (left.isNull())
        left = term;
    else
        right = term;
}

TermPtr A::apply() const
{
    Q_ASSERT(isWellFormed());
    if (isThunk)
        return eval(left, right);
    SubEval subEval;
    subEval.addPrefix(BLUE() + QStringLiteral("A") + RESET());
    return eval(left, right);
}

TermPtr A::apply(const TermPtr& x) const
{
    if (left.isNull() || right.isNull())
        return x;

    TermPtr evaluate;
    {
        SubEval subEval;
        subEval.addPostfix(x->toString());
        evaluate = apply();
    }
    return eval(evaluate, x);
}

bool evaluationListIsWellFormed(const EvaluationList& list)
{
    EvaluationList::const_iterator it = list.begin();
    for (; it != list.end(); ++it) {
        if ((*it)->type() == Term::a_)
            return static_cast<const A*>((*it).data())->isWellFormed();
    }
    return true;
}

void cppInterpreter(const QString& string)
{
    Verbose::instance()->generateProgramString("program: " + string);
    Verbose::instance()->generateProgramString("begin", true /*replace*/);
    int postfixIndex = Verbose::instance()->addPostfix(string);

    if (string.isEmpty()) {
        Verbose::instance()->generateProgramString("end", true /*replace*/);
        return;
    }

    EvaluationList evaluationList;
    for (int x = 0; x < string.length(); x++) {
        TermPtr term;
        QChar ch = string.at(x);
        switch (ch.unicode()) {
        case 'I': term = i(); break;
        case 'K': term = k(); break;
        case 'S': term = s(); break;
        case 'V': term = v(); break;
        case 'P': term = p(); break;
        case 'R': term = r(); break;
        case 'A': term = TermPtr(new A); break;
        default:
            QString error = QString("Error: invalid char in hof program! ch=`%1`").arg(ch);
            Q_ASSERT_X(false, "hof", qPrintable(error));
            return;
        }

        if (evaluationList.isEmpty()) {
            evaluationList.append(term);
            continue;
        }

        if (!evaluationList.isEmpty()) {
            if (term->type() == Term::a_) {
                if (evaluationList.back()->type() == Term::a_) {
                    A* a = static_cast<A*>(evaluationList.back().data());
                    Q_ASSERT(!a->isWellFormed());
                    a->addTerm(term);
                } else {
                    evaluationList.append(term);
                }

                continue; // can't possibly be well formed at this point
            }

            if (evaluationList.back()->type() == Term::a_) {
                A* a = static_cast<A*>(evaluationList.back().data());
                Q_ASSERT(!a->isWellFormed());
                a->addTerm(term);
                term = TermPtr(); // remove the term since it was added
            }

            if (!evaluationListIsWellFormed(evaluationList))
                continue;

            Verbose::instance()->removePostfix(postfixIndex);
            postfixIndex = Verbose::instance()->addPostfix(string.right(string.length() - x - 1));

            SubEval subEval;
            subEval.addPostfix(!term.isNull() ? term->toString() : QString());

            TermPtr evaluate = evaluationList.takeFirst();
            EvaluationList::const_iterator it = evaluationList.begin();
            for (; it != evaluationList.end(); ++it)
                evaluate = eval(evaluate, *it);

            subEval.clear();

            if (!term.isNull())
                evaluate = eval(evaluate, term);

            while (evaluate->type() == Term::a_) {
                A* a = static_cast<A*>(evaluate.data());
                if (!a->isWellFormed()) { break; }
                evaluate = a->apply();
            }

            evaluationList = EvaluationList() << evaluate;
        }
    }

    Q_ASSERT(evaluationList.length() >= 1);

    TermPtr evaluate = evaluationList.takeFirst();
    Verbose::instance()->generateReturnString(evaluate);
    Verbose::instance()->generateInputString(evaluationList);
    Verbose::instance()->generateProgramString("end", true /*replace*/);
}

Hof::Hof(QTextStream* outputStream, QTextStream* verboseStream)
{
    static_cast<P*>(p().data())->setStream(outputStream);
    Verbose::instance()->setStream(verboseStream);
}

Hof::~Hof()
{ }

void Hof::run(const QString& string)
{
    cppInterpreter(string);
}

