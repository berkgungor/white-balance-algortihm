#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFile"
#include "logger.h"
#include <QSerialPortInfo>
#include <vector>
#include "QMessageBox"
#include "arserialport.h"
#include <QDebug>
#include <math.h>
#include <ctime>


#define WAIT_AFTER_COMMAND 1
#define WAIT_AFTER_SET_PATTERN 1
#define WAIT_AFTER_MEASURED 1

std::vector<ColorAnalyzerValues> values;
logger myLog("log.txt");

QByteArray Start_Session()
{
    QByteArray arr;
    bool ok;
    arr.append(static_cast<char>(START_HEADER));
    arr.append(static_cast<char>(QString("04").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("01").toUInt(&ok,16)));
    arr.append(static_cast<char>(ArSerialPort::CalculateCheckSum(arr)));

    return arr;
}

QByteArray Set_Internal_Pattern()
{
    QByteArray arr;
    bool ok;
    arr.append(static_cast<char>(START_HEADER));
    arr.append(static_cast<char>(QString("05").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("31").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("00").toUInt(&ok,16))); //No internal pattern
    arr.append(static_cast<char>(ArSerialPort::CalculateCheckSum(arr)));

    return arr;
}

QByteArray Set_Pic_WBA_Default()
{
    QByteArray arr;
    bool ok;
    arr.append(static_cast<char>(START_HEADER));
    arr.append(static_cast<char>(QString("04").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("79").toUInt(&ok,16)));
    arr.append(static_cast<char>(ArSerialPort::CalculateCheckSum(arr)));

    return arr;
}

QByteArray Set_WBA_Color_Temp_Setting(int C_N_W) //C_N_W=0 Cool, C_N_W=1 Normal, C_N_W=2 Warm
{
    QByteArray arr;
    bool ok;
    arr.append(static_cast<char>(START_HEADER));
    arr.append(static_cast<char>(QString("06").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("80").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("17").toUInt(&ok,16)));
    arr.append(static_cast<char>(C_N_W));
    arr.append(static_cast<char>(ArSerialPort::CalculateCheckSum(arr)));

    return arr;
}

QByteArray Set_OP2_Internal_RGB()
{
    QByteArray arr;
    bool ok;
    arr.append(static_cast<char>(START_HEADER));
    arr.append(static_cast<char>(QString("06").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("80").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("13").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("CC").toUInt(&ok,16)));
    arr.append(static_cast<char>(ArSerialPort::CalculateCheckSum(arr)));

    return arr;
}

QByteArray Set_RGB_Values(int C_N_W,int R_G_B,int Value)
{
    QByteArray arr;
    bool ok;
    arr.append(static_cast<char>(START_HEADER));
    arr.append(static_cast<char>(QString("07").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("03").toUInt(&ok,16)));
    arr.append(static_cast<char>(C_N_W));
    arr.append(static_cast<char>(R_G_B));
    arr.append(static_cast<char>(Value));
    arr.append(static_cast<char>(ArSerialPort::CalculateCheckSum(arr)));

    return arr;
}

QByteArray Exit_OP2_Internal_Pattern_Mode()
{
    QByteArray arr;
    bool ok;
    arr.append(static_cast<char>(START_HEADER));
    arr.append(static_cast<char>(QString("05").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("80").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("12").toUInt(&ok,16)));
    arr.append(static_cast<char>(ArSerialPort::CalculateCheckSum(arr)));

    return arr;
}

QByteArray Copy_WBA_EEPROM()
{
    QByteArray arr;
    bool ok;
    arr.append(static_cast<char>(START_HEADER));
    arr.append(static_cast<char>(QString("06").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("0B").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("00").toUInt(&ok,16)));
    arr.append(static_cast<char>(QString("00").toUInt(&ok,16)));
    arr.append(static_cast<char>(ArSerialPort::CalculateCheckSum(arr)));

    return arr;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    serialPort = new ArSerialPort();
    minolta = new ColorAnalyzerMinolta();
    ui->setupUi(this);
    this->setFixedSize(QSize(this->size()));


    for( auto a : QSerialPortInfo::availablePorts())
    {
        ui->comboBox_minolta->addItem(a.portName());
        ui->comboBox_debugTool->addItem(a.portName());
    }
}

void writeValuesToCSV(const ColorAnalyzerValues &c,int R, int G, int B, QTextStream &output)
{
    output << R << "," << G << "," << B << "," << c.fChannel1 << ","  << c.fChannel2 << "," << c.fChannel3 << endl;
}

void writeValuesToCSV(const std::vector<ColorAnalyzerValues> & v, QTextStream &output)
{
    for(auto a : v)
    {
        output <<  "\"0," << a.fChannel1 << "\"" << ",";
    }
    output << endl;
    for(auto a : v)
    {
        output <<  "\"0," << a.fChannel2 << "\"" << ",";
    }
    output << endl;
    for(auto a : v)
    {
        QString Lv = QString::number(a.fChannel3).replace(".",",");
        output <<  "\"" << Lv << "\"" << ",";
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete serialPort;
    delete minolta;
}

void MainWindow::on_actionactionRefresh_Ports_2_triggered()
{
    ui->comboBox_minolta->clear();
    ui->comboBox_debugTool->clear();
    for( auto a : QSerialPortInfo::availablePorts())
    {
        ui->comboBox_minolta->addItem(a.portName());
        ui->comboBox_debugTool->addItem(a.portName());
    }
}

void MainWindow::on_debugToolConnectButton_clicked()
{
    if(serialPort->StartConnection(ui->comboBox_debugTool->currentText(),ui->debugToolBaudRate->text().toInt()))
    {
        ui->connection_label_debugTool->setText("OK");
    }
    else
    {
        myLog.write("Can not open debug tool port",logFlags::error);
    }
}

void MainWindow::on_debugToolDisconnectButton_clicked()
{
    serialPort->StopConnection();
    ui->connection_label_debugTool->setText("NOK");
}

void MainWindow::on_minoltaConnectButton_clicked()
{
    if(minolta->Connect(ui->comboBox_minolta->currentText(),static_cast<QSerialPort::BaudRate>(ui->minoltaBaudRate->text().toInt())))
    {
        ui->connection_label_minolta->setText("OK");
    }
    else
    {
        myLog.write("Can not open minolta port",logFlags::error);
    }
}

void MainWindow::on_minoltaDisconnectButton_clicked()
{
    minolta->Disconnect();
    ui->connection_label_minolta->setText("NOK");
}


void MainWindow::on_start_button_clicked()
{
    double a1=255, a2=255, a3=255, a4=255, a5=255, a6=255, a7=128, a8=128, a9=128;

    int b1=0, b2=0, b3=0;

    serialPort->Write(Start_Session());

    serialPort->Write(Set_Internal_Pattern());

    serialPort->Write(Set_Pic_WBA_Default());

    serialPort->Write(Set_WBA_Color_Temp_Setting(0));

    QThread::msleep(150);

    serialPort->Write(Set_OP2_Internal_RGB());

    QThread::msleep(150);

    auto c = minolta->GetMeasuredValues();

    ui->initial_x->setText(QString::number(c.fChannel1/10000));
    ui->initial_y->setText(QString::number(c.fChannel2/10000));
    ui->initial_lv->setText(QString::number(c.fChannel3));

    clock_t start = clock(), end;
    float mahir =(float)start;

    double r_x = 0.64;
    double r_y = 0.33;
    double r_z = 1 - r_x - r_y;

    double g_x = 0.3;
    double g_y = 0.6;
    double g_z = 1 - g_x - g_y;

    double b_x = 0.15;
    double b_y = 0.06;
    double b_z = 1 - b_x - b_y;


    double w_x = c.fChannel1/10000;
    double w_y = c.fChannel2/10000;
    double w_z = 1 - w_x - w_y;

    z:

    double K[3][3]= { { r_x, r_y, r_z }, { g_x, g_y, g_z }, { b_x, b_y, b_z } };


	// ------------------------------------------------------------------------------- LABVIEW GECÝRÝLDÝ

    double W_x = w_x / w_y;
    double W_y = 1;
    double W_z = w_z / w_y;

    double W [1][3]= { { W_x, W_y, W_z } };

    //K matrisinin tersi

    double determinant = 0;

    double K_i [3][3];

    for (int i = 0; i < 3; i++)
         determinant = determinant + (K[0][i] * (K[1][(i + 1) % 3] * K[2][(i + 2) % 3] - K[1][(i + 2) % 3] * K[2][(i + 1) % 3]));

               for (int i = 0; i < 3; i++)
                   for (int j = 0; j < 3; j++)
                       K_i[i][j] = ((K[(i + 1) % 3][(j + 1) % 3] * K[(i + 2) % 3][(j + 2) % 3]) - (K[(i + 1) % 3][(j + 2) % 3] * K[(i + 2) % 3][(j + 1) % 3])) / determinant;

               double V [1][3]={0,0,0};

               //V=W*K_i

               for (int i = 0; i < 3; i++)
                   for (int j = 0; j < 3; j++)
                       V[0][i] += (W[0][j] * K_i[i][j]);

               double G [3][3]= { { V[0][0], 0, 0 }, { 0, V[0][1], 0 }, { 0, 0, V[0][2] } };

               double N [3][3];

               // N=G*K

               for (int i = 0; i <= 2; i++)
                   for (int j = 0; j <= 2; j++)
                   {
                       double toplam = 0;
                       for (int k = 0; k <= 2; k++)
                       {
                           toplam += G[i][k] * K[k][j];
                       }

                       N[i][j] = toplam;
                   }


               //M=N_i
               double determinant_1 = 0;

               double M [3][3];
               for (int i = 0; i < 3; i++)
                   determinant_1 += (N[0][i] * (N[1][(i + 1) % 3] * N[2][(i + 2) % 3] - N[1][(i + 2) % 3] * N[2][(i + 1) % 3]));

               for (int i = 0; i < 3; i++)
                   for (int j = 0; j < 3; j++)
                       M[i][j] = ((N[(i + 1) % 3][(j + 1) % 3] * N[(i + 2) % 3][(j + 2) % 3]) - (N[(i + 1) % 3][(j + 2) % 3] * N[(i + 2) % 3][(j + 1) % 3])) / determinant_1;

               double C_T_Y = 255;
               double N_T_Y = 255;
               double W_T_Y = 255;

               b:

               double C_T [3][1] { { 0.272 / 0.278 * C_T_Y }, { C_T_Y }, { (1 - 0.272 - 0.278) / 0.278 * C_T_Y } };
               double N_T [3][1] { { 0.283 / 0.297 * N_T_Y }, { N_T_Y }, { (1 - 0.283 - 0.297) / 0.297 * N_T_Y } };
               double W_T [3][1] { { 0.3127 / 0.329 * W_T_Y }, { W_T_Y }, { (1 - 0.3127 - 0.329) / 0.329 * W_T_Y } };

               double R_C [3][1];
               double R_N [3][1];
               double R_W [3][1];

               for (int i = 0; i <= 2; i++)
                   for (int j = 0; j <= 0; j++)
                   {
                       double toplam = 0;
                       for (int k = 0; k <= 2; k++)
                       {
                           toplam += M[i][k] * C_T[k][j];
                       }

                       R_C[i][j] = toplam;

                   }

               for (int i = 0; i < 3; i++)
               {
                   if (R_C[i][0] / 255 > 0.0031308)
                       R_C[i][0] = ((pow((R_C[i][0] / 255), 0.4166666666666667) * 1.055) - 0.055) * 255;
                   else
                       R_C[i][0] = R_C[i][0] * 12.92;

                 if (R_C[i][0] > 255.01)
                   {
                       C_T_Y = C_T_Y - 0.15;
                       goto b;
                   }
               }

               for (int i = 0; i <= 2; i++)
                   for (int j = 0; j <= 0; j++)
                   {
                       double toplam = 0;
                       for (int k = 0; k <= 2; k++)
                       {
                           toplam += M[i][k] * N_T[k][j];
                       }

                       R_N[i][j] = toplam;

                   }
               for (int i = 0; i < 3; i++)
               {
                   if (R_N[i][0] / 255 > 0.0031308)
                       R_N[i][0] = ((pow((R_N[i][0] / 255), 0.4166666666666667) * 1.055) - 0.055) * 255;
                   else
                       R_N[i][0] = R_N[i][0] * 12.92;

                  if (R_N[i][0] > 255.01)
                   {
                       N_T_Y = N_T_Y - 0.15;
                       goto b;
                   }
               }

               for (int i = 0; i <= 2; i++)
                   for (int j = 0; j <= 0; j++)
                   {
                       double toplam = 0;
                       for (int k = 0; k <= 2; k++)
                       {
                           toplam += M[i][k] * W_T[k][j];
                       }

                       R_W[i][j] = toplam;

                   }
               for (int i = 0; i < 3; i++)
               {
                   if (R_W[i][0] / 255 > 0.0031308)
                       R_W[i][0] = ((pow((R_W[i][0] / 255), 0.4166666666666667) * 1.055) - 0.055) * 255;
                   else
                       R_W[i][0] = R_W[i][0] * 12.92;

                 if (R_W[i][0] > 255.01)
                   {
                       W_T_Y = W_T_Y - 0.15;
                       goto b;
                   }
               }



    if(b1==0)
    {
        double R_C_R, R_C_G, R_C_B;
        R_C_R = R_C[0][0]/R_C[1][0]*255;
        R_C_G = R_C[1][0]/R_C[1][0]*255;
        R_C_B = R_C[2][0]/R_C[1][0]*255;

    serialPort->Write(Set_RGB_Values(0,0,rint(rint(R_C_R*a1*128/(255*255)))));
    serialPort->Write(Set_RGB_Values(0,1,rint(rint(R_C_G*a2*128/(255*255)))));
    serialPort->Write(Set_RGB_Values(0,2,rint(rint(R_C_B*a3*128/(255*255)))));

    QThread::msleep(150);

    auto m = minolta->GetMeasuredValues();

    ui->cool_x->setText(QString::number(m.fChannel1/10000));
    ui->cool_y->setText(QString::number(m.fChannel2/10000));
    ui->cool_lv->setText(QString::number(m.fChannel3));

    if(m.fChannel1>2770 || m.fChannel1<2670 || m.fChannel2>2830 || m.fChannel2<2730)
        {
            w_x = m.fChannel1/10000;
            w_y = m.fChannel2/10000;
            w_z = 1 - w_x - w_y;

            a1=R_C_R;
            a2=R_C_G;
            a3=R_C_B;

            goto z;
        }
    }

    if(b2==0)
    {
    serialPort->Write(Set_WBA_Color_Temp_Setting(1));

    serialPort->Write(Set_Pic_WBA_Default());

    serialPort->Write(Set_OP2_Internal_RGB());

    serialPort->Write(Set_RGB_Values(1,0,rint(R_N[0][0]*a4*128/(255*255))));
    serialPort->Write(Set_RGB_Values(1,1,rint(R_N[1][0]*a5*128/(255*255))));
    serialPort->Write(Set_RGB_Values(1,2,rint(R_N[2][0]*a6*128/(255*255))));

    QThread::msleep(150);

    auto n = minolta->GetMeasuredValues();

    ui->normal_x->setText(QString::number(n.fChannel1/10000));
    ui->normal_y->setText(QString::number(n.fChannel2/10000));
    ui->normal_lv->setText(QString::number(n.fChannel3));

    if(n.fChannel1/10000>0.2880 || n.fChannel1/10000<0.2780 || n.fChannel2/10000>0.3020 || n.fChannel2/10000<0.2920)
        {
            w_x = n.fChannel1/10000;
            w_y = n.fChannel2/10000;
            w_z = 1 - w_x - w_y;

            a4=R_N[0][0];
            a5=R_N[1][0];
            a6=R_N[2][0];

            b1=1;

            goto z;
        }

    }

    if(b3==0)

    {

    serialPort->Write(Set_WBA_Color_Temp_Setting(2));

    serialPort->Write(Set_OP2_Internal_RGB());

    serialPort->Write(Set_RGB_Values(2,0,rint(a7*R_W[0][0]/255)));
    serialPort->Write(Set_RGB_Values(2,1,rint(a8*R_W[1][0]/255)));
    serialPort->Write(Set_RGB_Values(2,2,rint(a9*R_W[2][0]/255)));

     QThread::msleep(150);

    auto t = minolta->GetMeasuredValues();

    ui->warm_x->setText(QString::number(t.fChannel1/10000));
    ui->warm_y->setText(QString::number(t.fChannel2/10000));
    ui->warm_lv->setText(QString::number(t.fChannel3));

    if(t.fChannel1>3177 || t.fChannel1<3077 || t.fChannel2>3340 || t.fChannel2<3240)
        {
            w_x = t.fChannel1/10000;
            w_y = t.fChannel2/10000;
            w_z = 1 - w_x - w_y;

            a7=rint(128*R_W[0][0]/255);
            a8=rint(128*R_W[1][0]/255);
            a9=rint(128*R_W[2][0]/255);

            b1=1;
            b2=1;

            goto z;
        }
    }

    serialPort->Write(Exit_OP2_Internal_Pattern_Mode());

    serialPort->Write(Copy_WBA_EEPROM());

    end = clock();

    serialPort->Write(Set_Internal_Pattern());

    ui->Time->setText(QString::number((end-mahir)/CLOCKS_PER_SEC));


}



