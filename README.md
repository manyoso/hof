## Introduction

Hof is a new language based on the SKI calculus named after Douglas Hofstadter.

    Syntax:       Semantics:    Meaning:
    T -> I        ^x.x          I combinator from SKI calculus
    T -> K        ^xy.x         K combinator from SKI calculus
    T -> S        ^xyz.xz(yz)   S combinator from SKI calculus
    T -> V        ^v.^x.x       Void combinator ignores v and returns I
    T -> P        ^x.x          Print x to stdout
    T -> R        ^xy.(x|y)     Randomly return either x or y with Bernoulli
                                probability distribution
    T -> A T₁ T₂  (T₁T₂)        Form a new term out of the application of
                                terms T₁ and T₂
