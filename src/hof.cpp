#include "hof.h"

#include "cache.h"
#include "combinators.h"
#include "colors.h"
#include "verbose.h"

void cppInterpreter(const QString& string)
{
    Verbose::instance()->generateProgramString("hof: " + string);
    Verbose::instance()->generateProgramString("begin");

    if (string.isEmpty()) {
        Verbose::instance()->generateProgramString("end");
        return;
    }

    CombinatorPtr evaluate;
    CombinatorPtr application;
    for (int x = 0; x < string.length(); x++) {
        CombinatorPtr term;
        QChar ch = string.at(x);
        switch (ch.unicode()) {
        case 'I': term = i(); break;
        case 'K': term = k(); break;
        case 'S': term = s(); break;
        case 'P': term = p(); break;
        case 'R': term = r(); break;
        case 'A': term = CombinatorPtr(new A); break;
        default: term = CombinatorPtr(new Var(ch)); break;
        }

        if (application.isNull() && term->type() == Combinator::a_) {
            application = term;
            continue;
        }

        if (!application.isNull()) {
            A* a = static_cast<A*>(application.data());
            Q_ASSERT(!a->isWellFormed());
            a->addCombinator(term);
            if (!a->isWellFormed())
                continue;

            term = application;
            application = CombinatorPtr();
        }

        if (evaluate.isNull()) {
            evaluate = term;
            continue;
        }

        evaluate = eval(evaluate, term);
    }

    while (!evaluate.isNull() && evaluate->type() == Combinator::a_) {
        A* a = static_cast<A*>(evaluate.data());
        if (!a->isWellFormed()) { break; }
            evaluate = a->apply();
    }

    Verbose::instance()->generateInputString(application);
    Verbose::instance()->generateReturnString(evaluate);
    Verbose::instance()->generateProgramEnd();
}

Hof::Hof(QTextStream* outputStream)
{
    static_cast<P*>(p().data())->setStream(outputStream);
}

Hof::~Hof()
{ }

void Hof::run(const QString& string)
{
    cppInterpreter(string);
}

