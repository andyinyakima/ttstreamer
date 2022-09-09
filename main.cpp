#include "tts.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    tts w;
    w.setWindowIconText("Run");
    w.show();

    return a.exec();
}
