## Introduction

Hof is a new language based on the SKI calculus named after Douglas Hofstadter.

    Syntax:       Semantics:    Meaning:
    F -> I        ^x.x          I combinator from SKI calculus
    F -> K        ^x.y.x        K combinator from SKI calculus
    F -> S        ^xyz.xz(yz)   S combinator from SKI calculus
    F -> V        ^v.^x.x       Void combinator ignores v and returns I
    F -> P        ^x.x          Print x to stdout
    F -> R        ^xy.(x|y)     Randomly return either x or y with Bernoulli probability distribution
    F -> A F₁ F₂  (F₁F₂)        Form a new term out of the application of terms F₁ and F₂
