#ifndef USB_H
#define USB_H

#include <QObject>

struct hub_info
{
    int busnum, devnum;
    struct usb_device *dev;
    int nport;
    int indicator_support;
};

class Usb : public QObject
{
    Q_OBJECT

public:
    explicit Usb(QObject *parent = 0);

public slots:
    void toggleUsb(bool power);

signals:
    void usbToggled(bool);

private:
    QList<hub_info> hubs;
};

#endif // USB_H
