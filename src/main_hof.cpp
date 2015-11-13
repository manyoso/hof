#include <QtCore>

#include "hof.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("hof");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption fileOption("file", "Specify a hof file to run.", "file");
    parser.addOption(fileOption);

    QCommandLineOption programOption("program", "Specify a hof program to run.", "program");
    parser.addOption(programOption);

    QCommandLineOption inputOption("input", "Specify a hof input to run.", "input");
    parser.addOption(inputOption);

    QCommandLineOption verboseOption("verbose", "Verbose execution evaluation.");
    parser.addOption(verboseOption);

    parser.process(*QCoreApplication::instance());

    bool isFile = parser.isSet(fileOption);
    bool isProgram = parser.isSet(programOption);
    bool isInput = parser.isSet(inputOption);
    bool isVerbose = parser.isSet(verboseOption);

    if ((isFile && isProgram) || (!isFile && !isProgram))
        parser.showHelp(-1);

    QString program;

    if (isFile) {
        QString fileName = parser.value(fileOption);
        QFile file(fileName);
        if (!file.exists()) {
            qDebug() << "Error: file does not exist: " << fileName;
            exit(-1);
        }

        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Error: could not open file for reading: " << fileName;
            exit(-1);
        }

        QFileInfo info(file);
        program = file.readAll();
    } else if (isProgram) {
        program = parser.value(programOption);
    }

    // Remove all whitespace
    program = program.simplified();
    program.replace(" ", "");

    if (isInput)
        program.append(parser.value(inputOption));

    // Remove all whitespace from input too
    program = program.simplified();
    program.replace(" ", "");

    QTextStream stream(stdout);
    QTextStream verboseStream(stderr);
    Hof hof(&stream, isVerbose ? &verboseStream : 0);
    hof.run(program);
    if (!isVerbose) {
        stream << "\n";
        stream.flush();
    }

    return EXIT_SUCCESS;
}
