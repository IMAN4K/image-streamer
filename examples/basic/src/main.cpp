#include <ImageStreamer/ImageStreamer.h>
#include <ImageStreamer/Utility.h>

#include <QtCore>
#include <QtWidgets>

int main(int argc, char* argv[]) {
    new QApplication(argc, argv);

    QLabel label;

    constexpr auto kPort = 55100;

    ImageStreamer receiver;
    receiver.listen(QHostAddress::LocalHost, kPort);
    QObject::connect(&receiver, QOverload<QPixmap>::of(&ImageStreamer::ready), [&](QPixmap pix) {
        label.setScaledContents(true);
        label.setPixmap(pix);
        label.show();
    });

    ImageStreamer sender;
    sender.update(QHostAddress(QHostAddress::LocalHost), kPort);
    sender.send(utility::captureScreen(), 100);

    return QCoreApplication::exec();
}
