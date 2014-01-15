/*
 * Copyright 2013 John Pritchard, Syntelos.  All rights reserved.
 */
#include <QAction>
#include <QDebug>
#include <QDesktopWidget>
#include <QDialog>
#include <QKeySequence>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QObject>
#include <QScriptValue>
#include <QStatusBar>
#include <QRect>
#include <QSvgWidget>

#include "Storage/StorageTreeEditorDialog.h"
#include "System/SystemScriptSymbol.h"
#include "Configuration/Configuration.h"
#include "Configuration/HCDB.h"
#include "Configuration/Devices.h"
#include "Configuration/Device.h"
#include "Graphics/GraphicsBody.h"
#include "Terminal/Terminal.h"
#include "XPORT/XportConnection.h"
#include "TMTC/TMTCMessage.h"
#include "Multiplex/Multiplex.h"
#include "Window.h"



QScriptValue windowToScriptValue(QScriptEngine *engine, Window* const &in){
    return engine->newQObject(in);
}

void windowFromScriptValue(const QScriptValue &object, Window* &out){
    out = qobject_cast<Window*>(object.toQObject());
}


Window* Window::instance;

/*
 */
Window::Window(QScriptEngine* script)
    : QMainWindow(0), configureOpen(false), init_program(0)
{
    Window::instance = this;

    Configuration* configuration = Configuration::Init(script);

    qScriptRegisterMetaType(script, windowToScriptValue, windowFromScriptValue);

    initSystemScriptable(this);

    /*
     */
    QMenuBar* menuBar = this->menuBar();
    {
        QKeySequence ctrlC(Qt::CTRL + Qt::Key_C);
        QKeySequence ctrlE(Qt::CTRL + Qt::Key_E);
        QKeySequence ctrlF(Qt::CTRL + Qt::Key_F);
        QKeySequence ctrlO(Qt::CTRL + Qt::Key_O);
        QKeySequence ctrlQ(Qt::CTRL + Qt::Key_Q);
        QKeySequence ctrlS(Qt::CTRL + Qt::Key_S);
        QKeySequence ctrlW(Qt::CTRL + Qt::Key_W);

        QMenu* file = menuBar->addMenu("File");

        file->addAction("Open",this,SLOT(open()),ctrlO);
        file->addAction("Edit",this,SLOT(edit()),ctrlE);
        file->addAction("Save",this,SLOT(save()),ctrlS);
        file->addAction("Close",this,SLOT(close()),ctrlC);
        file->addAction("Configure",this,SLOT(configure()),ctrlF);
        file->addSeparator();
        {
            QList<QKeySequence> shortcuts;
            shortcuts << ctrlQ << ctrlW;

            QAction* action = file->addAction("Quit");
            action->setShortcuts(shortcuts);
            action->connect(action,SIGNAL(triggered()),this,SLOT(quit()));
        }
    }
    /*
     */
    GraphicsBody* body = new GraphicsBody(this);

    this->setCentralWidget(body);
    /*
     * System initialization
     */
    this->init_program = new Init(configuration);

    if (this->init_program->run()){

        qDebug() << "Multiplex configured";

        /*****************************************************************
         * Temporary
         *
         * Some script would add a terminal into the "body" widget...
         */
        Multiplex* multiplex = this->init_program->getMultiplex();

        Terminal* terminal = new Terminal();

        body->add(terminal);

        /*
         */
        if (QObject::connect(terminal,SIGNAL(send(const TMTCMessage*)),multiplex,SLOT(receivedFromUser(const TMTCMessage*))) &&
            QObject::connect(multiplex,SIGNAL(sendToUser(const TMTCMessage*)),terminal,SLOT(received(const TMTCMessage*)))
            )
        {
            qDebug() << "Window/Terminal: terminal & multiplex configured (duplex)";
        }
        else {

            qDebug() << "Window/Terminal: terminal & multiplex configuration (duplex) failed";
        }
    }
    else {

        qDebug() << "Multiplex not configured";
    }
    /*
     * Normal default process 
     */
    this->statusBar();
    /*
     * TODO: use QSettings for user's window geometry.
     */
    const QRect& available = qApp->desktop()->availableGeometry();

    QSize window(available.width()>>1,available.height()>>1);

    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                                          Qt::AlignCenter,
                                          window,
                                          available));
    /*
     */
    this->show();
}
/*
 */
Window::~Window(){

    if (this->init_program){

        Init* init = this->init_program;

        this->init_program = 0;

        delete init;
    }
}
void Window::run(){

    if (this->init_program->isUp()){

        initConfigurationScriptable(this);

        emit init();
    }
    else {

        Window::configure();
    }
}
Init* Window::getInit() const {

    return this->init_program;
}
Configuration* Window::getConfiguration() const {

    return Configuration::Instance();
}
void Window::open(){

    Configuration::Instance()->configureWindowInit();
}
void Window::edit(){

}
void Window::save(){

}
void Window::close(){

    Configuration::Instance()->deconfigureWindowInit();
}
void Window::configure(){
    if (!configureOpen){
        configureOpen = true;

        StorageTreeEditorDialog* dialog = new StorageTreeEditorDialog(Configuration::Instance(),this);

        dialog->connectFinishedTo(this,SLOT(configureDone(int)));
    }
}
void Window::quit(){

    QApplication::exit();
}
void Window::configureDone(int error){

    configureOpen = false;
}
QScriptValue Window::alert(QScriptContext* cx, QScriptEngine* se){
    switch(cx->argumentCount()){
    case 1:

        QMessageBox::critical(Window::instance, "Script Alert", cx->argument(0).toString());
        return QScriptValue(true);

    case 2:

        QMessageBox::critical(Window::instance, cx->argument(0).toString(), cx->argument(1).toString());
        return QScriptValue(true);

    default:
        return QScriptValue(false);
    }
}
QScriptValue Window::status(QScriptContext* cx, QScriptEngine* se){
    QString string;
    const int count = cx->argumentCount();
    int cc;
    for (cc = 0; cc < count; cc++){
        if (0 < cc){
            string.append(' ');
        }
        string.append(cx->argument(cc).toString());
    }
    QStatusBar* statusBar = Window::instance->statusBar();
    statusBar->clearMessage();
    statusBar->showMessage(string);

    return QScriptValue(true);
}
