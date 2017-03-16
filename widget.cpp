#include "widget.h"
#include "ui_widget.h"

#include <QApplication>
#include <QMovie>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    connect(ui->switchButton, SIGNAL(toggled(bool)), SIGNAL(switchToggledUser(bool)));
    connect(ui->closeButton, SIGNAL(clicked()), qApp, SLOT(quit()));

    QMovie *movie = new QMovie(":/spinner.gif", QByteArray(), ui->spinnerLabel);
    ui->spinnerLabel->setMovie(movie);
    movie->start();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::toggleSwitch(bool toggled)
{
    QSignalBlocker a(ui->switchButton);
    ui->switchButton->setChecked(toggled);
}

void Widget::setConnectedState(bool connected)
{
    ui->statusBox->setChecked(connected);
    ui->spinnerLabel->setVisible(!connected);
}

void Widget::setConnected()
{
    setConnectedState(true);
}

void Widget::setDisconnected()
{
    setConnectedState(false);
}
