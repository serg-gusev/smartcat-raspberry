#include "usb-client.h"

#include <errno.h>
#include <usb.h>
#include <stdio.h>
#include <string.h>

#include <QDebug>

#define USB_RT_HUB          (USB_TYPE_CLASS | USB_RECIP_DEVICE)
#define USB_RT_PORT         (USB_TYPE_CLASS | USB_RECIP_OTHER)
#define USB_PORT_FEAT_POWER 8
#define USB_DIR_IN          0x80

#define HUB_CHAR_LPSM       0x0003
#define HUB_CHAR_PORTIND    0x0080

struct usb_hub_descriptor
{
    unsigned char bDescLength;
    unsigned char bDescriptorType;
    unsigned char bNbrPorts;
    unsigned char wHubCharacteristics[2];
    unsigned char bPwrOn2PwrGood;
    unsigned char bHubContrCurrent;
    unsigned char data[0];
};

#define CTRL_TIMEOUT 1000
#define USB_STATUS_SIZE 4

static void hub_port_status (usb_dev_handle *uh, int nport)
{
    printf(" Hub Port Status:\n");
    for (int i = 0; i < nport; i++) {
        char buf[USB_STATUS_SIZE];
        int ret = usb_control_msg (uh,
                                   USB_ENDPOINT_IN | USB_TYPE_CLASS | USB_RECIP_OTHER,
                                   USB_REQ_GET_STATUS,
                                   0, i + 1,
                                   buf, USB_STATUS_SIZE,
                                   CTRL_TIMEOUT);

        if (ret < 0) {
            fprintf (stderr,
                 "cannot read port %d status, %s (%d)\n",
                 i + 1, strerror(errno), errno);
            break;
        }

        printf("   Port %d: %02x%02x.%02x%02x", i + 1, buf[3], buf [2], buf[1], buf [0]);

        printf("%s%s%s%s%s",
               (buf[2] & 0x10) ? " C_RESET" : "",
               (buf[2] & 0x08) ? " C_OC" : "",
               (buf[2] & 0x04) ? " C_SUSPEND" : "",
               (buf[2] & 0x02) ? " C_ENABLE" : "",
               (buf[2] & 0x01) ? " C_CONNECT" : "");

        printf("%s%s%s%s%s%s%s%s%s%s\n",
               (buf[1] & 0x10) ? " indicator" : "",
               (buf[1] & 0x08) ? " test" : "",
               (buf[1] & 0x04) ? " highspeed" : "",
               (buf[1] & 0x02) ? " lowspeed" : "",
               (buf[1] & 0x01) ? " power" : "",
               (buf[0] & 0x10) ? " RESET" : "",
               (buf[0] & 0x08) ? " oc" : "",
               (buf[0] & 0x04) ? " suspend" : "",
               (buf[0] & 0x02) ? " enable" : "",
               (buf[0] & 0x01) ? " connect" : "");
    }
}

Usb::Usb(QObject *parent) : QObject(parent)
{
    usb_init();
    usb_find_busses();
    usb_find_devices();

    struct usb_bus *busses = usb_get_busses();
    if (!busses) {
        qDebug() << "failed to access USB: No hubs found";
        return;
    }

    for (struct usb_bus *bus = busses; bus; bus = bus->next) {
        struct usb_device *dev;

        for (dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.bDeviceClass != USB_CLASS_HUB)
                continue;

            usb_dev_handle *uh = usb_open (dev);

            if (uh != NULL) {
                char buf[1024];
                struct usb_hub_descriptor *uhd = (struct usb_hub_descriptor *)buf;

                int len = usb_control_msg (uh, USB_DIR_IN | USB_RT_HUB,
                                           USB_REQ_GET_DESCRIPTOR,
                                           USB_DT_HUB << 8, 0,
                                           buf, sizeof (buf), CTRL_TIMEOUT);
                if (len > sizeof (struct usb_hub_descriptor)) {
                    if (!(uhd->wHubCharacteristics[0] & HUB_CHAR_PORTIND) && (uhd->wHubCharacteristics[0] & HUB_CHAR_LPSM) >= 2)
                        continue;

                    qDebug() << QString("Hub #%1 at %2:%3\n")
                                .arg(hubs.count())
                                .arg(bus->dirname)
                                .arg(dev->devnum);

                    switch ((uhd->wHubCharacteristics[0] & HUB_CHAR_LPSM)) {
                    case 0:
                        qDebug() << " INFO: ganged switching";
                        break;
                    case 1:
                        qDebug() << " INFO: individual power switching";
                        break;
                    case 2:
                    case 3:
                        qDebug() << " WARN: No power switching";
                        break;
                    }

                    if (!(uhd->wHubCharacteristics[0] & HUB_CHAR_PORTIND)) {
                        qDebug() << " WARN: Port indicators are NOT supported";
                    }
                } else {
                    qDebug() << "Can't get hub descriptor";
                    usb_close (uh);
                    continue;
                }

                int nport = buf[2];
                hub_info h;
                h.busnum = atoi (bus->dirname);
                h.devnum = dev->devnum;
                h.dev = dev;
                h.indicator_support = (uhd->wHubCharacteristics[0] & HUB_CHAR_PORTIND)? 1 : 0;
                h.nport = nport;
                hubs << h;

                hub_port_status (uh, nport);
                usb_close (uh);
            }
        }
    }

    if (hubs.count() <= 0) {
        qDebug() << "No hubs found";
    }
}

void Usb::toggleUsb(bool power)
{
    if (hubs.count() < 0)
        return;

    usb_dev_handle *uh = usb_open (hubs.first().dev);
    if (uh) {
        int request = power ? USB_REQ_SET_FEATURE : USB_REQ_CLEAR_FEATURE;
        int feature = USB_PORT_FEAT_POWER;
        int index = 2;

        qDebug() << QString("Send control message (REQUEST=%1, FEATURE=%2, INDEX=%3)")
                    .arg(request).arg(feature).arg(index);

        int res = usb_control_msg (uh, USB_RT_PORT, request, feature, index, NULL, 0, CTRL_TIMEOUT);
        if (res < 0) {
            qDebug() << "failed to control";
        } else {
            emit usbToggled(power);
        }

        hub_port_status(uh, hubs.first().nport);
        usb_close(uh);
    }
}
