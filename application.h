#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QtWebSockets/QWebSocket>

#include "widget.h"
#include "usb-client.h"

#include <raspicam/raspicam.h>

class Application : public QObject
{
    Q_OBJECT

public:
    Application();
    virtual ~Application();

    void runConnect();

private slots:
    void onMessageReceived(const QString &msg);
    void onVideoMessageReceived(const QString &msg);
    void onSwitchToggled(bool toggled);

    void onConnected();
    void onDisconnected();

private:
    Widget w;
    QWebSocket s, c;
    Usb u;
    raspicam::RaspiCam camera;
};

#endif // APPLICATION_H
