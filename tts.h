/****************************************************************************

** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
**
**
**
**
****************************************************************************/

#ifndef TTS_H
#define TTS_H


#include <QMainWindow>
#include <QProcess>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFont>
#include <QDesktopWidget>
#include <QWidget>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/time.h>

#include <time.h>
#include <unistd.h>

#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>



#include <QMainWindow>

namespace Ui {
class tts;
}

class tts : public QMainWindow
{
   Q_OBJECT

public:
    int ota_enabled_flag;


    int wide;
    int high;
    int posy=20;
    int posx=20;

    int fd0;
    int fd1;
    int fd2;
    int fd3;

    int fetype;
    int streamFlag;
    fe_status_t status;

    QString fename;
    QString stuff;
    QString scan_adapter;

   // QVideoWidget *vw;
   // QMediaPlayer *player;
   // QMediaPlaylist *playlist;


    struct dvb_frontend_info fe_stuff0;
    struct dvb_frontend_info fe_stuff1;
    struct dvb_frontend_info fe_stuff2;
    struct dvb_frontend_info fe_stuff3;

    const char *frontend_name0;
    const char *frontend_name1;
    const char *frontend_name2;
    const char *frontend_name3;

    explicit tts(QWidget *parent = 0);
    ~tts();

private slots:
    void on_channelBspinBox_valueChanged(int channel);

    void on_channelAspinBox_valueChanged(int channel);

    void on_channelCspinBox_valueChanged(int channel);

    void on_channelDspinBox_valueChanged(int channel);

    void on_pushButton_clicked();

    void ShowVideos();

    void createStreams();

    void ScanTuner();

    void readScan();

    void stopTunerA();

    void endScan();

    void reconfCreate();

    void cnvfrqtchnl(QString temp);

    void loadReconf();

    void on_showStreamradioButt_clicked();

    bool channelGood(int value);

    void check_adapter_FE();

    void open_fd(int cnt);

    void on_createStreamsradioButt_toggled(bool checked);

    void on_udpm_Button_toggled(bool checked);

private:
    Ui::tts *ui;
};

#endif // TTS_H
