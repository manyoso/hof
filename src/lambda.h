#ifndef lambda_h
#define lambda_h

#include <QtCore>

class Lambda {
public:
    /**
     * Taken from the formal definition on
     * https://en.wikipedia.org/wiki/Lambda_calculus#Formal_definition
     *
     * Lambda expressions are defined as follows:
     *   1) variables v1, v2, ..., vn, ...
     *   2) the abstraction symbols lambda 'λ' and dot '.'
     *   3) the application symbols parenthesis '(', ')'
     *
     * The set of lambda expressions, Λ, can be defined inductively:
     *   1) If x is a variable, then x is in Λ
     *   2) If x is a variable and M is in Λ, then (λx.M) is in Λ
     *   3) If M, N are in Λ, then (M N) is in Λ
     *
     * Instances of rules #1, 2, and #3 are known as lambda terms.
     * Instances of rule #1 are known as lambda variables.
     * Instances of rule #2 are known as lambda abstractions.
     * Instances of rule #3 are known as lambda applications.
     *
     * Notation is strict and application requires parenthesis while
     * nested abstraction requires lambda and dot.  Variables are restricted to
     * single characters.
     */
    static QString fromLambda(const QString& lambda);
};

#endif // lambda_h
