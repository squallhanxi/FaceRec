#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QDateTime>
#include <QSettings>
#include <QFileInfo>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>
#include <QSoundEffect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QException>
#include <QHostInfo>
#include <QNetworkInterface>
#include "qmsgdialog.h"

#define DEFAULT_IP "127.0.0.1"  //默认服务器IP地址
#define DEFAULT_PORT 8537   //默认服务器端口
#define DEFAULT_CAMID "100001"  //默认设备ID
#define DEFAULT_POSID "200001"  //默认站点IP
#define DEFAULT_POSNAME "POSTEST"   //默认站点名称
//#define HTTPREQ "http://dev.management.gzf.sczlcq.com/public/selectResidentialQuartersList.do?nhCode=all"   //默认HTTP请求地址
//#define HTTPREQ "http://139.155.52.181/Face/public/index/Face/peoplein.html?camid=111&posid=abc"   //默认HTTP请求地址
#define DISPLAY_TIME 5000   //人员进入事件显示时间
#define HTTP_INTERVAL 2000    //请求包间隔
#define REBOOTWAIT_TIME 300    //启动后等待时间

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    //QTextCodec *codec;

    //申明显示时间和请求包计时器
    QTimer *time_now;
    QTimer *time_httpRequest;
    QString nowTime;

    //申明读取ini文件的相关变量
    QSettings *pos_info;
    QString config_file_name;

    //申明站点配置的相关变量
    QString ip;
    int port;
    QString camid;
    QString posid;
    QString posname;

    //申明人员进入消息提示窗口
    QMsgDialog *msgDialog;

    //申明接收及显示http回复包相关变量
    QByteArray str_ba;
    QString dis_str;

    //申明解析json结构数据的相关对象
    QJsonParseError jsonError;
    QJsonDocument doucment;
    QString jsonname;
    QString jsontime;

    //申明HTTP请求相关对象
    QNetworkRequest request;
    QNetworkAccessManager *naManager;

    //申明提示音对象
    QSoundEffect *effect;
    QString wav_file_name;

    //申明本站请求字符串
    QString httpGetStr;

    //申明ip地址变量
    QString ipaddr;

private slots:
    void ShowTime();    //显示当前时间槽函数
    void HttpRequest(); //HTTP请求槽函数
    void RequestFinished(QNetworkReply *reply); //HTTP请求回复槽函数

private:
    Ui::Widget *ui;

    //设置和读取ini文件函数
    void SetConfigData(QSettings *psetting, QString qstrnodename, QString qstrkeyname, QVariant qvarvalue);
    QVariant GetConfigData(QSettings *psetting, QString qstrnodename, QString qstrkeyname);

    //延时函数
    void SleepMs(int msec);

    //获取IP地址
    QString getlockIp();
};

#endif // WIDGET_H
