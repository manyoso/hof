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

// built-in combinators
#define I "I"
#define K "K"
#define S "S"
#define V "V"
#define P "P"
#define R "R"
#define A "A"
#define PRINT(X)  P X
#define RANDOM(X, Y) R X Y
#define PTERM(X)  A P X

// boolean logic
#define TRUE K
#define FALSE "AKI"
#define IF(X, Y, Z) X Y Z
#define IFNOT(X, Y, Z) X Z Y
#define AND(X, Y) X Y FALSE
#define OR(X, Y) X TRUE Y

// p-numerals
#define PINC(X) "AASAASAKSAASAKASAKSAASAKASAKKAASAKASIKI" X
#define PDEC(X) "IKI" X

// church numerals
#define INC(X) "AASAASAKSK" X
#define DEC(X) "AASAASAKSAASAKASAKSAASAASAKSAASAKASAKSAASAKASAKKAASAASAKSKAKAASAKASAKASIAASAKASAKKAASAKASIKAKAKKAKAKAKI" X
#define ADD(M, N) N INC(M)
#define SUBTRACT(M, N) N DEC(M)
#define ZERO FALSE
#define ONE I
#define TWO INC(ONE)
#define THREE INC(TWO)
#define FOUR INC(THREE)
#define FIVE INC(FOUR)

// church comparison operators
#define ISZERO(X) "AASAASIAKAKAKIAKK" X

// church pairs and lists
#define PAIR(X, Y) "AASAASAKSAASAKKAASAKSAASAKASIKAKK" X Y
#define FIRST(X) "AASIAK" TRUE X
#define SECOND(X) "AASIAK" FALSE X
#define CONS PAIR
#define HEAD FIRST
#define TAIL SECOND
#define NIL FALSE
#define ISNIL ISZERO

// recursion
#define OMEGA "SIIAASII"

// standard y combinator
#define Y(X) "SAKAASIIAASAASAKSKAKAASII" X
#define Y1(X) "SSKAASAKAASSASAASSKK" X

// standard beta recursion combinator
#define BETA(X) "SAK" X "AASII"
#define BETA_RECURSE(X) BETA(X) "AA" BETA(X)

// while loop
#define WHILE(COND, OPERATION, INITIAL) "AASAKASAKAASAKAASIIAASAASAKSKAKAASIIAASAASAKSAASAKASAKSAASAKASAKASAKSAASAASAKSAASAKASAKSAASAKASAKASAKSAASAASAKSAASAKKAASAKSAASAKKSAKAASAKASAKASAKKAASAKASAASAKSKKAKAKAKKAKAKAKAKAKI" COND OPERATION INITIAL

void TestHof::testPrint()
{
    bool ok = false;
    QString out;

    // print
    out = runHof(PRINT(I), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);
}

void TestHof::testChurchLogic()
{
    bool ok = false;
    QString out;

    // if-then-else
    out = runHof(IF(TRUE, PTERM(I), PTERM(K)), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);

    out = runHof(IF(FALSE, PTERM(I), PTERM(K)), &ok);
    QCOMPARE(out, QString(K));
    QVERIFY(ok);

    // if-not-then-else
    out = runHof(IFNOT(TRUE, PTERM(I), PTERM(K)), &ok);
    QCOMPARE(out, QString(K));
    QVERIFY(ok);

    out = runHof(IFNOT(FALSE, PTERM(I), PTERM(K)), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);

    //if-and-then-else
    out = runHof(IF(AND(TRUE, TRUE), PTERM(I), PTERM(K)), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);

    out = runHof(IF(AND(TRUE, FALSE), PTERM(I), PTERM(K)), &ok);
    QCOMPARE(out, QString(K));
    QVERIFY(ok);

    // if-or-then-else
    out = runHof(IF(OR(TRUE, FALSE), PTERM(I), PTERM(K)), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);

    out = runHof(IF(OR(FALSE, TRUE), PTERM(I), PTERM(K)), &ok);
    QCOMPARE(out, QString(I));
    QVERIFY(ok);

    out = runHof(IF(OR(FALSE, FALSE), PTERM(I), PTERM(K)), &ok);
    QCOMPARE(out, QString(K));
    QVERIFY(ok);
}

void TestHof::testChurchNumerals()
{
    bool ok = false;
    QString out;

    // church numerals
    out = runHof(QString(TWO) + PRINT(I), &ok);
    QCOMPARE(out, QString("II"));
    QVERIFY(ok);

    out = runHof(QString(THREE) + PRINT(I), &ok);
    QCOMPARE(out, QString("III"));
    QVERIFY(ok);

    out = runHof(QString(FOUR) + PRINT(I), &ok);
    QCOMPARE(out, QString("IIII"));
    QVERIFY(ok);

    out = runHof(QString(FIVE) + PRINT(I), &ok);
    QCOMPARE(out, QString("IIIII"));
    QVERIFY(ok);

    // decrement
    out = runHof(QString(DEC(FIVE)) + PRINT(I), &ok);
    QCOMPARE(out, QString("IIII"));
    QVERIFY(ok);

    // add
    out = runHof(QString(ADD(ONE, ONE)) + PRINT(I), &ok);
    QCOMPARE(out, QString("II"));
    QVERIFY(ok);

    // subtract
    out = runHof(QString(SUBTRACT(THREE, ONE)) + PRINT(I), &ok);
    QCOMPARE(out, QString("II"));
    QVERIFY(ok);
}

void TestHof::testChurchComparison()
{
    bool ok = false;
    QString out;

    // comparison operators
    out = runHof(QString(IF(ISZERO(ZERO), PTERM(I), PTERM(K))), &ok);
    QCOMPARE(out, QString("I"));
    QVERIFY(ok);

    out = runHof(QString(IF(ISZERO(ONE), PTERM(I), PTERM(K))), &ok);
    QCOMPARE(out, QString("K"));
    QVERIFY(ok);
}

void TestHof::testChurchPairsAndLists()
{
    bool ok = false;
    QString out;

    out = runHof(QString(IF("A" ISNIL("A" FIRST("AA" CONS(NIL, ONE))), PTERM(I), PTERM(K))), &ok);
    QCOMPARE(out, QString("I"));
    QVERIFY(ok);

    out = runHof(QString(IF("A" ISNIL("A" FIRST("AA" CONS(ONE, NIL))), PTERM(I), PTERM(K))), &ok);
    QCOMPARE(out, QString("K"));
    QVERIFY(ok);
}

void TestHof::testRandom()
{
    bool ok = false;
    QString out;

    // print random
    out = runHof(RANDOM(PTERM(I), PTERM(K)), &ok);
    QVERIFY2(out == "I" || out == "K", qPrintable(out));
    QVERIFY(ok);

}

void TestHof::testY()
{
    bool ok = false;
    QString out;
    int timeout = 500;
    bool verbose = false;

    out = runHof(Y("API"), &ok, verbose, timeout, Expectation::Timeout);
    QVERIFY(out.count('I') > 1);
    out.replace("I", "");
    QCOMPARE(out, QString());
    QVERIFY(ok);
}

bool testYBenchmarkImplemented(QElapsedTimer* timer)
{
    QDir bin(QCoreApplication::applicationDirPath());

    QProcess hof;
    hof.setProgram(bin.path() + QDir::separator() + "hof");

    QStringList args = QStringList()
      << "--program"
      << Y("API");

    static bool verbose = false;
    if (verbose) {
        args.append("--verbose");
        hof.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    }

    hof.setArguments(args);
    hof.start();

    static qint64 total = 1000;
    qint64 totalRead = 0;
    bool timerStart = false;
    while (hof.waitForReadyRead() && totalRead < total) {
        qint64 bytesAvailable = hof.bytesAvailable();
        qint64 remaining = qMin(total - totalRead, bytesAvailable);
        QByteArray output = hof.read(remaining);
        qint64 size = output.size();
        for (qint64 i = 0; i < size; ++i) {
            if (output.at(i) != 'I')
                return false;

            if (!timerStart && totalRead > 1) {
                timer->start();
                timerStart = true;
            }

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
        testYBenchmarkImplemented(&timer);
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
    QCOMPARE(out, QString(Y("")));
    QVERIFY(ok);
}

void TestHof::testTranslateLambda()
{
    bool ok = false;
    QString out;

    QString lambdaI = "λx.x";
    out = runHof(lambdaI, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "lambda" /*translate*/);
    QCOMPARE(out, QString("I"));
    QVERIFY(ok);

    QString lambdaK = "λx.λy.x";
    out = runHof(lambdaK, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "lambda" /*translate*/);
    QCOMPARE(out, QString("K"));
    QVERIFY(ok);

    QString lambdaS = "λx.λy.λz.xz(yz)";
    out = runHof(lambdaS, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "lambda" /*translate*/);
    QCOMPARE(out, QString("S"));
    QVERIFY(ok);

    QString lambdaFalse = "λx.λy.y";
    out = runHof(lambdaFalse, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "lambda" /*translate*/);
    QCOMPARE(out, QString("AKI"));
    QVERIFY(ok);

    QString lambdaReverser = "λx.λy.yx";
    out = runHof(lambdaReverser, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "lambda" /*translate*/);
    QCOMPARE(out, QString("AASAKASIK"));
    QVERIFY(ok);

    QString lambdaIdentity = "λf.λx.fx";
    out = runHof(lambdaIdentity, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "lambda" /*translate*/);
    QCOMPARE(out, QString("I"));
    QVERIFY(ok);

    QString lambdaIsZero = "λn.n (λx.λx.λy.y) λx.λy.x";
    out = runHof(lambdaIsZero, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "lambda" /*translate*/);
    QCOMPARE(out, QString("AASAASIAKAKAKIAKK"));
    QVERIFY(ok);

    QString pred = "λn.λf.λx.n (λg.λh.h (g f)) (λu.x) (λu.u)";
    out = runHof(pred, &ok, false /*verbose*/, 5000 /*timeout*/, Expectation::Normal, "lambda" /*translate*/);
    QCOMPARE(out, QString("AASAASAKSAASAKASAKSAASAASAKSAASAKASAKSAASAKASAKKAASAASAKSKAKAASAKASAKASIAASAKASAKKAASAKASIKAKAKKAKAKAKI"));
    QVERIFY(ok);
}

