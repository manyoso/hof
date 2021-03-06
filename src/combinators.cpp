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
    switch (left->type()) {
    case Combinator::i_:
        r = static_cast<const I*>(left.data())->apply(right); break;
    case Combinator::k_:
        r = static_cast<const K*>(left.data())->apply(right); break;
    case Combinator::s_:
        r = static_cast<const S*>(left.data())->apply(right); break;
    case Combinator::p_:
        r = static_cast<const P*>(left.data())->apply(right); break;
    case Combinator::r_:
        r = static_cast<const R*>(left.data())->apply(right); break;
    case Combinator::a_:
        r = static_cast<const A*>(left.data())->apply(right); break;
    case Combinator::var_:
        r = static_cast<const Var*>(left.data())->apply(right); break;
    case Combinator::capture_:
      {
          Capture* cap = static_cast<Capture*>(left.data());
          if (!cap->isFull()) {
              cap->append(right);
              r = left;
          } else {
              switch (cap->callback->type()) {
              case Combinator::k_:
                  r = static_cast<const K*>(cap->callback.data())->apply(right, left); break;
              case Combinator::r_:
                  r = static_cast<const R*>(cap->callback.data())->apply(right, left); break;
              case Combinator::s_:
                  r = static_cast<const S*>(cap->callback.data())->apply(right, left); break;
              case Combinator::b_:
                  r = static_cast<const B*>(cap->callback.data())->apply(right, left); break;
              case Combinator::c_:
                  r = static_cast<const C*>(cap->callback.data())->apply(right, left); break;
              default:
                  {
                      Q_ASSERT(false);
                      r = i();
                      break;
                  }
              }
          }
          break;
      }
    default:
        {
            Q_ASSERT(false);
            r = i();
            break;
        }
    }

    evaluationDepth--;

    if (r->type() != Combinator::capture_ &&
        left->type() != Combinator::p_ &&
        left->type() != Combinator::r_ &&
        (left->type() != Combinator::a_ || !static_cast<A*>(left.data())->doNotCache())) {
        EvaluationCache::instance()->insert(left->toStringApply(right), r);
    }

    return r;
}

QString Combinator::typeToString() const
{
    switch (m_type) {
    case i_:  return QStringLiteral("I");
    case k_:  return QStringLiteral("K");
    case s_:  return QStringLiteral("S");
    case p_:  return QStringLiteral("P");
    case r_:  return QStringLiteral("R");
    case a_:  return QStringLiteral("A");
    case b_:  return QStringLiteral("B");
    case c_:  return QStringLiteral("C");
    case var_:
              return QStringLiteral("Var");
    case capture_:
              return QStringLiteral("Capture");
    default:
        Q_ASSERT(false);
        return QString();
    }
}

QString Combinator::toString() const
{
    switch(m_type) {
    case i_:
    case k_:
    case s_:
    case p_:
    case r_:
    case b_:
    case c_:
        return typeToString();
    case var_:
      {
          const Var* v = static_cast<const Var*>(this);
          return v->ch;
      }
    case a_:
      {
          const A* a = static_cast<const A*>(this);
          return QString("%1%2%3").arg(!a->isThunk ? "A" : QString())
                                  .arg(a->left ? a->left->toString() : QString())
                                  .arg(a->right ? a->right->toString() : QString());
      }
    case capture_:
      {
          const Capture* cap = static_cast<const Capture*>(this);
          QString str = cap->callback->toString();
          foreach (CombinatorPtr ptr, cap->args)
              str.append(ptr->toString());
          return str;
      }
    default:
        Q_ASSERT(false);
        return QString();
    }
}

QString Combinator::toStringApply(const CombinatorPtr& arg, OutputFormat f) const
{
    QString argString = arg.isNull() ? RESET(f) : RED(f) + arg->toString() + RESET(f);
    switch(m_type) {
    case i_:
    case k_:
    case s_:
    case p_:
    case r_:
    case a_:
    case b_:
    case c_:
    case var_:
        return GREEN(f) + toString() + argString;
    case capture_:
      {
          const Capture* cap = static_cast<const Capture*>(this);
          int l = cap->argsToCapture;
          Q_ASSERT(l <= 3);
          QString s = toString();
          if (l == 1)
              s.insert(1, "₁");
          else if (l == 2)
              s.insert(1, "₂");
          else
              s.insert(1, "₃");
          return CYAN(f) + s + argString;
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

CombinatorPtr K::apply(const CombinatorPtr& arg, CombinatorPtr capture) const
{
    if (capture.isNull()) {
        CombinatorPtr newC(new Capture(k(), 1));
        newC.staticCast<Capture>()->args.append(arg);
        return newC;
    }

    Capture* cap = static_cast<Capture*>(capture.data());
    Q_ASSERT(cap->args.length() == 1);
    return cap->x();
}

CombinatorPtr B::apply(const CombinatorPtr& arg, CombinatorPtr capture) const
{
    Q_ASSERT(!capture.isNull());
    Capture* cap = static_cast<Capture*>(capture.data());
    Q_ASSERT(cap->args.length() == 2);
    CombinatorPtr x = cap->x();
    CombinatorPtr y = cap->y();
    CombinatorPtr z = arg;

    CombinatorPtr first = EvaluationCache::instance()->result(y->toStringApply(z));
    if (first.isNull()) {
        A* yz = new A;
        yz->left = y;
        yz->right = z;
        yz->isThunk = true;
        first = CombinatorPtr(yz);
    }

    A* evaluate = new A;
    evaluate->left = x;
    evaluate->right = first;
    evaluate->isThunk = true;

    return CombinatorPtr(evaluate);
}

CombinatorPtr C::apply(const CombinatorPtr& arg, CombinatorPtr capture) const
{
    Q_ASSERT(!capture.isNull());
    Capture* cap = static_cast<Capture*>(capture.data());
    Q_ASSERT(cap->args.length() == 2);
    CombinatorPtr x = cap->x();
    CombinatorPtr y = cap->y();
    CombinatorPtr z = arg;

    CombinatorPtr first = EvaluationCache::instance()->result(x->toStringApply(z));
    if (first.isNull()) {
        if (Verbose::instance()->isVerbose()) {
            SubEval subEval;
            subEval.addPostfix(y->toString() + z->toString());
            first = eval(x, z);
        } else
            first = eval(x, z);
    }

    A* evaluate = new A;
    evaluate->left = first;
    evaluate->right = y;
    evaluate->isThunk = true;

    return CombinatorPtr(evaluate);
}

CombinatorPtr S::apply(const CombinatorPtr& arg, CombinatorPtr capture) const
{
    if (capture.isNull()) {
        CombinatorPtr newC(new Capture(s(), 1));
        newC.staticCast<Capture>()->args.append(arg);
        return newC;
    }

    Capture* cap = static_cast<Capture*>(capture.data());
    Q_ASSERT(cap->args.length() >= 1);
    CombinatorPtr x = cap->x();
    if (cap->args.length() == 1) {
        /*
         * Various optimizations taken from the paper, "Another Algorithm for
         * Bracket Abstraction" by D. A. Turner and "The Implementation of
         * Functional Programming Languages" by Simon L. Peyton Jones.
         */

        /* identity optimization: SKx -> I */
        if (x->type() == Combinator::k_) {
            Verbose::instance()->generateReplacementString(capture, i());
            return i();
        }

        cap->argsToCapture = 2; // capture one more...
        cap->append(arg);

        CombinatorPtr y = cap->y();

        if (x->type() == Combinator::a_) {
            A* aX = static_cast<A*>(x.data());
            if (aX->left->type() == Combinator::k_) {

                /* k optimization: SAKpAKq -> KApq */
                if (y->type() == Combinator::a_) {
                    A* aY = static_cast<A*>(y.data());
                    if (aY->left->type() == Combinator::k_) {

                        A* pq = new A;
                        pq->left = aX->right;
                        pq->right = aY->right;

                        CombinatorPtr newC(new Capture(k(), 1));
                        newC.staticCast<Capture>()->args.append(CombinatorPtr(pq));

                        Verbose::instance()->generateReplacementString(capture, newC);
                        return newC;
                    }
                }

                /* b optimization: SAKxy -> Bxy*/
                /* special b optimization */
                if (y->type() == Combinator::i_) {
                    Verbose::instance()->generateReplacementString(capture, aX->right);
                    return aX->right;
                }

#if OPTIMIZATIONS
                CombinatorPtr newC(new Capture(b(), 2));
                newC.staticCast<Capture>()->args.append(aX->right);
                newC.staticCast<Capture>()->args.append(y);

                Verbose::instance()->generateReplacementString(capture, newC);
                return newC;
#endif
            }
        }

#if OPTIMIZATIONS
        /* c optimization: SxAKy -> Cxy*/
        if (y->type() == Combinator::a_) {
            A* aY = static_cast<A*>(y.data());
            if (aY->left->type() == Combinator::k_) {
                CombinatorPtr newC(new Capture(c(), 2));
                newC.staticCast<Capture>()->args.append(x);
                newC.staticCast<Capture>()->args.append(aY->right);

                Verbose::instance()->generateReplacementString(capture, newC);
                return newC;
            }
        }
#endif

        return capture;
    }

    Q_ASSERT(cap->args.length() == 2);
    CombinatorPtr y = cap->y();
    CombinatorPtr z = arg;
    CombinatorPtr first = EvaluationCache::instance()->result(x->toStringApply(z));
    if (first.isNull()) {
        if (Verbose::instance()->isVerbose()) {
            SubEval subEval;
            subEval.addPostfix(y->toString() + z->toString());
            first = eval(x, z);
        } else
            first = eval(x, z);
    }

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

CombinatorPtr R::apply(const CombinatorPtr& arg, CombinatorPtr capture) const
{
    if (capture.isNull()) {
        CombinatorPtr newC(new Capture(r(), 1));
        newC.staticCast<Capture>()->args.append(arg);
        return newC;
    }

    Capture* cap = static_cast<Capture*>(capture.data());
    Q_ASSERT(cap->args.length() == 1);
    return Random::instance()->boolean() ? cap->x() : arg /*y*/;
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

void Capture::append(const CombinatorPtr& arg)
{
    Q_ASSERT(args.length() < argsToCapture);
    Q_ASSERT(arg.data() != this);
    args.append(arg);
}

CombinatorPtr Var::apply(const CombinatorPtr& x) const
{
    return x;
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

CombinatorPtr b()
{
    static CombinatorPtr s_instance;
    if (!s_instance)
        s_instance = CombinatorPtr(new B);
    return s_instance;
}

CombinatorPtr c()
{
    static CombinatorPtr s_instance;
    if (!s_instance)
        s_instance = CombinatorPtr(new C);
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
