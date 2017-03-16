#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

public slots:
    void toggleSwitch(bool toggled);

    void setConnectedState(bool connected);
    void setConnected();
    void setDisconnected();

signals:
    void switchToggledUser(bool);

private:
    Ui::Widget *ui;
};

#endif // WIDGET_H
