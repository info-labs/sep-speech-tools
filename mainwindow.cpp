#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QSettings>
#include <QWebView>

#include <mecab.h>

#include "autocursor.h"
#include "preferencedialog.h"
#include "speechcounter.h"
#include "utility.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
#if defined(Q_OS_LINUX)
    settings(new QSettings()),
#elif defined(Q_OS_WIN32) || defined(Q_OS_CYGWIN)
    settings(new QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName())),
#endif
    preference(new PreferenceDialog(this->settings)),
    tagger(nullptr),
    speechCounter()
{
    this->ui->setupUi(this);

    this->initializeTagger(QString());
}

MainWindow::~MainWindow()
{
    if ( tagger ) {
        MeCab::deleteTagger(tagger);
    }
}

void MainWindow::initializeTagger(QString dictPath, QString userDictPath)
{
    if ( dictPath.isEmpty() ) {
        dictPath = this->preference->getSysDictDir();
    }
    if ( userDictPath.isEmpty() ) {
        userDictPath = this->preference->getUserDictFile();
    }
    // build args
    QStringList taggerArgs;
    taggerArgs.append(QCoreApplication::applicationFilePath()); // argv[0]
    taggerArgs.append("-U");
    taggerArgs.append("Unknown");
#if defined(Q_OS_WIN)
    QDir appDir(QCoreApplication::applicationDirPath());
    taggerArgs.append("-r");
    taggerArgs.append(appDir.filePath("mecabrc"));
#endif
    taggerArgs.append("-d");
    taggerArgs.append(dictPath + "/");
    if ( ! userDictPath.isEmpty() ) {
        taggerArgs.append("-u");
        taggerArgs.append(userDictPath);
    }
    // unicode args to raw args
    QList<QByteArray> args;
    for ( const QString &arg : taggerArgs ) {
        args.append(toLocalEncoding(arg));
    }
    // raw args to argv
    QScopedPointer<char *> argv(new char *[args.size()]);
    int argc = 0;
    for ( QByteArray &arg : args ) {
        argv.data()[argc++] = arg.data();
    }
    // create tagger
    this->tagger = MeCab::Tagger::create(argc, argv.data());
}

void MainWindow::on_actionShowRuby_triggered()
{
    if ( ! this->speechCounter ) {
        return;
    }

    QWebView *view = new QWebView(nullptr);
    view->setHtml(this->speechCounter->toRubyHtml());
    view->show();
}

void MainWindow::on_actionPreference_triggered()
{
    if ( this->preference->exec() == QDialog::Accepted ) {
        this->preference->saveSettings();
        this->initializeTagger(QString());  // from new settings
    }
}

void MainWindow::setTextInformation(int charCount, double speechCount)
{
    QString message;
    if ( speechCount < 0 ) {
        message = tr("%1 chars").arg(charCount);
    } else {
        message = tr("%1 chars, %2 syllable").arg(charCount).arg(speechCount);
    }
    this->ui->statusBar->showMessage(message, 0);
}

void MainWindow::on_textEdit_textChanged()
{
    BusyAutoCursor cursor(this);
    Q_UNUSED(cursor);

    QString text = this->ui->textEdit->toPlainText().normalized(QString::NormalizationForm_KC);

    QString serialized(text);
    serialized.remove(QRegExp("\\s+"));

    this->speechCounter = QSharedPointer<SpeechCounter>(new SpeechCounter(this->tagger, text));
    this->setTextInformation(serialized.size(), this->speechCounter->getSpeechCount());
}
