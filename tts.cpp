/*tts ... stands for tune then stream
 *
 * the program will use "scan" to pick up the channel services available
 * "vlc" will be used for setting up the screen streams using mosaic scheme
 * "mpv" or "ffplay" will be used for showing and monitoring the streams
 *  this will hopefully be an improved version of TTTuner1
 *
 *
 */

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
**  Programmer: Andy Laberge
**  email: andylaberge@linux.com
** start development: 2012
**
**
****************************************************************************/

#include "tts.h"
#include "ui_tts.h"


int freqA;
int khzA;
int freqB;
int khzB;
int freqC;
int khzC;
int freqD;
int khzD;
int numTuners;  // number of tuners
int adaptersEn =0;

int sidcnt;

QObject *parent;

// four tuners should be enough- note: cpu i5 core ivy bridge 12 gig DDR3
QProcess *ttsA = new QProcess(parent);
QProcess *ttsB = new QProcess(parent);
QProcess *ttsC = new QProcess(parent);
QProcess *ttsD = new QProcess(parent);
QProcess *vlc = new QProcess(parent);

// QTimer *timerA ;

// globals watch out for these

QString globalChannel; // watch out for this
QString globalName; // name of service
QStringList streamid;
QStringList nameid;



tts::tts(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::tts)
{
    ui->setupUi(this);
    streamFlag=0x01;
    ota_enabled_flag=0;

// part of initiation
// this scans for tuner adapters
    QString path = "/dev/dvb"; // path for the adapters in Linux
    QDir dir;
    dir.setPath(path);

    QStringList adapters;
    check_adapter_FE();
    adapters=dir.entryList();
    numTuners = adapters.count();
    // look for physical adapter name and if found enable the tuner group
    if(numTuners>0)
    {
      //  numTuners=numTuners-2;
        ui->textBrowser1->clear();
   //     reconfCreate();
        loadReconf();
        if((adapters.contains("adapter0" )) && (ui->tunerNameA->text()=="ATSC"))
        {

                ui->tunerAgroupBox->setEnabled(true);
                adaptersEn++;
                scan_adapter="-a0";
                ota_enabled_flag+=1;



        }
        else
            ui->tunerAgroupBox->setEnabled(false);
        if((adapters.contains("adapter1")) && (ui->tunerNameB->text()=="ATSC"))
        {

                ui->tunerBgroupBox->setEnabled(true);
                adaptersEn++;
                scan_adapter="-a1";
                ota_enabled_flag+=2;
        }

        else
            ui->tunerBgroupBox->setEnabled(false);
        if((adapters.contains("adapter2")) && (ui->tunerNameC->text()=="ATSC"))
        {
            ui->tunerCgroupBox->setEnabled(true);
            adaptersEn++;
            scan_adapter="-a2";
            ota_enabled_flag+=4;
        }
        else
            ui->tunerCgroupBox->setEnabled(false);
        if((adapters.contains("adapter3")) && (ui->tunerNameD->text()=="ATSC"))
        {
            ui->tunerDgroupBox->setEnabled(true);
            adaptersEn++;
            scan_adapter="-a3";
            ota_enabled_flag+=8;
        }
        else
            ui->tunerDgroupBox->setEnabled(false);
     }
   else
   {
        ui->statusBar->showMessage("You have no tuner adapters"); // if there are no working adapters state so
        ui->centralWidget->setEnabled(false);
    }

    // added 18mar2013
    // checks for radio button pushed
    if(ui->createStreamsradioButt->isChecked()==true)
        ui->streamGroupBox->setEnabled(true);


}

tts::~tts()
{
    // when destroying the class kill any external processes
    ui->textBrowser->clear();
    QString homepath = QDir::homePath();

     homepath.append("/tts.strm");


     QFile strfile(homepath);
     if (!strfile.open(QIODevice::WriteOnly | QIODevice::Text))
            return ;


     QTextStream strout(&strfile);

     strout<<ui->textBrowser->toPlainText();
  //   file.close();
     statusBar()->showMessage(tr("File - %1 - Saved")
             .arg(homepath));
     //streamin.flush();
     strout.flush();

    ttsA->kill();
    ttsB->kill();
    ttsC->kill();
    ttsD->kill();
    vlc->kill();

    delete ui;
}

void tts::on_pushButton_clicked()
{


    if(ui->ATSCscanradioButt->isChecked()==true)
        ScanTuner();    
    if(ui->showStreamradioButt->isChecked()==true)
        ShowVideos();
    if(ui->createStreamsradioButt->isChecked()==true)
        createStreams();
}


void tts::ScanTuner()
{
         QString homepath = QDir::homePath();
         QString outform = "-o";
         QString formout = "zap"; //was zap
         QString numadapter ="-a";
         QString filename;

         filename = QFileDialog::getOpenFileName(this,tr("Open File"),homepath+"/tts",
                                                                     tr(" (*ATSC.*)"));
                     if(!filename.isEmpty())
                     {
                         QFile file(filename);
                         if (!file.open(QFile::ReadOnly | QFile::Text))
                         {
                             QMessageBox::warning(this, tr("Recent Files"),
                                                  tr("Cannot read file %1:\n%2.")
                                                  .arg(filename)
                                                  .arg(file.errorString()));
                             return;
                         }
                      }
       if(adaptersEn>0)
        {
            ui->textBrowser->clear();
            ui->textBrowser1->clear();

            QString programA ="scan";// scan is a tuner scaning program installed with dvb-apps
            QStringList argumentsA;

           // ttsA->setStandardOutputFile("/ttt_channels.conf");
            ttsA->setEnvironment(QProcess::systemEnvironment());
            ttsA->setProcessChannelMode(QProcess::MergedChannels);

            numadapter.append(QString::number(ui->scan_adapter_spinBox->value()));
            argumentsA<<outform<<formout<<scan_adapter<<numadapter<<filename; // arguments that scan for ATSC signals in the USA and produces config file

            ttsA->start(programA,argumentsA);
        // below takes the scan program and outputs it to a text browser in function read:Scan()
            ttsA->waitForStarted();
         //   connect(ttsA,SIGNAL(readyReadStandardError()),this,SLOT(endScan()));
            connect(ttsA,SIGNAL(readyReadStandardOutput()),this,SLOT(readScan()));
            connect(ttsA,SIGNAL(finished(int)),this,SLOT(stopTunerA()));


        }

 }

void tts::readScan()
{
    QString line;
    QString homepath = QDir::homePath();

    QProcess *ttsA = dynamic_cast<QProcess *>(sender());

    if(ttsA)
    ui->textBrowser->insertPlainText(ttsA->readAllStandardOutput());

    QFile file(homepath+"/tts.conf");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
           return ;


    QTextStream out(&file);
    line=out.readLine();
    out<<ui->textBrowser->toPlainText();
    file.close();
   //ui->textBrowser->hide();

}



void tts::ShowVideos()
{
        int cnt;
        QString line;

    //    int foundfreq;
        int foundprog;
        int linelen;
        int flag_en;

        QString programs;
        QString prognum;
        QString destination;
        QString destAll;
        QString homepathrd = QDir::homePath();

        QString homepath = QDir::homePath();

        homepath.append("/tts_vlm.conf");


        QFile file(homepath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                 return ;
        QString cntText;
        QString tunerNew;
        QString tunerSetup;
        QString tunerOutput;
        QString tunerControl;
        QString frequency;

        homepathrd.append("/tts.reconf");
        QFile filein(homepathrd);
        filein.open(QIODevice::ReadOnly | QIODevice::Text);

        QTextStream streamin(&filein);

        ui->textBrowser->clear();

        flag_en=ota_enabled_flag;
        for(cnt=0;cnt<adaptersEn;cnt++)
        {
           // cntText= QString::number(cnt);

            if((flag_en & 0x01)==0x01)
            {
                cntText="0";
                flag_en-=0x01;
            }
            else if((flag_en & 0x02)==0x02)
            {
                cntText="1";
                flag_en-=0x02;
            }
            else if((flag_en & 0x04)==0x04)
            {
                cntText="2";
                flag_en-=0x04;
            }
            else if((flag_en & 0x08)==0x08)
            {
                cntText="3";
                flag_en-=0x08;
            }

            tunerNew="new tuner_"+cntText+" broadcast enabled\n";

            ui->textBrowser->insertPlainText(tunerNew);

        }

         ui->textBrowser->insertPlainText("\n\n");


         flag_en=ota_enabled_flag;
         for(cnt=0;cnt<adaptersEn;cnt++)
         {
             // cntText= QString::number(cnt);
             if((flag_en & 0x01)==0x01)
             {
                 cntText="0";
                 flag_en-=0x01;
             }
             else if((flag_en & 0x02)==0x02)
             {
                 cntText="1";
                 flag_en-=0x02;
             }
             else if((flag_en & 0x04)==0x04)
             {
                 cntText="2";
                 flag_en-=0x04;
             }
             else if((flag_en & 0x08)==0x08)
             {
                 cntText="3";
                 flag_en-=0x08;
             }


             tunerSetup="setup tuner_"+cntText+" input 'atsc://'\n";
              ui->textBrowser->insertPlainText(tunerSetup);
              tunerSetup="setup tuner_"+cntText+" option dvb-adapter="+cntText+"\n";
              ui->textBrowser->insertPlainText(tunerSetup);
             // now insert the frequency of the selected channel for the adapter
             // adapter0 = tuner group a
              if(cntText=="0")
                 frequency=QString::number(ui->FrequencyA->intValue());
              else if(cntText=="1")
                  frequency=QString::number(ui->FrequencyB->intValue());
              else if(cntText=="2")
                  frequency=QString::number(ui->FrequencyC->intValue());
              else if(cntText=="3")
                  frequency=QString::number(ui->FrequencyD->intValue());

              tunerSetup="setup tuner_"+cntText+" option dvb-frequency="+frequency+"000\n";
              ui->textBrowser->insertPlainText(tunerSetup);
              tunerSetup="setup tuner_"+cntText+" option dvb-modulation=8VSB\n";
              ui->textBrowser->insertPlainText(tunerSetup);

              programs="programs=";
              while(!filein.atEnd())
              {
                  line = filein.readLine(0);
                  linelen=line.length();
                  if(line.contains(frequency)==true)
                  {
                     destination="dst=display,select= program=";
                     foundprog=line.indexOf("program=");
                     prognum=line.mid(foundprog+8,(linelen-1)-(foundprog+8));
                     prognum.append(",");
                     programs.append(prognum);
                     destination.append(prognum);
                     destAll.append(destination);


                  }


              }
              tunerSetup="setup tuner_"+cntText+" option "+programs+"\n" ;
              tunerOutput="setup tuner_"+cntText+" output '#duplicate{"+destAll+"}'\n";
              destAll.clear();
              filein.seek(0);

              ui->textBrowser->insertPlainText(tunerSetup);
              ui->textBrowser->insertPlainText("\n");
             ui->textBrowser->insertPlainText(tunerOutput);

             ui->textBrowser->insertPlainText("\n");


        }
          streamin.flush();
          filein.close();

         flag_en=ota_enabled_flag;
         for(cnt=0;cnt<adaptersEn;cnt++)
         {
            //cntText=QString::number(cnt);
             if((flag_en & 0x01)==0x01)
             {
                 cntText="0";
                 flag_en-=0x01;
             }
             else if((flag_en & 0x02)==0x02)
             {
                 cntText="1";
                 flag_en-=0x02;
             }
             else if((flag_en & 0x04)==0x04)
             {
                 cntText="2";
                 flag_en-=0x04;
             }
             else if((flag_en & 0x08)==0x08)
             {
                 cntText="3";
                 flag_en-=0x08;
             }

            tunerControl="control tuner_"+cntText+" play\n";
            ui->textBrowser->insertPlainText(tunerControl);

         }
         ui->textBrowser->insertPlainText("\n");

        QTextStream out(&file);

        out<<ui->textBrowser->toPlainText();
     //   file.close();
        statusBar()->showMessage(tr("File - %1 - Saved")
                .arg(homepath));



        QString program  ="cvlc";
        QStringList arguments;
// added : 24Feb2013 scale from .1 to 1

        arguments<<"--vlm-conf"<<homepath<<"--no-audio"<<"--zoom="+QString::number(ui->ScaleSpinBox->value());
        vlc->start(program,arguments);


}

void tts::createStreams()
{
    //this basically sets up the vlm-conf file
    // "new" creates the tuner stream
    // "setup" sets up stream
    // "control" starts,stops,pauses

    int cnt;
    QString line;


    int foundch;
    int foundcolon;
    int foundprog;
    int fnd_name;
    int fnd_name_colon;
    int linelen;
    int flag_en;


    QString programs;
    QString prognum;
    QString destinationA;
    QString destinationB;
    QString destAll;
    QString homepathrd = QDir::homePath();
    QString progname;
    QString prgkill = "pkill";
    QString pkwho = "vlc";
    QStringList arguk;
    arguk<<pkwho;

    QProcess *kstream = new QProcess(this);
    kstream->start(prgkill,arguk);


    QString homepath = QDir::homePath();
    sidcnt=0;
    streamid.clear();
    nameid.clear();

    homepath.append("/ttsrtp_vlm.conf");


    QFile file(homepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
             return ;
    QString cntText;
    QString rtpNew;
    QString rtpSetup;
    QString rtpOutput;
    QString rtpControl;
    QString frequency;
    QString ch;

    homepathrd.append("/tts.reconf");
    QFile filein(homepathrd);
    filein.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream streamin(&filein);

    ui->textBrowser->clear();

    flag_en=ota_enabled_flag;
    for(cnt=0;cnt<adaptersEn;cnt++)
    {
        if((flag_en & 0x01)==0x01)
        {
            cntText="0";
            flag_en-=0x01;
        }
        else if((flag_en & 0x02)==0x02)
        {
            cntText="1";
            flag_en-=0x02;
        }
        else if((flag_en & 0x04)==0x04)
        {
            cntText="2";
            flag_en-=0x04;
        }
        else if((flag_en & 0x08)==0x08)
        {
            cntText="3";
            flag_en-=0x08;
        }

        // cntText= QString::number(cnt);
        rtpNew="new rtp_"+cntText+" broadcast enabled\n";

        ui->textBrowser->insertPlainText(rtpNew);

    }

    ui->textBrowser->insertPlainText("\n");

    flag_en=ota_enabled_flag;
    for(cnt=0;cnt<adaptersEn;cnt++)
    {
        if((flag_en & 0x01)==0x01)
        {
            cntText="0";
            flag_en-=0x01;
        }
        else if((flag_en & 0x02)==0x02)
        {
            cntText="1";
            flag_en-=0x02;
        }
        else if((flag_en & 0x04)==0x04)
        {
            cntText="2";
            flag_en-=0x04;
        }
        else if((flag_en & 0x08)==0x08)
        {
            cntText="3";
            flag_en-=0x08;
        }

         //cntText= QString::number(cnt);
         rtpSetup="setup rtp_"+cntText+" input 'atsc://'\n";
         ui->textBrowser->insertPlainText(rtpSetup);
         rtpSetup="setup rtp_"+cntText+" option dvb-adapter="+cntText+"\n";
         ui->textBrowser->insertPlainText(rtpSetup);
        // now insert the frequency of the selected channel for the adapter
        // adapter0 = tuner group a
         if(cntText=="0")
            frequency=QString::number(ui->FrequencyA->intValue());
         else if(cntText=="1")
             frequency=QString::number(ui->FrequencyB->intValue());
         else if(cntText=="2")
             frequency=QString::number(ui->FrequencyC->intValue());
         else if(cntText=="3")
             frequency=QString::number(ui->FrequencyD->intValue());

         rtpSetup="setup rtp_"+cntText+" option dvb-frequency="+frequency+"000\n";
         ui->textBrowser->insertPlainText(rtpSetup);
         rtpSetup="setup rtp_"+cntText+" option dvb-modulation=8VSB\n";
         ui->textBrowser->insertPlainText(rtpSetup);

         programs="programs=";
         while(!filein.atEnd())
         {
             line = filein.readLine(0);
             linelen=line.length();
             if(line.contains(frequency)==true)
             {
                foundch=line.indexOf("CH ");
                foundcolon=line.indexOf(":");
                ch=line.mid(foundch+3,foundcolon-(foundch+3));
                fnd_name=line.indexOf("Name=");
                fnd_name_colon=line.indexOf(":",fnd_name+5);
                int namelen=(fnd_name_colon)-(fnd_name+5);
                progname=line.mid(fnd_name+5,namelen);

    // notes on mux
    // ps and raw don't seem to work well in this format

    // added: 23feb2013 the IPlineedit for editing IP address 127.0.0.1 is default localhost
                if(ui->udpm_Button->isChecked()==true)
                {
                    destinationA="dst=udp{mux=ts,ttl=12,dst="+ui->IPlineEdit->text()+":"+ch;     // change to udp or rtp see below
                }
                else if(ui->udpButton->isChecked()==true)
                {
                    destinationA="dst=udp{mux=ts,ttl=12,dst="+ui->IPlineEdit->text()+":"+ch;
                }

                destinationB=",select= program=";

                foundprog=line.indexOf("program=");
                prognum=line.mid(foundprog+8,(linelen-1)-(foundprog+8));
                programs.append(prognum);
                programs.append(",");
                if(prognum.size()<2)
                {
                    destinationA.append("0");
                    ch.append("0");
                 }
                destinationA.append(prognum);
                ch.append(prognum);

                if(ui->udpm_Button->isChecked()==true)
                {
                    ch.prepend("udp://@"+ui->IPlineEdit->text()+":");  // change to udp ot rtp see above
                }
                if(ui->udpButton->isChecked()==true)
                {
                   // ch.prepend("udp://@:");
                    ch.prepend("udp://@"+ui->IPlineEdit->text()+":");
                }

                destinationA.append("}");
                destinationB.append(prognum+",");
                destinationA.append(destinationB);
                destAll.append(destinationA);
                streamid.append(ch);
                nameid.append(progname);
                sidcnt++;

             }


         }
         rtpSetup="setup rtp_"+cntText+" option "+programs+"\n" ;
         rtpOutput="setup rtp_"+cntText+" output '#duplicate{"+destAll+"}'\n";
         destAll.clear();
         filein.seek(0);

         ui->textBrowser->insertPlainText(rtpSetup);
         ui->textBrowser->insertPlainText("\n");
        ui->textBrowser->insertPlainText(rtpOutput);

        ui->textBrowser->insertPlainText("\n");


   }
    streamin.flush();
    filein.close();

   flag_en=ota_enabled_flag;
   for(cnt=0;cnt<adaptersEn;cnt++)
   {
       if((flag_en & 0x01)==0x01)
       {
           cntText="0";
           flag_en-=0x01;
       }
       else if((flag_en & 0x02)==0x02)
       {
           cntText="1";
           flag_en-=0x02;
       }
       else if((flag_en & 0x04)==0x04)
       {
           cntText="2";
           flag_en-=0x04;
       }
       else if((flag_en & 0x08)==0x08)
       {
           cntText="3";
           flag_en-=0x08;
       }

      //cntText=QString::number(cnt);
      rtpControl="control rtp_"+cntText+" play\n";
      ui->textBrowser->insertPlainText(rtpControl);

   }
   ui->textBrowser->insertPlainText("\n");
    QTextStream out(&file);

    out<<ui->textBrowser->toPlainText();
 //   file.close();
    statusBar()->showMessage(tr("File - %1 - Saved")
            .arg(homepath));

    QString program  ="cvlc";
    QStringList arguments;
// added : 24Feb2013 scale from .1 to 1

   // arguments<<"--vlm-conf"<<homepath<<"--zoom="+QString::number(ui->ScaleSpinBox->value());
    arguments<<"--vlm-conf"<<homepath;
    vlc->start(program,arguments);

    // added : 7 apr 2019
    // create and store file "tts.strm" which is needed for "ttviewer"
    // "tts.strm" will be cleared when "ttstreamer" shuts down.

    ui->textBrowser->clear();
    for(int i=0;i<sidcnt;i++)
    {
        ui->textBrowser->append(streamid.at(i)+"   "+nameid.at(i));
    }
        ui->textBrowser->append("\n"); // <<-- this is needed!! added carriage return for next program "ttviewer" etal

       homepath = QDir::homePath();

        homepath.append("/tts.strm");


        QFile strfile(homepath);
        if (!strfile.open(QIODevice::WriteOnly | QIODevice::Text))
               return ;


        QTextStream strout(&strfile);

        strout<<ui->textBrowser->toPlainText();
     //   file.close();
        statusBar()->showMessage(tr("File - %1 - Saved")
                .arg(homepath));
        streamin.flush();
        strout.flush();


}






void tts::on_channelAspinBox_valueChanged(int channel)
{


        QPalette* palette = new QPalette();
        palette->setColor(QPalette::Text,Qt::black);

        ui->channelAspinBox->setPalette(*palette);

        channel=ui->channelAspinBox->value();

        if(channel>=2 && channel<=4)
        {
            khzA=(channel*6+45)*1000;
            ui->FrequencyA->display(khzA);

        }
        if(channel>=5 && channel<=6)
        {
            khzA=(channel*6+49)*1000;
            ui->FrequencyA->display(khzA);

        }

        if(channel>=7 && channel<=13)
        {
            khzA=(channel*6+135)*1000;
            ui->FrequencyA->display(khzA);

        }
        if(channel>=14 && channel<=51)
        {
            khzA=(channel*6+389)*1000;
            ui->FrequencyA->display(khzA);

        }

        //channelGood(channel);

            if(channelGood(channel)==true)     // see if channel is exist
            {
               palette->setColor(QPalette::Text,Qt::red);
               ui->channelAspinBox->setPalette(*palette);
               ui->tunerNameA->clear();
               ui->tunerNameA->insert(globalName);
            }

}

void tts::on_channelBspinBox_valueChanged(int channel)
{
    //    this will take the channel and convert it to kilohertz

        QPalette* palette = new QPalette();
        palette->setColor(QPalette::Text,Qt::black);

        ui->channelBspinBox->setPalette(*palette);

        channel=ui->channelBspinBox->value();

        if(channel>=2 && channel<=4)
        {
            khzB=(channel*6+45)*1000;
            ui->FrequencyB->display(khzB);

        }
        if(channel>=5 && channel<=6)
        {
            khzB=(channel*6+49)*1000;
            ui->FrequencyB->display(khzB);

        }

        if(channel>=7 && channel<=13)
        {
            khzB=(channel*6+135)*1000;
            ui->FrequencyB->display(khzB);

        }
        if(channel>=14 && channel<=51)
        {
            khzB=(channel*6+389)*1000;
            ui->FrequencyB->display(khzB);

        }
        //channelGood(channel);

            if(channelGood(channel)==true)     // see if channel is exist
            {
               palette->setColor(QPalette::Text,Qt::red);
               ui->channelBspinBox->setPalette(*palette);
               ui->tunerNameB->clear();
               ui->tunerNameB->insert(globalName);
            }
}

void tts::on_channelCspinBox_valueChanged(int channel)
{
    //    this will take the channel and convert it to kilohertz

        QPalette* palette = new QPalette();
        palette->setColor(QPalette::Text,Qt::black);

        ui->channelCspinBox->setPalette(*palette);

        channel=ui->channelCspinBox->value();

        if(channel>=2 && channel<=4)
        {
            khzC=(channel*6+45)*1000;
            ui->FrequencyC->display(khzC);

        }
        if(channel>=5 && channel<=6)
        {
            khzC=(channel*6+49)*1000;
            ui->FrequencyC->display(khzC);

        }

        if(channel>=7 && channel<=13)
        {
            khzC=(channel*6+135)*1000;
            ui->FrequencyC->display(khzC);

        }
        if(channel>=14 && channel<=51)
        {
            khzC=(channel*6+389)*1000;
            ui->FrequencyC->display(khzC);

        }
        //channelGood(channel);

            if(channelGood(channel)==true)     // see if channel is exist
            {
               palette->setColor(QPalette::Text,Qt::red);
               ui->channelCspinBox->setPalette(*palette);
               ui->tunerNameC->clear();
               ui->tunerNameC->insert(globalName);
            }
}

void tts::on_channelDspinBox_valueChanged(int channel)
{
    //    this will take the channel and convert it to kilohertz

        QPalette* palette = new QPalette();
        palette->setColor(QPalette::Text,Qt::black);

        ui->channelDspinBox->setPalette(*palette);

        channel=ui->channelDspinBox->value();

        if(channel>=2 && channel<=4)
        {
            khzD=(channel*6+45)*1000;
            ui->FrequencyD->display(khzD);

        }
        if(channel>=5 && channel<=6)
        {
            khzD=(channel*6+49)*1000;
            ui->FrequencyD->display(khzD);

        }

        if(channel>=7 && channel<=13)
        {
            khzD=(channel*6+135)*1000;
            ui->FrequencyD->display(khzD);

        }
        if(channel>=14 && channel<=51)
        {
            khzD=(channel*6+389)*1000;
            ui->FrequencyD->display(khzD);

        }
        //channelGood(channel);

            if(channelGood(channel)==true)     // see if channel is exist
            {
               palette->setColor(QPalette::Text,Qt::red);
               ui->channelDspinBox->setPalette(*palette);
               ui->tunerNameD->clear();
               ui->tunerNameD->insert(globalName);
            }
}

void tts::endScan()
{
   // QProcess *ttsA = dynamic_cast<QProcess *>(sender());



}


void tts::stopTunerA()
{
    ttsA->waitForFinished();
    if(ttsA->state() != QProcess::NotRunning);
        ttsA->kill();

        reconfCreate();
}

void tts::reconfCreate()
{
    // chop down the tts.conf file reorganize and create tts.reconf
    int foundtuneto;
    int foundcolon;
    int found8V;
    int foundlastcolon;
    int len;
    int proglen;
    int rmvlen;

   // QStringList channelsVirtual;


    QString temp;
 //   QString temp2;

    QString line;
    QString homepath = QDir::homePath();
    QString program;
    ui->textBrowser1->clear(); // clear the text editor

    // **********************************************
    // BIG NOTE TO MYSELF  BIG NOTE TO MYSELF
    // MAKE SURE YOU CHANGE THE "tts1.conf"
    // BACK TO                  "tts.conf"
    // END OF NOTE   END OF NOTE
    // ************************************************

    QFile filein(homepath+"/tts.conf");
    if(!filein.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream streamin(&filein);

    line = filein.readLine(0);

 //   the routine below needs to be done in a
 // succint manner so that errors do not occur
 // in the ouput

    while(!filein.atEnd())
    {
        line = filein.readLine(0);
        foundtuneto=line.indexOf("Done");
        if(foundtuneto!=-1)
        {
            while(!filein.atEnd())
            {
                line = filein.readLine(0);
                len = line.length()-1;
                foundlastcolon=line.lastIndexOf(":");
                proglen=len-foundlastcolon;
                program = line.right(proglen);
                program.prepend(":program=");
                found8V=line.indexOf(":8V");
                rmvlen=len-found8V+1;
                line.remove(found8V,rmvlen);
                line.append(program);
                line.prepend("Name=");
                foundcolon= line.indexOf(":");
                temp=line.mid(foundcolon+1,3);
                cnvfrqtchnl(temp);
                line.insert(foundcolon+1,"f=");
                line.prepend(globalChannel);


                ui->textBrowser1->insertPlainText(line);

            }
        }
    }
//   QString homepath = QDir::homePath();

    homepath.append("/tts.reconf");


    QFile file(homepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
           return ;


    QTextStream out(&file);

    out<<ui->textBrowser1->toPlainText();
 //   file.close();
    statusBar()->showMessage(tr("File - %1 - Saved")
            .arg(homepath));
    streamin.flush();
    out.flush();

}
void tts::cnvfrqtchnl(QString temp)
{
    if(temp=="570")
        globalChannel="CH 2:";
    else if(temp=="630")
        globalChannel="CH 3:";
    else if(temp=="690")
        globalChannel="CH 4:";
    else if(temp=="790")
        globalChannel="CH 5:";
    else if(temp=="850")
        globalChannel="CH 6:";

    else
    {

        int freq1;
        int freq2;

        freq1=temp.toInt();

        if(freq1<472)
            freq2=freq1-135;
        else
            freq2=freq1-389;

        freq1=freq2/6;
        globalChannel=QString::number(freq1);
        globalChannel.prepend("CH ");
        globalChannel.append(":");
    }
}




void tts::loadReconf()
{
    QString  line;
    QString homepath = QDir::homePath();
    homepath.append("/tts.reconf");
    QFile filein(homepath);
    filein.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream streamin(&filein);

    while(!filein.atEnd())
    {
        line=filein.readLine(0);
        ui->textBrowser1->insertPlainText(line);
    }


}

void tts::on_showStreamradioButt_clicked()
{



}

bool tts::channelGood(int value)
{
    QString channel=QString::number(value);
   // ui->textBrowser->insertPlainText(channel);

    QString line;
 //   QString channel;
    QString lineChannel;

    int foundCH;
    int found;
    int foundName;  // index for "Name="
    int found2;     // second colon ":f"
    int lineChannelsize;

    QString homepath = QDir::homePath();
    homepath.append("/tts.reconf");
    QFile filein(homepath);
    filein.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream streamin(&filein);

    while(!filein.atEnd())
    {
        line = filein.readLine(0);
        foundCH=line.indexOf("CH ");
        found=line.indexOf(":");
        foundName=line.indexOf("Name=");
        found2=line.indexOf(":f");

        lineChannelsize=found-(foundCH+3);
        lineChannel= line.mid(foundCH+3,lineChannelsize);

        globalName=line.mid(foundName+5,(found2)-(foundName+5));

        if(lineChannel==channel)
        {
            filein.close();
            streamin.flush();
            return true;
        }

    }
        filein.close();
        streamin.flush();
        return false;
}

void tts::check_adapter_FE()
{

    ui->textBrowser->clear();
    for(int cnt=0;cnt<4;cnt++)



    {
        fetype=-1;
        fename.clear();
        QString adapter = QString::number(cnt);
        open_fd(cnt);

        switch (fetype)
        {

        case 0:
        {
            stuff="QPSK";
            break;
        }
        case 1:
        {
            stuff="QAM";
            break;
        }
        case 2:
        {
            stuff="OFDM";
            break;
        }

        case 3:
        {
            stuff="ATSC";
            break;
        }
        case -1:
        {
            stuff="NONE";
            break;
        }
        }


        ui->textBrowser->insertPlainText("Adapter"+adapter+" is "+fename+" "+stuff+"\n");
        if(cnt==0)
        {
            ui->tunerNameA->insert(stuff);
            if(fetype!=-1)
                ui->tunerAgroupBox->setEnabled(true);
        }
        if(cnt==1)
        {
            ui->tunerNameB->insert(stuff);
            if(fetype!=-1)
                ui->tunerBgroupBox->setEnabled(true);
        }
        if(cnt==2)
        {
            ui->tunerNameC->insert(stuff);
            if(fetype!=-1)
                ui->tunerCgroupBox->setEnabled(true);
        }
        if(cnt==3)
        {
            ui->tunerNameD->insert(stuff);
            if(fetype!=-1)
                ui->tunerDgroupBox->setEnabled(true);
        }
        // close(fd);

    }
}

void tts::open_fd(int cnt)
{
    switch(cnt)
    {

        case 0:
        {
        frontend_name0 = "/dev/dvb/adapter0/frontend0";

        if((fd0=open(frontend_name0, O_RDONLY | O_NONBLOCK))<0)
        {
            ui->textBrowser->insertPlainText("Failed ");
            fetype=-1;


        }
        else
        {
            ui->textBrowser->insertPlainText("Opened ");

            if(ioctl(fd0,FE_GET_INFO,&fe_stuff0)!=-1)
            {
                fetype=fe_stuff0.type;
                fename=fe_stuff0.name;
                fd0=close();
            }
        }
        break;
      }

    case 1:
        {
        frontend_name1 = "/dev/dvb/adapter1/frontend0";

        if((fd1=open(frontend_name1, O_RDONLY | O_NONBLOCK))<0)
        {
            ui->textBrowser->insertPlainText("Failed ");
            fetype=-1;
        }
        else
        {
            ui->textBrowser->insertPlainText("Opened ");

            if(ioctl(fd1,FE_GET_INFO,&fe_stuff1)!=-1)
            {
                fetype=fe_stuff1.type;
                fename=fe_stuff1.name;
                fd0=close();

            }

        }
        break;
      }

    case 2:
        {
        frontend_name2 = "/dev/dvb/adapter2/frontend0";

        if((fd2=open(frontend_name2, O_RDONLY | O_NONBLOCK))<0)
        {
            ui->textBrowser->insertPlainText("Failed ");
            fetype=-1;
        }
        else
        {
            ui->textBrowser->insertPlainText("Opened ");

            if(ioctl(fd2,FE_GET_INFO,&fe_stuff2)!=-1)
            {
                fetype = fe_stuff2.type;
                fename=fe_stuff2.name;
                fd2=close();

            }

        }
        break;
      }
    case 3:
        {
        frontend_name3 = "/dev/dvb/adapter3/frontend0";

        if((fd3=open(frontend_name3, O_RDONLY | O_NONBLOCK))<0)
        {
            ui->textBrowser->insertPlainText("Failed ");
            fetype=-1;

        }
        else
        {
            ui->textBrowser->insertPlainText("Opened ");

            if(ioctl(fd3,FE_GET_INFO,&fe_stuff3)!=-1)
            {
                fetype = fe_stuff3.type;
                fename=fe_stuff3.name;
                fd3=close();
            }

        }
        break;
      }
    }

}
void tts::on_createStreamsradioButt_toggled(bool checked)
{
    if(checked==true)
        ui->streamGroupBox->setEnabled(true);
    if(checked==false)
        ui->streamGroupBox->setEnabled(false);


}

void tts::on_udpm_Button_toggled(bool checked)
{
    if(checked==true){
        ui->IPlineEdit->clear();
        ui->IPlineEdit->insert("234.15.32.36");
       streamFlag=0x01;
    }
    else
     {
        ui->IPlineEdit->clear();
        ui->IPlineEdit->insert("127.0.0.1");
        streamFlag=0x02;
    }
}

