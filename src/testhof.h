#include <QtTest/QtTest>

class TestHof: public QObject {
    Q_OBJECT
private slots:
    void testHof();
    void testY();
    void testYBenchmark();
    void testOmega();
    void testHofNoise();
    void testTranslateSki();
    void testTranslateLambda();
};
