#include "widget.h"
#include "ui_widget.h"

//构造函数
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //界面初始化
    setWindowTitle("两江新区公租房智慧人脸识别门禁系统 V1.0.0.1");
    ui->label_stat->setText("初始化....");
    ui->label_logo->setPixmap(QPixmap(":/logo/logo.jpg"));
    ui->label_logo->setScaledContents(true);
    ui->label_logo->setFixedSize(700,496);
    //showMaximized();
    showFullScreen();
    //codec = QTextCodec::codecForName("GBK");

    //显示当前时间和请求包计时器初始化
    time_now = new QTimer(this);
    time_httpRequest = new QTimer(this);
    connect(time_now, SIGNAL(timeout()), this, SLOT(ShowTime()));
    connect(time_httpRequest, SIGNAL(timeout()), this, SLOT(HttpRequest()));
    //heart_time_out = 0;

    //站点配置信息赋初值
    ip = DEFAULT_IP;
    port = DEFAULT_PORT;
    camid = DEFAULT_CAMID;
    posid = DEFAULT_POSID;
    posname = DEFAULT_POSNAME;

    //读取ini文件更新站点配置信息
    config_file_name = QCoreApplication::applicationDirPath() + "/config";
    pos_info = new QSettings(config_file_name, QSettings::IniFormat);
    QFileInfo fileinfo(config_file_name);
    if(!fileinfo.exists())
    {
        qDebug("%s is not exists!", config_file_name.toLatin1().data());
        this->SetConfigData(pos_info, "server", "ip", DEFAULT_IP);
        this->SetConfigData(pos_info, "server", "port", DEFAULT_PORT);
        this->SetConfigData(pos_info, "posinfo", "camid", DEFAULT_CAMID);
        this->SetConfigData(pos_info, "posinfo", "posid", DEFAULT_POSID);
        this->SetConfigData(pos_info, "posinfo", "posname", DEFAULT_POSNAME);
    }
    else
    {
        ip = this->GetConfigData(pos_info, "server", "ip").toString();
        port = this->GetConfigData(pos_info, "server", "port").toInt();
        camid = this->GetConfigData(pos_info, "posinfo", "camid").toString();
        posid = this->GetConfigData(pos_info, "posinfo", "posid").toString();
        posname = this->GetConfigData(pos_info, "posinfo", "posname").toString();
        qDebug("%s %d %s %s %s", ip.toLatin1().data(), port, camid.toLatin1().data(), posid.toLatin1().data(), posname.toLatin1().data());
    }

    //http请求对象初始化
    naManager = new QNetworkAccessManager(this);
    connect(naManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(RequestFinished(QNetworkReply *)));

    //提示音对象初始化
    effect = new QSoundEffect;
    wav_file_name = QCoreApplication::applicationDirPath() + "/enter.wav";
    effect->setSource(QUrl::fromLocalFile(wav_file_name));
    effect->setLoopCount(1);    //循环次数
    effect->setVolume(1);   //音量0~1之间

    //提示窗口对象初始化
    msgDialog = new QMsgDialog(this);
    Qt::WindowFlags flags = msgDialog->windowFlags();
    msgDialog->setWindowFlags(flags | Qt::MSWindowsFixedSizeDialogHint);
    msgDialog->setModal(false);
    //QTimer::singleShot(DISPLAY_TIME, msgDialog, SLOT(accept()));

    //显示时间计时器开始运行
    time_now->start(1000);

    //初始化本站请求字符串
    httpGetStr = ip + "?camid=" + camid + "&posid=" + posid;

    //读取本地ip地址
    ipaddr = getlockIp();

    //延时后开始请求HTTP
    SleepMs(REBOOTWAIT_TIME);
    time_httpRequest->start(HTTP_INTERVAL);

    ui->label_stat->setText(camid + "    运行中...." + "   IP:" + ipaddr);
}

//析构函数
Widget::~Widget()
{
    delete msgDialog;
    delete ui;
}

//显示当前时间
void Widget::ShowTime()
{
    QDateTime nowtime = QDateTime::currentDateTime();
    nowTime = nowtime.toString("yyyy年MM月dd日 hh:mm:ss");
    ui->label_now->setText("当前时间：" + nowTime);
}

//HTTP请求槽函数
void Widget::HttpRequest()
{
    try
    {
        ui->label_stat->setText(camid + "    请求数据....");
        qDebug() << httpGetStr;
        request.setUrl(QUrl(httpGetStr));
        naManager->get(request);
        ui->label_stat->setText(camid + "    运行中...." + "   IP:" + ipaddr);
    }
    catch(...)
    {
        ui->label_stat->setText(camid + "    请求错误....");
    }
}

//HTTP请求回复槽函数
void Widget::RequestFinished(QNetworkReply *reply)
{
    //获取状态码
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(statusCode.isValid())
    {
        qDebug() << "status code = " << statusCode.toInt();
    }

    //获取状态原因
    QVariant reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    if(reason.isValid())
    {
        qDebug() << "reason = " << reason.toString();
    }

    //解析数据
    QNetworkReply::NetworkError err = reply->error();
    if(err != QNetworkReply::NoError)
    {
        qDebug() << "Failed: " << reply->errorString();
    }
    else
    {
        //收数据至buff_str
        QString buff_str = "";
        buff_str.append(reply->readAll());
        qDebug() << buff_str.toUtf8();

        //按实际json结构解析buff_str
        try
        {
            str_ba = buff_str.toUtf8();
            doucment = QJsonDocument::fromJson(str_ba, &jsonError);
            if (!doucment.isNull() && jsonError.error == QJsonParseError::NoError)
            {
                if (doucment.isObject())
                {
                    QJsonObject object = doucment.object();

                    /*
                    if (object.contains("data"))    //包含指定的 key
                    {
                        QJsonValue value = object.value("data");
                        if (value.isArray())    //判断value是否为数组
                        {
                            QJsonArray array = value.toArray();
                            QJsonValue arr_val = array.at(0);
                            if(arr_val.type() == QJsonValue::Object)    //类型为object
                            {
                                QJsonObject ob = arr_val.toObject();
                                jsonname = ob.value("nhName").toString();
                                qDebug() << "Recvie json --> nhName_O : " << jsonname;
                            }
                            if(arr_val.type() == QJsonValue::String)    //类型为string
                            {
                                jsonname = arr_val.toString();
                                qDebug() << "Recvie json --> nhName_S : " << jsonname;
                            }
                            if(arr_val.type() == QJsonValue::Double)    //类型为double
                            {
                                qDebug() << "Recvie json --> nhName_D : " << arr_val.toDouble();
                            }
                            if(arr_val.type() == QJsonValue::Bool)  //类型为bool
                            {
                                qDebug() << "Recvie json --> nhName_B : " << arr_val.toBool();
                            }
                        }
                    }
                    if (object.contains("status"))
                    {
                        QJsonValue value = object.value("status");
                        if (value.isString())
                        {
                            jsontime = value.toString();
                            qDebug() << "Recvie json --> status : " << jsontime;
                        }
                    }
                    */

                    if (object.contains("name") && object.contains("time")) //解析到人员进入
                    {
                        QJsonValue value_name = object.value("name");
                        QJsonValue value_time = object.value("time");
                        if (value_name.isString() && value_time.isString())
                        {
                            jsonname = value_name.toString();
                            jsontime = value_time.toString();
                            qDebug() << "Recvie json --> name : " << jsonname;
                            qDebug() << "Recvie json --> time : " << jsontime;

                            effect->play(); //播放提示音

                            //格式化显示字符串dis_str
                            dis_str.clear();
                            //dis_str.append("<font size='72' color='green'>");
                            dis_str.append("<font color='green'>");
                            dis_str.append("欢迎    ");
                            dis_str.append("</font>");
                            dis_str.append("<font color='red'>");
                            dis_str.append(jsonname.left(1) + "*");
                            dis_str.append("</font>");
                            dis_str.append("<font color='green'>");
                            dis_str.append("    进入!");
                            dis_str.append("</font>");
                            //dis_str.append("<br></br>");
                            //dis_str.append("<font color='blue'>");
                            //dis_str.append(nowTime);
                            //dis_str.append("</font>");

                            //QMessageBox *recv_msg = new QMessageBox(QMessageBox::Information, QString("人员进入通知"), QString(dis_str));
                            //QTimer::singleShot(DISPLAY_TIME,recv_msg,SLOT(accept()));
                            //recv_msg->setModal(false);
                            //recv_msg->show();

                            //显示人员进入提示窗
                            msgDialog->hide();
                            QTimer::singleShot(DISPLAY_TIME, msgDialog, SLOT(accept()));
                            msgDialog->setText(dis_str, nowTime);
                            //msgDialog->exec();
                            msgDialog->show();
                        }
                    }
                }
            }
        } catch(...){}
    }
}

//写配置文件
void Widget::SetConfigData(QSettings *psetting, QString qstrnodename, QString qstrkeyname, QVariant qvarvalue)
{
    if(psetting)
    {
        psetting->setValue(QString("/%1/%2").arg(qstrnodename).arg(qstrkeyname), qvarvalue);
    }
}

//读配置文件
QVariant Widget::GetConfigData(QSettings *psetting, QString qstrnodename, QString qstrkeyname)
{
    QVariant qvar = -1;
    if(psetting)
    {
        qvar = psetting->value(QString("/%1/%2").arg(qstrnodename).arg(qstrkeyname));
    }
    return qvar;
}

//延时函数
void Widget::SleepMs(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

QString Widget::getlockIp()
{
    QString ip_address;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (int i = 0; i < ipAddressesList.size(); ++i)
    {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&  ipAddressesList.at(i).toIPv4Address())
        {
            ip_address = ipAddressesList.at(i).toString();
            qDebug()<<ip_address;  //debug
            //break;
        }
    }
    if (ip_address.isEmpty())
        ip_address = QHostAddress(QHostAddress::LocalHost).toString();
    return ip_address;
}

