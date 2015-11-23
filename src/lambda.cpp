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
        Sub
    };

    static QString typeToString(Type type)
    {
        switch (type) {
        case None: return QStringLiteral("None");
        case Lambda: return QStringLiteral("Lambda");
        case Dot: return QStringLiteral("Dot");
        case Variable: return QStringLiteral("Variable");
        case LParen: return QStringLiteral("LParen");
        case RParen: return QStringLiteral("RParen");
        case Sub: return QStringLiteral("Sub");
        default: return QString();
        }
    }

    Token() : m_type(None) { }
    Token(Type t, const QString s) : m_type(t), m_token(s) { }

    Type type() const { return m_type; }
    QString token() const { return m_token; }
    QString toString() const
    {
        QString r = typeToString(m_type);
        return r + ": '" + (m_token.isEmpty() ? QString("\\0") : m_token) + "'";
    }

private:
    Type m_type;
    QString m_token;
};

struct LambdaTerm {
    enum Type { Variable, Abstraction, Application, Ski, Sub };
    virtual ~LambdaTerm() { }
    virtual Type type() const = 0;
    virtual QString toString() const  = 0;

    // Implemented with the transformation rules found here:
    // https://en.wikipedia.org/wiki/Combinatory_logic#Completeness_of_the_S-K_basis
    virtual LambdaTerm* toSki() = 0;
};

struct Substitution : LambdaTerm {
    QString sub;
    Substitution(const QString& s) : sub(s) {}
    virtual Type type() const { return Sub; }
    virtual QString toString() const  { return "{" + sub + "}"; }
    virtual LambdaTerm* toSki() { return this; }
};

struct Combinator : LambdaTerm {
    QString ski;
    Combinator(const QString& s) : ski(s) {}
    virtual Type type() const { return Ski; }
    virtual QString toString() const  { return ski; }
    virtual LambdaTerm* toSki() { return this; }
};

struct LambdaVariable : LambdaTerm {
    Token token;

    virtual Type type() const { return Variable; }

    virtual QString toString() const
    {
        return token.token();
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
            a->left = new Combinator("K");
            a->right = body->toSki();
            return a;
        }

        // rule #4
        if (body->type() == Variable && variable->toString() == body->toString())
            return new Combinator("I");

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
            leftApp->left = new Combinator("S");
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

    LambdaTerm* parseLambdaTermOrApplication();
    LambdaTerm* parseLambdaTerm();
    LambdaVariable* parseLambdaVariable();
    LambdaAbstraction* parseLambdaAbstraction();
    LambdaApplication* parseLambdaApplication();

    void error(Token::Type type = Token::None);

    QStringList m_errors;
    QList<LambdaTerm*> m_terms;
    QList<Token> m_tokens;
    int m_index;
};

QString Lambda::fromLambda(const QString& string)
{
    // Remove all whitespace
    QString program = string;
    program = program.simplified();
    program.replace("\n", "");
    program.replace(" ", "");

    bool isSub = false;
    QString sub = QString();

    // Lexer
    QList<Token> tokens;
    for (int x = 0; x < program.length(); x++) {
        QChar ch = program.at(x);
        Token::Type t;
        switch (ch.unicode()) {
        case LAMBDA: t = Token::Lambda; break;
        case DOT: t = Token::Dot; break;
        case '(': t = Token::LParen; break;
        case ')': t = Token::RParen; break;
        case '{': isSub = true; continue;
        case '}':
          {
              tokens.append(Token(Token::Sub, sub));
              isSub = false;
              continue;
          }
        default: t = Token::Variable; break;
        };

        if (isSub)
            sub.append(ch);
        else
            tokens.append(Token(t, ch));
    }

    LambdaParser parser(tokens);
    parser.parse();

    QStringList errors = parser.errors();
    if (!errors.isEmpty()) {
        QString error = "Found errors while parsing: " + program;
        return error + errors.join("\n");
    }

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
        LambdaTerm* term = parseLambdaTermOrApplication();
        if (term)
            m_terms.append(term);
    }
}

Token LambdaParser::current() const
{
    if (m_index >= m_tokens.count())
        return Token();
    return m_tokens.at(m_index);
}

Token LambdaParser::advance(int i)
{
    m_index += i;
    return current();
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

    error(type);
    return false;
}

LambdaTerm* LambdaParser::parseLambdaTermOrApplication()
{
    LambdaTerm* term = parseLambdaTerm();
    if (!term)
        return 0;

    Token ahead = look(1);
    while (ahead.type() == Token::Variable ||
           ahead.type() == Token::Lambda ||
           ahead.type() == Token::LParen) {

        advance(1);
        LambdaTerm* right = parseLambdaTerm();
        if (!right) break;

        LambdaApplication* a = new LambdaApplication;
        a->left = term;
        a->right = right;
        term = a;
        ahead = look(1);
    }

    return term;
}

LambdaTerm* LambdaParser::parseLambdaTerm()
{
    switch (current().type()) {
    case Token::Variable: return parseLambdaVariable();
    case Token::Lambda: return parseLambdaAbstraction();
    case Token::LParen:
        {
            advance(1);
            LambdaTerm* term = parseLambdaTermOrApplication();
            Token token = advance(1);
            if (!expect(token, Token::RParen)) {
                delete term;
                return 0;
            }
            return term;
        }
    case Token::Sub: return new Substitution(current().token());
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
    LambdaTerm* body = parseLambdaTermOrApplication();
    if (!body)
        return 0;

    LambdaAbstraction* a = new LambdaAbstraction;
    a->variable = variable;
    a->body = body;
    return a;
}

void LambdaParser::error(Token::Type expected)
{
    if (expected != Token::None) {
        m_errors.append(QString("Expected: {%1}, found: {%2} at index: %3")
                      .arg(Token::typeToString(expected))
                      .arg(current().toString())
                      .arg(m_index));
    } else {
        m_errors.append(QString("Unexpected token: {%1} at index: %2")
                        .arg(current().toString())
                        .arg(m_index));
    }
}
