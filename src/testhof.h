#ifndef testhof_h
#define testhof_h

#include <QtTest/QtTest>

class TestHof: public QObject {
    Q_OBJECT
private slots:
    void testPrint();
    void testChurchLogic();
    void testChurchNumerals();
    void testChurchComparison();
    void testChurchPairsAndLists();
    void testRandom();
    void testY();
    void testYBenchmark();
    void testOmega();
    void testHofNoise();
    void testTranslateSki();
    void testTranslateLambda();
    void testExamples();
};

#endif // testhof_h
