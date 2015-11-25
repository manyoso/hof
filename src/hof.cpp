#include "hof.h"

#include "cache.h"
#include "combinators.h"
#include "colors.h"
#include "verbose.h"

bool evaluationListIsWellFormed(const EvaluationList& list)
{
    EvaluationList::const_iterator it = list.begin();
    for (; it != list.end(); ++it) {
        if ((*it)->type() == Combinator::a_)
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
        CombinatorPtr term;
        QChar ch = string.at(x);
        switch (ch.unicode()) {
        case 'I': term = i(); break;
        case 'K': term = k(); break;
        case 'S': term = s(); break;
        case 'P': term = p(); break;
        case 'R': term = r(); break;
        case 'A': term = CombinatorPtr(new A); break;
        default:
            QString error = QString("Error: invalid char in hof program! ch=`%1`").arg(ch);
            Q_ASSERT_X(false, "hof", qPrintable(error));
            return;
        }

        if (evaluationList.isEmpty()) {
            evaluationList.append(term);
            continue;
        }

        if (term->type() == Combinator::a_) {
            if (evaluationList.back()->type() == Combinator::a_) {
                A* a = static_cast<A*>(evaluationList.back().data());
                Q_ASSERT(!a->isWellFormed());
                a->addCombinator(term);
            } else {
                evaluationList.append(term);
            }

            continue; // can't possibly be well formed at this point
        }

        if (evaluationList.back()->type() == Combinator::a_) {
            A* a = static_cast<A*>(evaluationList.back().data());
            Q_ASSERT(!a->isWellFormed());
            a->addCombinator(term);
            term = CombinatorPtr(); // remove the term since it was added
        }

        if (!evaluationListIsWellFormed(evaluationList))
            continue;

        CombinatorPtr evaluate = evaluationList.takeFirst();
        if (Verbose::instance()->isVerbose()) {
            Verbose::instance()->removePostfix(postfixIndex);
            postfixIndex = Verbose::instance()->addPostfix(string.right(string.length() - x - 1));

            SubEval subEval;
            subEval.addPostfix(!term.isNull() ? term->toString() : QString());

            EvaluationList::const_iterator it = evaluationList.begin();
            for (; it != evaluationList.end(); ++it)
                evaluate = eval(evaluate, *it);
        } else {
            EvaluationList::const_iterator it = evaluationList.begin();
            for (; it != evaluationList.end(); ++it)
                evaluate = eval(evaluate, *it);
        }

        if (!term.isNull())
            evaluate = eval(evaluate, term);

        while (evaluate->type() == Combinator::a_) {
            A* a = static_cast<A*>(evaluate.data());
            if (!a->isWellFormed()) { break; }
            evaluate = a->apply();
        }

        evaluationList = EvaluationList() << evaluate;
    }

    Q_ASSERT(evaluationList.length() >= 1);

    CombinatorPtr evaluate = evaluationList.takeFirst();
    Verbose::instance()->generateReturnString(evaluate);
    Verbose::instance()->generateInputString(evaluationList);
    Verbose::instance()->generateProgramEnd();
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

