## Introduction

Hof is a new language based on the SKI calculus named after Douglas Hofstadter.

    Syntax:     Semantics:    Meaning:
    F -> I      ^x.x          I combinator from SKI calculus
    F -> K      ^x.y.x        K combinator from SKI calculus
    F -> S      ^xyz.xz(yz)   S combinator from SKI calculus
    F -> V      ^v.^x.x       V combinator ignores v and returns I
    F -> P      ^x.x          print x to stdout
    F -> R      ^xy.(x|y)     system will randomly return either x or y
    F -> A F F  (FF)          used to control evaluation order
