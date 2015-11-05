## Introduction

Hof is a new language based on the SKI calculus named after Douglas Hofstadter.

Syntax:     Semantics:    Meaning:
F -> I      ^x.x          I combinator from SKI calculus
F -> K      ^x.y.x        K combinator from SKI calculus
F -> S      ^xyz.xz(yz)   S combinator from SKI calculus
F -> A F F  [F][F]        used to control evaluation order
F -> P      ^x.x          print x to stdout
F -> R      ^xy.(x|y)     system will randomly return either x or y
