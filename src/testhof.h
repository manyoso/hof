#include <QtTest/QtTest>

class TestHof: public QObject {
    Q_OBJECT
private slots:
    void testPrint();
    void testLogic();
    void testChurch();
    void testComparison();
    void testRandom();
    void testY();
    void testYBenchmark();
    void testOmega();
    void testHofNoise();
    void testTranslateSki();
    void testTranslateLambda();
};
