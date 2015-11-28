#ifndef combinators_h
#define combinators_h

#include <QtCore>

class Combinator;
typedef QSharedPointer<Combinator> CombinatorPtr;

// singleton combinators
CombinatorPtr i();
CombinatorPtr k();
CombinatorPtr s();
CombinatorPtr p();
CombinatorPtr r();
CombinatorPtr b();
CombinatorPtr c();

// general evaluation function
CombinatorPtr eval(const CombinatorPtr& left, const CombinatorPtr& right);

enum OutputFormat {
  None,
  Bash
};

class Combinator {
public:
    enum Type { i_, k_, s_, p_, r_, a_, b_, c_, capture_ };

    Combinator() : m_type(Type(-1)) { }
    Combinator(Type t) : m_type(t) { }
    ~Combinator() { }
    Type type() const { return m_type; }
    CombinatorPtr apply(const CombinatorPtr& x) const;
    QString toString() const;
    QString toStringApply(const CombinatorPtr& arg, OutputFormat f = None) const;
    QString typeToString() const;

private:
    Type m_type;
};

struct Capture : Combinator {
    Capture(const CombinatorPtr& c, int args)
        : Combinator(Combinator::capture_)
        , callback(c)
        , argsToCapture(args) { }

    bool isFull() const { return argsToCapture == args.length(); }
    void append(const CombinatorPtr& c);
    QList<CombinatorPtr> args;
    CombinatorPtr callback;
    int argsToCapture;

    CombinatorPtr x() const { Q_ASSERT(args.length() >= 1); return args.at(0); }
    CombinatorPtr y() const { Q_ASSERT(args.length() >= 2); return args.at(1); }
    CombinatorPtr z() const { Q_ASSERT(args.length() >= 3); return args.at(2); }
    CombinatorPtr k() const { Q_ASSERT(args.length() == 4); return args.at(3); }
};

struct I : Combinator {
    I() : Combinator(Combinator::i_) { }
    CombinatorPtr apply(const CombinatorPtr& x) const;
};

struct K : Combinator {
    K() : Combinator(Combinator::k_) { }
    CombinatorPtr apply(const CombinatorPtr& arg, CombinatorPtr cap = CombinatorPtr()) const;
};

struct S : Combinator {
    S() : Combinator(Combinator::s_) { }
    CombinatorPtr apply(const CombinatorPtr& arg, CombinatorPtr cap = CombinatorPtr()) const;
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
    CombinatorPtr apply(const CombinatorPtr& arg, CombinatorPtr cap = CombinatorPtr()) const;
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

struct B : Combinator {
    B() : Combinator(Combinator::b_) { }
    CombinatorPtr apply(const CombinatorPtr& arg, CombinatorPtr cap = CombinatorPtr()) const;
};

struct C : Combinator {
    C() : Combinator(Combinator::c_) { }
    CombinatorPtr apply(const CombinatorPtr& arg, CombinatorPtr cap = CombinatorPtr()) const;
};

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
