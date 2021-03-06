#include <QtCore>
#include <QtTest/QtTest>

#include "testhof.h"

#define _NAME_ "hoftests"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(_NAME_);

    int rc = 0;
    TestHof hof;
    rc = QTest::qExec(&hof, argc, argv) == 0 ? rc : -1;

    return rc;
}
