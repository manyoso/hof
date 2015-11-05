#include <QtCore>

#include "testhof.h"

QString runHof(const QString& program, bool verbose = false)
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
    return hof.readAll().trimmed();
}

#define I "I"
#define K "K"
#define S "S"
#define APPLY(X) "A" X
#define PRINT(X) "P" X
#define RANDOM(X, Y) "R" X Y
#define TRUE "K"
#define FALSE "SK"
#define IF(X, Y, Z) X Y Z
#define IFNOT(X, Y, Z) X Z Y
#define AND(X, Y) X Y FALSE

void TestHof::testHof()
{
    QString out;

    out = runHof(IF(AND(TRUE, TRUE), APPLY(PRINT(I)), APPLY(PRINT(K))), true);

    out = runHof(PRINT(I));
    QCOMPARE(out, QString(I));

    out = runHof(IF(TRUE, APPLY(PRINT(I)), APPLY(PRINT(K))));
    QCOMPARE(out, QString(I));

    out = runHof(IF(FALSE, APPLY(PRINT(I)), APPLY(PRINT(K))));
    QCOMPARE(out, QString(K));

    out = runHof(IFNOT(TRUE, APPLY(PRINT(I)), APPLY(PRINT(K))));
    QCOMPARE(out, QString(K));

    out = runHof(IFNOT(FALSE, APPLY(PRINT(I)), APPLY(PRINT(K))));
    QCOMPARE(out, QString(I));

    out = runHof(RANDOM(APPLY(PRINT(I)), APPLY(PRINT(K))));
    QVERIFY2(out == "I" || out == "K", qPrintable(out));
}
