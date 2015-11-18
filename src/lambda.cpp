#include "lambda.h"
#include "ski.h"

#define LAMBDA 0x03BB
#define DOT 0x002E

class Token {
public:
    enum Type {
        None,
        Lambda,
        Dot,
        Variable,
        LParen,
        RParen,
        Space
    };

    Token() : m_type(None) { }
    Token(Type t, QChar c) : m_type(t), m_char(c) { }

    Type type() const { return m_type; }
    QChar character() const { return m_char; }
    QString toString() const
    {
        QString r;
        switch (m_type) {
        case None: r = QStringLiteral("None"); break;
        case Lambda: r = QStringLiteral("Lambda"); break;
        case Dot: r = QStringLiteral("Dot"); break;
        case Variable: r = QStringLiteral("Variable"); break;
        case LParen: r = QStringLiteral("LParen"); break;
        case RParen: r = QStringLiteral("RParen"); break;
        case Space: r = QStringLiteral("Space"); break;
        }
        return r + ": '" + m_char + "'";
    }

private:
    Type m_type;
    QChar m_char;
};

struct LambdaTerm {
    enum Type { Variable, Abstraction, Application, Ski };
    virtual ~LambdaTerm() { }
    virtual Type type() const = 0;
    virtual QString toString() const  = 0;

    // Implemented with the transformation rules found here:
    // https://en.wikipedia.org/wiki/Combinatory_logic#Completeness_of_the_S-K_basis
    virtual LambdaTerm* toSki() = 0;
};

struct Combinator : LambdaTerm {
    QChar ski;
    Combinator(const QChar& ch) : ski(ch) {}
    virtual Type type() const { return Ski; }
    virtual QString toString() const  { return ski; }
    virtual LambdaTerm* toSki() { return this; }
};

struct LambdaVariable : LambdaTerm {
    Token token;

    virtual Type type() const { return Variable; }

    virtual QString toString() const
    {
        return token.character();
    }

    virtual LambdaTerm* toSki()
    {
        // rule #1
        return this;
    }
};

struct LambdaApplication : LambdaTerm {
    LambdaTerm* left;
    LambdaTerm* right;

    LambdaApplication()
        : left(0)
        , right(0) { }

    virtual ~LambdaApplication()
    {
        delete left;
        delete right;
    }

    virtual Type type() const { return Application; }

    virtual QString toString() const
    {
        Q_ASSERT(left);
        Q_ASSERT(right);
        return QStringLiteral("(") +
               left->toString() + QStringLiteral(" ") +
               right->toString() + QStringLiteral(")");
    }

    virtual LambdaTerm* toSki()
    {
        // rule # 2
        left = left->toSki();
        right = right->toSki();
        return this;
    }
};

struct LambdaAbstraction : LambdaTerm {
    LambdaVariable* variable;
    LambdaTerm* body;
    bool owner;

    bool isFree() const { return body->toString().contains(variable->toString()); }

    LambdaAbstraction(bool owner = true)
        : variable(0)
        , body(0)
        , owner(owner) { }

    virtual ~LambdaAbstraction()
    {
        if (!owner)
            return;

        delete variable;
        delete body;
    }

    virtual Type type() const { return Abstraction; }

    virtual QString toString() const
    {
        Q_ASSERT(variable);
        Q_ASSERT(body);
        return QString(LAMBDA) + variable->toString() + QString(DOT) + body->toString();
    }

    virtual LambdaTerm* toSki()
    {
        // η-reduction
        if (body->type() == Application) {
            LambdaApplication* a = static_cast<LambdaApplication*>(body);
            if (variable->toString() == a->right->toString() &&
                !a->left->toString().contains(variable->toString())) {
                return a->left->toSki();
            }
        }

        // rule #3
        if (!isFree()) {
            LambdaApplication* a = new LambdaApplication;
            a->left = new Combinator('K');
            a->right = body->toSki();
            return a;
        }

        // rule #4
        if (body->type() == Variable && variable->toString() == body->toString())
            return new Combinator('I');

        // rule #5
        if (body->type() == Abstraction && isFree()) {
            body = body->toSki();
            return toSki();
        }

        // rule #6
        if (body->type() == Application && isFree()) {
            LambdaApplication* a = static_cast<LambdaApplication*>(body);

            LambdaAbstraction* leftA = new LambdaAbstraction;
            leftA->variable = new LambdaVariable;
            leftA->variable->token = variable->token;
            leftA->body = a->left;

            LambdaApplication* leftApp = new LambdaApplication;
            leftApp->left = new Combinator('S');
            leftApp->right = leftA;

            LambdaAbstraction* rightA = new LambdaAbstraction;
            rightA->variable = new LambdaVariable;
            rightA->variable->token = variable->token;
            rightA->body = a->right;

            a->left = leftApp;
            a->right = rightA;
            return a->toSki();
        }

        Q_ASSERT(false);
        return 0;
    }
};

class LambdaParser {
public:
    LambdaParser(const QList<Token>& tokens)
        : m_tokens(tokens)
        , m_index(-1) { }
    ~LambdaParser() { qDeleteAll(m_terms); }

    void parse();
    QStringList errors() const { return m_errors; }
    QList<LambdaTerm*> terms() const { return m_terms; }

private:
    Token current() const;
    Token advance(int i);
    Token look(int i) const;
    bool expect(Token tok, Token::Type type);

    LambdaTerm* parseLambdaTerm();
    LambdaVariable* parseLambdaVariable();
    LambdaAbstraction* parseLambdaAbstraction();
    LambdaApplication* parseLambdaApplication();

    void error();

    QStringList m_errors;
    QList<LambdaTerm*> m_terms;
    QList<Token> m_tokens;
    int m_index;
};

QString Lambda::fromLambda(const QString& string)
{
    // Lexer
    QList<Token> tokens;
    for (int x = 0; x < string.length(); x++) {
        QChar ch = string.at(x);
        Token::Type t;
        switch (ch.unicode()) {
        case LAMBDA: t = Token::Lambda; break;
        case DOT: t = Token::Dot; break;
        case '(': t = Token::LParen; break;
        case ')': t = Token::RParen; break;
        case ' ': t = Token::Space; break;
        default: t = Token::Variable; break;
        };
        tokens.append(Token(t, ch));
    }

    LambdaParser parser(tokens);
    parser.parse();

    QStringList errors = parser.errors();
    if (!errors.isEmpty())
        return errors.join("\n");

    QString out;
    QList<LambdaTerm*> terms = parser.terms();
    foreach (LambdaTerm* term, terms) {
        LambdaTerm* t = term->toSki();
        out.append(t->toString());;
    }

    out = out.simplified();
    out.replace(" ", "");
    return Ski::fromSki(out);
}

void LambdaParser::parse()
{
    if (m_tokens.isEmpty())
        return;

    while (m_index < m_tokens.length() - 1) {
        advance(1);
        LambdaTerm* term = parseLambdaTerm();
        if (term)
            m_terms.append(term);
    }
}

Token LambdaParser::current() const
{
    return m_tokens.at(m_index);
}

Token LambdaParser::advance(int i)
{
    m_index += i;
    return m_tokens.at(m_index);
}

Token LambdaParser::look(int i) const
{
    int index = m_index + i;
    if (index >= m_tokens.count())
        return Token();
    return m_tokens.at(index);
}

bool LambdaParser::expect(Token token, Token::Type type)
{
    if (token.type() == type)
        return true;

    error();
    return false;
}

LambdaTerm* LambdaParser::parseLambdaTerm()
{
    switch (current().type()) {
    case Token::Variable: return parseLambdaVariable();
    case Token::Lambda: return parseLambdaAbstraction();
    case Token::LParen: return parseLambdaApplication();
    default: error(); break;
    }
    return 0;
}

LambdaVariable* LambdaParser::parseLambdaVariable()
{
    Token token = current();
    if (!expect(token, Token::Variable))
        return 0;

    LambdaVariable* v = new LambdaVariable;
    v->token = token;
    return v;
}

LambdaAbstraction* LambdaParser::parseLambdaAbstraction()
{
    advance(1);

    LambdaVariable* variable = parseLambdaVariable();
    if (!variable)
        return 0;

    Token dot = advance(1);
    if (!expect(dot, Token::Dot))
        return 0;

    advance(1);
    LambdaTerm* body = parseLambdaTerm();
    if (!body)
        return 0;

    LambdaAbstraction* a = new LambdaAbstraction;
    a->variable = variable;
    a->body = body;
    return a;
}

LambdaApplication* LambdaParser::parseLambdaApplication()
{
    advance(1);

    LambdaTerm* left = parseLambdaTerm();
    if (!left)
        return 0;

    Token space = advance(1);
    if (!expect(space, Token::Space)) {
        delete left;
        return 0;
    }

    advance(1);
    LambdaTerm* right = parseLambdaTerm();
    if (!right)
        return 0;

    Token close = advance(1);
    if (!expect(close, Token::RParen)) {
        delete left;
        delete right;
        return 0;
    }

    LambdaApplication* a = new LambdaApplication;
    a->left = left;
    a->right = right;
    return a;
}

void LambdaParser::error()
{
    m_errors.append(QString("Unexpected token: {%1} at index: %2")
                    .arg(current().toString())
                    .arg(m_index));
}
