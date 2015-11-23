#include <QtCore>

#include "testhof.h"

enum Expectation {
    Normal,  // expect no timeout and no crash
    NoCrash, // expect no crash
    Timeout  // expect timeout
};

QString runHof(const QString& file,
               const QString& input,
               bool* ok,
               bool verbose = false,
               int msecsToTimeout = 5000,
               Expectation e = Expectation::Normal)
{
    QDir bin(QCoreApplication::applicationDirPath());

    QProcess hof;
    hof.setProgram(bin.path() + QDir::separator() + "hof");

    QStringList args = QStringList()
      << "--file"
      << file
      << "--input"
      << input;

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

// church numerals
#define INC(X) "AASAASAKSK" X
#define ZERO "AKI"
#define ONE "I"
#define TWO INC(ONE)
#define THREE INC(TWO)
#define FOUR INC(THREE)
#define FIVE INC(FOUR)

void TestHof::testExamples()
{
    bool ok = false;
    QString out;

    out = runHof("examples/iterate.lambda", TWO, &ok);
    QCOMPARE(out, QString("II"));
    QVERIFY(ok);
}
