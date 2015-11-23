#ifndef combinators_h
#define combinators_h

#include <QtCore>

#define LAZY_EVALUATION 1

class Combinator;
typedef QSharedPointer<Combinator> CombinatorPtr;
typedef QList<CombinatorPtr> EvaluationList;

// singleton combinators
CombinatorPtr i();
CombinatorPtr k();
CombinatorPtr s();
CombinatorPtr p();
CombinatorPtr r();

// general evaluation function
CombinatorPtr eval(const CombinatorPtr& left, const CombinatorPtr& right);

enum OutputFormat {
  None,
  Bash
};

class Combinator {
public:
    enum Type { i_, k_, k1_, s_, s1_, s2_, p_, r_, r1_, a_};

    Combinator() : m_type(Type(-1)) { }
    Combinator(Type t) : m_type(t) { }
    virtual ~Combinator() { }
    Type type() const { return m_type; }
    QString toString() const;
    QString toStringApply(const CombinatorPtr& arg, OutputFormat f = None) const;
    QString typeToString() const;

private:
    Type m_type;
};

struct I : Combinator {
    I() : Combinator(Combinator::i_) { }
    CombinatorPtr apply(const CombinatorPtr& x) const;
};

struct K : Combinator {
    K() : Combinator(Combinator::k_) { }
    struct K1 : Combinator {
        K1() : Combinator(Combinator::k1_) { }
        CombinatorPtr apply(const CombinatorPtr& /*y*/) const;
        CombinatorPtr x;
    };
    CombinatorPtr apply(const CombinatorPtr& x) const;
};

struct S : Combinator {
    S() : Combinator(Combinator::s_) { }
    struct S1 : Combinator {
        S1() : Combinator(Combinator::s1_) { }
        struct S2 : Combinator {
            S2() : Combinator(Combinator::s2_) { }
            CombinatorPtr apply(const CombinatorPtr& z) const;
            CombinatorPtr x;
            CombinatorPtr y;
        };
        CombinatorPtr x;
        CombinatorPtr apply(const CombinatorPtr& y) const;
    };
    CombinatorPtr apply(const CombinatorPtr& x) const;
};

class P : public Combinator {
public:
    P() : Combinator(Combinator::p_) { m_stream = 0; }
    CombinatorPtr apply(const CombinatorPtr& x) const;
    void setStream(QTextStream* stream);
private:
    QTextStream* m_stream;
};

struct R : Combinator {
    R() : Combinator(Combinator::r_) { }
    struct R1 : Combinator {
        R1() : Combinator(Combinator::r1_) { }
        CombinatorPtr apply(const CombinatorPtr& y) const;
        CombinatorPtr x;
    };
    CombinatorPtr apply(const CombinatorPtr& x) const;
};

struct A : Combinator {
    A() : Combinator(Combinator::a_), isThunk(false) { }
    CombinatorPtr apply() const;
    CombinatorPtr apply(const CombinatorPtr& x) const;

    bool isFull() const;
    bool isWellFormed() const;
    bool doNotCache() const;
    void addCombinator(const CombinatorPtr&);
    CombinatorPtr left;
    CombinatorPtr right;
    bool isThunk;
};

// optimizing combinators

class SubEval {
public:
    SubEval();
    SubEval(const SubEval& other);
    ~SubEval();

    void clear();
    void replacePrefix(const QString& prefix);
    void addPrefix(const QString& prefix);
    void addPostfix(const QString& postfix);

private:
    mutable int m_prefixNumber;
    mutable int m_postfixNumber;
};


#endif // combinators_h
