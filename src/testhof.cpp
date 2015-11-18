#include <QtCore>
#include <random>

#include "testhof.h"

enum Expectation {
    Normal,  // expect no timeout and no crash
    NoCrash, // expect no crash
    Timeout  // expect timeout
};

QString runHof(const QString& program,
               bool* ok,
               bool verbose = false,
               int msecsToTimeout = 5000,
               Expectation e = Expectation::Normal,
               const QString& translate = "")
{
    QDir bin(QCoreApplication::applicationDirPath());

    QProcess hof;
    hof.setProgram(bin.path() + QDir::separator() + "hof");

    QStringList args = QStringList()
      << "--program"
      << program;

    if (!translate.isEmpty()) {
        args.append("--translate");
        args.append(translate);
    }

    if (verbose) {
        args.append("--verbose");
        hof.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    }

    hof.setArguments(args);
    hof.start();
    bool finished = hof.waitForFinished(msecsToTimeout);
    if (!finished) {
        hof.kill();
        hof.waitForFinished();
    }

    /* special value used to indicate stack depth exceeded */
    switch (e) {
    case Expectation::Normal:
        {
            *ok = finished && hof.exitStatus() == QProcess::NormalExit &&
                  (hof.exitCode() == EXIT_SUCCESS || hof.exitCode() == 2);
            break;
        }
    case Expectation::NoCrash:
        {
            *ok = !finished || (hof.exitStatus() == QProcess::NormalExit &&
                  (hof.exitCode() == EXIT_SUCCESS || hof.exitCode() == 2));
            break;
        }
    case Expectation::Timeout:
        {
            *ok = !finished;
            break;
        }
    default:
        *ok = false;
        break;
    }

    if (!*ok) {
        qDebug() << "exit status:" << hof.exitStatus()
                 << " code:" << hof.exitCode()
                 << " error:" << hof.error();
    }

    return hof.readAll().trimmed();
}

#define I "I"
#define K "K"
#define S "S"
#define V "V"
#define P "P"
#define R "R"
#define A "A"
#define APPLY(X) A X
#define PRINT(X) P X
#define RANDOM(X, Y) R X Y
#define TRUE K
#define FALSE A K I
#define IF(X, Y, Z) X Y Z
#define IFNOT(X, Y, Z) X Z Y
#define AND(X, Y) X Y FALSE
#define OR(X, Y) X TRUE Y
#define ZERO FALSE
#define ONE I
#define SUCC(X) A A S A A S A K S K X
#define OMEGA S I I A A S I I

// standard beta recursion combinator
#define BETA_RECURSE(X) S A K X A A S I I A A S A K X A A S I I

// standard y combinator
#define YCOMBINATOR(X) S A K A A S I I A A S A A S A K S K A K A A S I I X

#define WHILE(COND, OPERATION, INITIAL) ""

void TestHof::testHof()
{
    bool ok = false;
    QString out;

    // print
    out = runHof(PRINT(I), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);

    // if-then-else
    out = runHof(IF(TRUE, APPLY(PRINT(I)), APPLY(PRINT(K))), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);

    out = runHof(IF(FALSE, APPLY(PRINT(I)), APPLY(PRINT(K))), &ok);
    QCOMPARE(out, QString(K));
    QVERIFY(ok);

    // if-not-then-else
    out = runHof(IFNOT(TRUE, APPLY(PRINT(I)), APPLY(PRINT(K))), &ok);
    QCOMPARE(out, QString(K));
    QVERIFY(ok);

    out = runHof(IFNOT(FALSE, APPLY(PRINT(I)), APPLY(PRINT(K))), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);

    //if-and-then-else
    out = runHof(IF(AND(TRUE, TRUE), APPLY(PRINT(I)), APPLY(PRINT(K))), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);

    out = runHof(IF(AND(TRUE, FALSE), APPLY(PRINT(I)), APPLY(PRINT(K))), &ok);
    QCOMPARE(out, QString(K));
    QVERIFY(ok);

    // if-or-then-else
    out = runHof(IF(OR(TRUE, FALSE), APPLY(PRINT(I)), APPLY(PRINT(K))), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);

    out = runHof(IF(OR(FALSE, TRUE), APPLY(PRINT(I)), APPLY(PRINT(K))), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);

    out = runHof(IF(OR(FALSE, FALSE), APPLY(PRINT(I)), APPLY(PRINT(K))), &ok);
    QCOMPARE(out, QString(K));
    QVERIFY(ok);

    // church numerals
    out = runHof(QString(SUCC(ONE)) + PRINT(I), &ok);
    QCOMPARE(out, QString("II"));
    QVERIFY(ok);

    out = runHof(QString(SUCC(SUCC(ONE))) + PRINT(I), &ok);
    QCOMPARE(out, QString("III"));
    QVERIFY(ok);

    out = runHof(QString(SUCC(SUCC(SUCC(ONE)))) + PRINT(I), &ok);
    QCOMPARE(out, QString("IIII"));
    QVERIFY(ok);

    out = runHof(QString(SUCC(SUCC(SUCC(SUCC(ONE))))) + PRINT(I), &ok);
    QCOMPARE(out, QString("IIIII"));
    QVERIFY(ok);

    // print random
    out = runHof(RANDOM(APPLY(PRINT(I)), APPLY(PRINT(K))), &ok);
    QVERIFY2(out == "I" || out == "K", qPrintable(out));
    QVERIFY(ok);
}

void TestHof::testY()
{
    bool ok = false;
    QString out;
    int timeout = 500;
    bool verbose = false;

    out = runHof(YCOMBINATOR("API"), &ok, verbose, timeout, Expectation::Timeout);
    QVERIFY(out.count('I') > 1);
    out.replace("I", "");
    QCOMPARE(out, QString());
    QVERIFY(ok);
}

bool testYBenchmarkImplemented()
{
    QDir bin(QCoreApplication::applicationDirPath());

    QProcess hof;
    hof.setProgram(bin.path() + QDir::separator() + "hof");

    QStringList args = QStringList()
      << "--program"
      << YCOMBINATOR("API");

    static bool verbose = false;
    if (verbose) {
        args.append("--verbose");
        hof.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    }

    hof.setArguments(args);
    hof.start();

    static qint64 total = 1000;
    qint64 totalRead = 0;
    while (hof.waitForReadyRead() && totalRead < total) {
        qint64 bytesAvailable = hof.bytesAvailable();
        qint64 remaining = qMin(total - totalRead, bytesAvailable);
        QByteArray output = hof.read(remaining);
        qint64 size = output.size();
        for (qint64 i = 0; i < size; ++i) {
            if (output.at(i) != 'I')
                return false;
            totalRead++;
        }
    }

    hof.kill();
    hof.waitForFinished();
    return true;
}

void TestHof::testYBenchmark()
{
    int iterations = 10;
    QList<int> results;

    for (int i = 0; i < iterations; ++i) {
        QElapsedTimer timer;
        timer.start();
        testYBenchmarkImplemented();
        results.append(timer.elapsed());
    }

    int totalTime = 0;
    {
        QList<int>::const_iterator it = results.begin();
        for (; it != results.end(); ++it)
            totalTime += *it;
    }

    qreal mean = totalTime / iterations;
    qreal sumOfSquares = 0;
    {
        QList<int>::const_iterator it = results.begin();
        for (; it != results.end(); ++it)
            sumOfSquares += (qreal(*it) - mean) * (qreal(*it) - mean);
    }

    qreal variance = sumOfSquares / qreal(iterations) - 1;
    qreal stdDeviation = qSqrt(variance);

    qDebug() << "iterations" << iterations
             << "time" << mean
             << "deviation" << stdDeviation;

#ifdef NDEBUG
    QVERIFY(mean < 75);
#else
    QVERIFY(mean < 80);
#endif
}

void TestHof::testOmega()
{
    bool ok = false;
    QString out;
    int timeout = 500;
    bool verbose = false;

    out = runHof(OMEGA, &ok, verbose, timeout, Expectation::Timeout);
    QCOMPARE(out, QString());
    QVERIFY(ok);
}

void TestHof::testHofNoise()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distLength(0, 100);
    std::uniform_int_distribution<int> distLetter(0, 5);

    for (int i = 0; i < 100; ++i) {
        int length = distLength(gen);
        QString randomHofProgram;
        for (int j = 0; j < length; ++j) {
            int letter = distLetter(gen);
            QChar ch;
            switch (letter) {
            case 0: ch = 'I'; break;
            case 1: ch = 'K'; break;
            case 2: ch = 'S'; break;
            case 3: ch = 'A'; break;
            case 4: ch = 'P'; break;
            case 5: ch = 'R'; break;
            default:
              Q_ASSERT(false);
            }
            randomHofProgram.append(ch);
        }
        bool ok = false;
        runHof(randomHofProgram, &ok, false /*verbose*/, 500 /*timeout*/, Expectation::NoCrash);
        QVERIFY2(ok, qPrintable(randomHofProgram));
    }
}

void TestHof::testTranslateSki()
{
    bool ok = false;
    QString out;

    QString skiYCombinator = "S(K(SII))(S(S(KS)K)(K(SII)))";
    out = runHof(skiYCombinator, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "ski" /*translate*/);
    QCOMPARE(out, QString(YCOMBINATOR("")));
    QVERIFY(ok);
}

void TestHof::testTranslateLambda()
{
    bool ok = false;
    QString out;

    QString lambdaIdentity = "λx.x";
    out = runHof(lambdaIdentity, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "lambda" /*translate*/);
    QCOMPARE(out, QString("I"));
    QVERIFY(ok);

    QString lambdaReverser = "λx.λy.(y x)";
    out = runHof(lambdaReverser, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "lambda" /*translate*/);
    QCOMPARE(out, QString("AASAKASIK")); // with η-reduction
    QVERIFY(ok);

    QString lambdaIdentity2 = "λf.λx.(f x)";
    out = runHof(lambdaIdentity2, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "lambda" /*translate*/);
    QCOMPARE(out, QString("I")); // with η-reduction
    QVERIFY(ok);
}
