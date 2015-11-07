#include "hof.h"

#include <QtCore>
#include <random>

class Term;
typedef QSharedPointer<Term> TermPtr;

class Term {
public:
    enum Type { i_, k_, k1_, s_, s1_, s2_, p_, r_, r1_, a_};

    Term() : m_type(Type(-1)) { }
    Term(Type t) : m_type(t) { }
    virtual ~Term() { }
    TermPtr apply(TermPtr term) const;
    TermPtr eval(TermPtr term) const;
    Type type() const { return m_type; }
    QString toString() const;
    QString toStringApply(const Term* arg) const;

private:
    Type m_type;
};

struct I : Term {
    I() : Term(Term::i_) { }
    TermPtr apply(TermPtr x) const;
};

struct K : Term {
    K() : Term(Term::k_) { }
    struct K1 : Term {
        K1() : Term(Term::k1_) { }
        TermPtr apply(TermPtr /*y*/) const;
        TermPtr x;
    };
    TermPtr apply(TermPtr x) const;
};

struct S : Term {
    S() : Term(Term::s_) { }
    struct S1 : Term {
        S1() : Term(Term::s1_) { }
        struct S2 : Term {
            S2() : Term(Term::s2_) { }
            TermPtr apply(TermPtr z) const;
            TermPtr x;
            TermPtr y;
        };
        TermPtr x;
        TermPtr apply(TermPtr y) const;
    };
    TermPtr apply(TermPtr x) const;
};

class P : public Term {
public:
    P() : Term(Term::p_) { }
    TermPtr apply(TermPtr x) const;
    QString output() const { return m_output; }

private:
    mutable QString m_output;
};

struct R : Term {
    R() : Term(Term::r_) { }
    struct R1 : Term {
        R1() : Term(Term::r1_) { }
        TermPtr apply(TermPtr y) const;
        TermPtr x;
    };
    TermPtr apply(TermPtr x) const;
};

struct A : Term {
    A() : Term(Term::a_) { }
    TermPtr apply() const;
    TermPtr apply(TermPtr x) const;

    bool isFull() const;
    bool isWellFormed() const;
    void addTerm(TermPtr);
    TermPtr left;
    TermPtr right;
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
    case p_:
        return QStringLiteral("P");
    case r_:
        return QStringLiteral("R");
    case r1_:
          return QString("R%1").arg(static_cast<const R::R1*>(this)->x->toString());
    case a_:
      {
          const A* a = static_cast<const A*>(this);
          return QString("A%1%2").arg(a->left ? a->left->toString() : QString())
                                 .arg(a->right ? a->right->toString() : QString());
      }
    default:
        Q_ASSERT(false);
        return QString();
    }
}

QString CYAN() { return "\033[96m"; }
QString PURPLE() { return "\033[95m"; }
QString BLUE() { return "\033[94m"; }
QString YELLOW() { return "\033[93m"; }
QString GREEN() { return "\033[92m"; }
QString RED() { return "\033[91m"; }
QString RESET() { return "\033[0m"; }

QString Term::toStringApply(const Term* arg) const
{
    switch(m_type) {
    case i_:
    case k_:
    case s_:
    case p_:
    case r_:
        return GREEN() + toString() + RED() + arg->toString() + RESET();
    case s1_:
    case k1_:
    case r1_:
    case s2_:
        return CYAN() + toString() + RED() + arg->toString() + RESET();
    case a_:
        return BLUE() + toString() + RED() + arg->toString() + RESET();
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
        fprintf(stderr, "%s", qPrintable(m_out));
        m_out.clear();
    }

    bool isVerbose() const
    { return m_verbose; }

    void setVerbose(bool verbose)
    { m_verbose = verbose; }

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
    void generateEvalString(const Term* term1, const Term* term2);
    void generateReturnString(const Term* r);
    void generateReplacementString(const Term* term1, const Term* term2);

private:
    Verbose()
    {
        m_verbose = false;
        m_stream = new QTextStream(&m_out);
    }

    bool m_verbose;
    QString m_out;
    QString m_program;
    QStringList m_prefix;
    QStringList m_postfix;
    QTextStream* m_stream;
};

void Verbose::generateProgramString(const QString& string, bool replace)
{
    if (!m_verbose)
        return;
    if (replace)
        m_program = string;
    else
        m_program += string;
    *m_stream << PURPLE() << m_program << "\n" << RESET();
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

void Verbose::generateEvalString(const Term* term1, const Term* term2)
{
    if (!m_verbose)
        return;

    QString apply = term1->toStringApply(term2);
    if (apply.isEmpty())
        return;

    *m_stream << "  "
        << prefix()
        << apply
        << postfix()
        << "\n";

    print();
}

void Verbose::generateReturnString(const Term* r)
{
    if (!m_verbose)
        return;

    QString ret = r->toString();
    if (ret.isEmpty())
        return;

    *m_stream << "  "
        << prefix()
        << ret
        << postfix()
        << "\n";
    print();
}

void Verbose::generateReplacementString(const Term* term1, const Term* term2)
{
    if (!m_verbose)
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

TermPtr Term::eval(TermPtr term) const
{
    Verbose::instance()->generateEvalString(this, term.data());

    TermPtr r;
    switch(m_type) {
    case i_:
        return static_cast<const I*>(this)->apply(term);
    case k_:
        return static_cast<const K*>(this)->apply(term);
    case k1_:
        return static_cast<const K::K1*>(this)->apply(term);
    case s_:
        return static_cast<const S*>(this)->apply(term);
    case s1_:
        return static_cast<const S::S1*>(this)->apply(term);
    case s2_:
        return static_cast<const S::S1::S2*>(this)->apply(term);
    case p_:
        return static_cast<const P*>(this)->apply(term);
    case r_:
        return static_cast<const R*>(this)->apply(term);
    case r1_:
        return static_cast<const R::R1*>(this)->apply(term);
    case a_:
        return static_cast<const A*>(this)->apply(term);
    default:
        {
            Q_ASSERT(false);
            r = i();
            break;
        }
    }

    Verbose::instance()->generateReturnString(r.data());
    return r;
}

TermPtr I::apply(TermPtr x) const
{
    return x;
}

TermPtr K::apply(TermPtr x) const
{
    TermPtr k1(new K1);
    k1.staticCast<K1>()->x = x;
    return k1;
}

TermPtr K::K1::apply(TermPtr /*y*/) const
{
    return x;
}

TermPtr S::apply(TermPtr x) const
{
    TermPtr s1(new S1);
    s1.staticCast<S1>()->x = x;
    return s1;
}

TermPtr S::S1::apply(TermPtr y) const
{
    if (x->type() == Term::k_) {
        TermPtr s2(new S2);
        s2.staticCast<S2>()->x = x;
        s2.staticCast<S2>()->y = y;
        Verbose::instance()->generateReplacementString(s2.data(), i().data());
        return i();
    }

    TermPtr s2(new S2);
    s2.staticCast<S2>()->x = x;
    s2.staticCast<S2>()->y = y;
    return s2;
}

TermPtr S::S1::S2::apply(TermPtr z) const
{
    TermPtr first;
    TermPtr second;
    if (Verbose::instance()->isVerbose()) {
        SubEval eval;
        eval.addPostfix("A" + y->toString() + z->toString());
        first = x->eval(z);
    } else
        first = x->eval(z);

    if (Verbose::instance()->isVerbose()) {
        SubEval eval;
        eval.addPrefix(first->toString() + BLUE() + "A" + RESET());
        second = y->eval(z);
    } else
        second = y->eval(z);

    return (first->eval(second));
}

TermPtr P::apply(TermPtr x) const
{
    m_output += x->toString();
    return x;
}

TermPtr R::apply(TermPtr x) const
{
    TermPtr r1(new R1);
    r1.staticCast<R1>()->x = x;
    return r1;
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

void A::addTerm(TermPtr term)
{
    Q_ASSERT(!isFull());

    if (left.isNull())
        left = term;
    else
        right = term;
}

TermPtr A::apply() const
{
    Q_ASSERT(isWellFormed());
    SubEval eval;
    eval.addPrefix(QStringLiteral("A"));
    return left->eval(right);
}

TermPtr A::apply(TermPtr x) const
{
    if (left.isNull() || right.isNull())
        return x;

    TermPtr evaluate = apply();
    return evaluate->eval(x);
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

TermPtr R::R1::apply(TermPtr y) const
{
    return Random::instance()->boolean() ? x : y;
}

typedef QList<TermPtr> EvaluationList;
bool evaluationListIsWellFormed(const EvaluationList& list)
{
    EvaluationList::const_iterator it = list.begin();
    for (; it != list.end(); ++it) {
        if ((*it)->type() == Term::a_)
            return static_cast<const A*>((*it).data())->isWellFormed();
    }
    return true;
}

QString cppInterpreter(const QString& string)
{
    Verbose::instance()->generateProgramString("program: " + string);
    Verbose::instance()->generateProgramString("begin", true /*replace*/);
    int postfixIndex = Verbose::instance()->addPostfix(string);

    EvaluationList evaluationList;
    for (int x = 0; x < string.length(); x++) {
        TermPtr term;
        QChar ch = string.at(x);
        switch (ch.unicode()) {
        case 'I': term = i(); break;
        case 'K': term = k(); break;
        case 'S': term = s(); break;
        case 'P': term = p(); break;
        case 'R': term = r(); break;
        case 'A': term = TermPtr(new A); break;
        default:
            QString error = QString("Error: invalid char in hof program! ch=`%1`").arg(ch);
            Q_ASSERT_X(false, "hof", qPrintable(error));
            return QString();
        }

        if (!evaluationList.isEmpty() && term->type() != Term::a_) {
            if (evaluationList.back()->type() == Term::a_) {
                A* a = static_cast<A*>(evaluationList.back().data());
                if (!a->isFull())
                    a->addTerm(term);
                if (!a->isFull())
                    continue;
                term = TermPtr(); // remove the term since it was added
            }

            if (!evaluationListIsWellFormed(evaluationList))
                continue;

            Verbose::instance()->removePostfix(postfixIndex);
            postfixIndex = Verbose::instance()->addPostfix(string.right(string.length() - x - 1));

            SubEval eval;
            eval.addPostfix(!term.isNull() ? term->toString() : QString());

            TermPtr evaluate = evaluationList.takeFirst();
            EvaluationList::const_iterator it = evaluationList.begin();
            for (; it != evaluationList.end(); ++it) {
                TermPtr next = *it;
                evaluate = evaluate->eval(next);
            }

            eval.clear();

            if (!term.isNull())
                evaluate = evaluate->eval(term);
            evaluationList.clear();
            term = evaluate;
        }

        evaluationList.append(term);
    }

    Q_ASSERT(evaluationList.size() <= 1);
    if (!evaluationList.isEmpty() && evaluationList.first()->type() == Term::a_) {
        Verbose::instance()->removePostfix(postfixIndex);
        A* a = static_cast<A*>(evaluationList.first().data());
        if (a->isWellFormed())
            a->apply();
    }

    Verbose::instance()->generateProgramString("end", true /*replace*/);
    return static_cast<P*>(p().data())->output();
}

Hof::Hof(bool verbose)
  : m_verbose(verbose)
{
    Verbose::instance()->setVerbose(verbose);
}

Hof::~Hof()
{ }

QString Hof::run(const QString& string)
{
    return cppInterpreter(string);
}
