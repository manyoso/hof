#include <QtCore>
#include <random>

#include "testhof.h"

QString runHof(const QString& program, bool* ok, bool verbose = false)
{
    QDir bin(QCoreApplication::applicationDirPath());

    QProcess hof;
    hof.setProgram(bin.path() + QDir::separator() + "hof");

    QStringList args = QStringList()
      << "--program"
      << program;

    if (verbose) {
        args.append("--verbose");
        hof.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    }

    hof.setArguments(args);
    hof.start();
    hof.waitForFinished();
    /* special value used to indicate stack depth exceeded */
    *ok = hof.exitStatus() == QProcess::NormalExit &&
        (hof.exitCode() == EXIT_SUCCESS || hof.exitCode() == 2);
    return hof.readAll().trimmed();
}

#define I "I"
#define K "K"
#define S "S"
#define APPLY(X) "A" X
#define PRINT(X) "P" X
#define RANDOM(X, Y) "R" X Y
#define TRUE "K"
#define FALSE "V"
#define IF(X, Y, Z) X Y Z
#define IFNOT(X, Y, Z) X Z Y
#define AND(X, Y) X Y FALSE
#define OR(X, Y) X TRUE Y
#define ZERO FALSE
#define ONE I
#define SUCC(X) "AA" S "AA" S "A" K S K X

// standard beta recursion combinator
#define BETA_RECURSE(X) S "A" K X "AA" S I I "AA" S "A" K X "AA" S I I

// standard y combinator
#define YCOMBINATOR = S "A" K "AA" S I I "A" S "A" S "A" K S K "A" K "AA" S I I

// tromp's y combinator
#define YCOMBINATOR_1 = S S K "A" S "A" K "AA" S S "A" S "AA" S S K K

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

void TestHof::testHofNoise()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distLength(0, 100);
    std::uniform_int_distribution<int> distLetter(0, 6);

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
            case 3: ch = 'V'; break;
            case 4: ch = 'A'; break;
            case 5: ch = 'P'; break;
            case 6: ch = 'R'; break;
            default:
              Q_ASSERT(false);
            }
            randomHofProgram.append(ch);
        }
        bool ok = false;
#if 0
        qDebug() << randomHofProgram;
#endif
        runHof(randomHofProgram, &ok, false /*verbose*/);
        QVERIFY(ok);
    }
}
