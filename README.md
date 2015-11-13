## Introduction

Hof is a new language based on the SKI calculus named after Douglas Hofstadter.

    Syntax:   Semantics:        Meaning:
    T -> I    λx.x              I combinator from SKI calculus
    T -> K    λx.λy.x           K combinator from SKI calculus
    T -> S    λx.λy.λz.xz(yz)   S combinator from SKI calculus
    T -> V    λv.λx.x           Void combinator ignores v and returns I
    T -> P    λx.x              Print x to stdout

    T -> R    λxy.(x|y)         Randomly return either x or y with Bernoulli
                                probability distribution

    T -> A    (T₁T₂)            Form a new term out of the application of
                                terms T₁ and T₂

The interpreter for the language is written in C++ and features lazy evaluations
implemented with memoized thunks.
