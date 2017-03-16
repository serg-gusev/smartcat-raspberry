#include "application.h"

#include <QDebug>
#include <QImage>
#include <QBuffer>
#include <QImageWriter>

#include <unistd.h>

Application::Application() : QObject()
{
    w.showFullScreen();

    connect(&s, SIGNAL(connected()), SLOT(onConnected()));
    connect(&s, SIGNAL(disconnected()), SLOT(onDisconnected()));
    connect(&s, SIGNAL(textMessageReceived(QString)), SLOT(onMessageReceived(QString)));
    connect(&c, SIGNAL(textMessageReceived(QString)), SLOT(onVideoMessageReceived(QString)));

    connect(&w, SIGNAL(switchToggledUser(bool)), SLOT(onSwitchToggled(bool)));

    camera.open();
    sleep(3);
}

Application::~Application()
{

}

void Application::runConnect()
{
    onDisconnected();
}

void Application::onMessageReceived(const QString &msg)
{
    qDebug() << msg;

    if (msg == "on") {
        onSwitchToggled(true);
    } else if (msg == "off") {
        onSwitchToggled(false);
    }
}

void Application::onVideoMessageReceived(const QString &msg)
{
    if (msg != "frame") {
        qDebug() << "video websocket message:" << msg;
        return;
    }

    if (!camera.isOpened()) {
        qDebug() << "camera is not opened";
        return;
    }

    camera.grab();
    int imageSize = camera.getImageTypeSize(raspicam::RASPICAM_FORMAT_RGB);
    unsigned char *data = new unsigned char[imageSize];
    camera.retrieve(data, raspicam::RASPICAM_FORMAT_RGB);
    
    QImage frame = QImage(data, camera.getWidth(), camera.getHeight(), QImage::Format_RGB888).scaledToWidth(400);

    QByteArray array;
    QBuffer buffer(&array);

    // write image to memory buffer
    QImageWriter writer;
    writer.setDevice(&buffer);
    writer.setFormat("JPEG");
    writer.setCompression(9);
    writer.write(frame);

    c.sendBinaryMessage(array);

    delete[] data;
}

void Application::onConnected()
{
    w.setConnected();
}

void Application::onDisconnected()
{
    w.setDisconnected();
    s.open(QUrl("ws://46.242.98.12:8889/ws"));
    c.open(QUrl("ws://46.242.98.12:8889/vws"));
}

void Application::onSwitchToggled(bool toggled)
{
    w.toggleSwitch(toggled);
    u.toggleUsb(toggled);
}
