#include "combinators.h"

#include "cache.h"
#include "colors.h"
#include "verbose.h"

#include <random>

CombinatorPtr eval(const CombinatorPtr& left, const CombinatorPtr& right)
{
    static int evaluationDepth = 0;

    if (evaluationDepth >= 1000) {
        qDebug() << "Hof program has exceed maximum stack depth!";
        exit(2);
    }

    evaluationDepth++;

    CombinatorPtr cached = EvaluationCache::instance()->result(left->toStringApply(right));
    Verbose::instance()->generateEvalString(left, right, evaluationDepth, !cached.isNull());
    if (!cached.isNull()) {
        evaluationDepth--;
        return cached;
    }

    CombinatorPtr r;
    switch(left->type()) {
    case Combinator::i_:
        r = static_cast<const I*>(left.data())->apply(right); break;
    case Combinator::k_:
        r = static_cast<const K*>(left.data())->apply(right); break;
    case Combinator::k1_:
        r = static_cast<const K::K1*>(left.data())->apply(right); break;
    case Combinator::s_:
        r = static_cast<const S*>(left.data())->apply(right); break;
    case Combinator::s1_:
        r = static_cast<const S::S1*>(left.data())->apply(right); break;
    case Combinator::s2_:
        r = static_cast<const S::S1::S2*>(left.data())->apply(right); break;
    case Combinator::p_:
        r = static_cast<const P*>(left.data())->apply(right); break;
    case Combinator::r_:
        r = static_cast<const R*>(left.data())->apply(right); break;
    case Combinator::r1_:
        r = static_cast<const R::R1*>(left.data())->apply(right); break;
    case Combinator::a_:
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
    if (left->type() != Combinator::p_ && left->type() != Combinator::r_ &&
       (left->type() != Combinator::a_ || !static_cast<A*>(left.data())->doNotCache())) {
        EvaluationCache::instance()->insert(left->toStringApply(right), r);
    }
#endif

    return r;
}

QString Combinator::typeToString() const
{
    switch (m_type) {
    case i_:  return QStringLiteral("I");
    case k_:  return QStringLiteral("K");
    case k1_: return QStringLiteral("K1");
    case s_:  return QStringLiteral("S");
    case s1_: return QStringLiteral("S1");
    case s2_: return QStringLiteral("S2");
    case p_:  return QStringLiteral("P");
    case r_:  return QStringLiteral("R");
    case r1_: return QStringLiteral("R1");
    case a_:  return QStringLiteral("A");
    default:
        Q_ASSERT(false);
        return QString();
    }
}

QString Combinator::toString() const
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
          return QString("%1%2%3").arg(!a->isThunk ? "A" : QString())
                                  .arg(a->left ? a->left->toString() : QString())
                                  .arg(a->right ? a->right->toString() : QString());
      }
    default:
        Q_ASSERT(false);
        return QString();
    }
}

QString Combinator::toStringApply(const CombinatorPtr& arg, OutputFormat f) const
{
    switch(m_type) {
    case i_:
    case k_:
    case s_:
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

CombinatorPtr I::apply(const CombinatorPtr& x) const
{
    return x;
}

CombinatorPtr K::apply(const CombinatorPtr& x) const
{
    CombinatorPtr k1(new K1);
    k1.staticCast<K1>()->x = x;
    return k1;
}

CombinatorPtr K::K1::apply(const CombinatorPtr& /*y*/) const
{
    return x;
}

CombinatorPtr S::apply(const CombinatorPtr& x) const
{
    CombinatorPtr s1(new S1);
    s1.staticCast<S1>()->x = x;
    return s1;
}

CombinatorPtr S::S1::apply(const CombinatorPtr& y) const
{
    if (x->type() == Combinator::k_) {
        CombinatorPtr s2(new S2);
        s2.staticCast<S2>()->x = x;
        s2.staticCast<S2>()->y = y;
        Verbose::instance()->generateReplacementString(s2, i());
        return i();
    }

    CombinatorPtr s2(new S2);
    s2.staticCast<S2>()->x = x;
    s2.staticCast<S2>()->y = y;
    return s2;
}

CombinatorPtr S::S1::S2::apply(const CombinatorPtr& z) const
{
#if LAZY_EVALUATION
    CombinatorPtr first;
    if (Verbose::instance()->isVerbose()) {
        SubEval subEval;
        subEval.addPostfix(y->toString() + z->toString());
        first = eval(x, z);
    } else
        first = eval(x, z);

    CombinatorPtr second = EvaluationCache::instance()->result(y->toStringApply(z));
    if (second.isNull()) {
        A* yz = new A;
        yz->left = y;
        yz->right = z;
        yz->isThunk = true;
        second = CombinatorPtr(yz);
    }

    A* evaluate = new A;
    evaluate->left = first;
    evaluate->right = second;
    evaluate->isThunk = true;

    return CombinatorPtr(evaluate);
#else
    CombinatorPtr first;
    CombinatorPtr second;
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

CombinatorPtr P::apply(const CombinatorPtr& x) const
{
    CombinatorPtr toPrint = x;
    while (toPrint->type() == Combinator::a_ && static_cast<A*>(toPrint.data())->isThunk) {
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

CombinatorPtr R::apply(const CombinatorPtr& x) const
{
    CombinatorPtr r1(new R1);
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

CombinatorPtr R::R1::apply(const CombinatorPtr& y) const
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

    bool leftIsWellFormed = left->type() != Combinator::a_ || static_cast<const A*>(left.data())->isWellFormed();
    bool rightIsWellFormed = right->type() != Combinator::a_ || static_cast<const A*>(right.data())->isWellFormed();
    return leftIsWellFormed && rightIsWellFormed;
}

bool A::doNotCache() const
{
    Q_ASSERT(!left.isNull());
    if (left->type() == Combinator::a_)
        return static_cast<A*>(left.data())->doNotCache();
    return left->type() == Combinator::r_ || left->type() == Combinator::p_;
}

void A::addCombinator(const CombinatorPtr& term)
{
    Q_ASSERT(!isWellFormed());

    if (!left.isNull() && left->type() == Combinator::a_) {
        A* leftA = static_cast<A*>(left.data());
        if (!leftA->isWellFormed()) {
            leftA->addCombinator(term);
            return;
        }
    }

    if (!right.isNull() && right->type() == Combinator::a_) {
        A* rightA = static_cast<A*>(right.data());
        if (!rightA->isWellFormed()) {
            rightA->addCombinator(term);
            return;
        }
    }

    if (left.isNull())
        left = term;
    else
        right = term;
}

CombinatorPtr A::apply() const
{
    Q_ASSERT(isWellFormed());
    if (isThunk)
        return eval(left, right);

    if (Verbose::instance()->isVerbose()) {
        SubEval subEval;
        subEval.addPrefix(BLUE() + QStringLiteral("A") + RESET());
        return eval(left, right);
    } else
        return eval(left, right);
}

CombinatorPtr A::apply(const CombinatorPtr& x) const
{
    if (left.isNull() || right.isNull())
        return x;

    CombinatorPtr evaluate;
    {
        if (Verbose::instance()->isVerbose()) {
            SubEval subEval;
            subEval.addPostfix(x->toString());
            evaluate = apply();
        } else
            evaluate = apply();
    }
    return eval(evaluate, x);
}

CombinatorPtr i()
{
    static CombinatorPtr s_instance;
    if (!s_instance)
        s_instance = CombinatorPtr(new I);
    return s_instance;
}

CombinatorPtr k()
{
    static CombinatorPtr s_instance;
    if (!s_instance)
        s_instance = CombinatorPtr(new K);
    return s_instance;
}

CombinatorPtr s()
{
    static CombinatorPtr s_instance;
    if (!s_instance)
        s_instance = CombinatorPtr(new S);
    return s_instance;
}

CombinatorPtr p()
{
    static CombinatorPtr s_instance;
    if (!s_instance)
        s_instance = CombinatorPtr(new P);
    return s_instance;
}

CombinatorPtr r()
{
    static CombinatorPtr s_instance;
    if (!s_instance)
        s_instance = CombinatorPtr(new R);
    return s_instance;
}

SubEval::SubEval()
    : m_prefixNumber(-1)
    , m_postfixNumber(-1) { }

SubEval::SubEval(const SubEval& other)
{
    m_prefixNumber = other.m_prefixNumber;
    m_postfixNumber = other.m_postfixNumber;
    other.m_prefixNumber = -1;
    other.m_postfixNumber = -1;
}

SubEval::~SubEval()
{
    clear();
}

void SubEval::clear()
{
    if (m_prefixNumber != -1)
        Verbose::instance()->removePrefix(m_prefixNumber);
    if (m_postfixNumber != -1)
        Verbose::instance()->removePostfix(m_postfixNumber);
}

void SubEval::replacePrefix(const QString& prefix)
{
    Q_ASSERT(m_prefixNumber != -1);
    Verbose::instance()->replacePrefix(m_prefixNumber, prefix);
}

void SubEval::addPrefix(const QString& prefix)
{
    m_prefixNumber = Verbose::instance()->addPrefix(prefix);
    Q_ASSERT(m_prefixNumber != -1);
}

void SubEval::addPostfix(const QString& postfix)
{
    m_postfixNumber = Verbose::instance()->addPostfix(postfix);
    Q_ASSERT(m_postfixNumber != -1);
}
