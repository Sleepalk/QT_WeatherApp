#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"weathertool.h"
#include<QContextMenuEvent>
#include<QAction>
#include<QMenu>
#include<QMouseEvent>
#include<QDebug>
#include<QMessageBox>
#include<QJsonArray>
#include<QJsonDocument>
#include<QJsonObject>
#include<QPainter>
#define INCREMENT 3//温度每升高/降低1°，y轴坐标的增量
#define POINT_RADIUS 3//曲线描点的大小
#define TEXT_OFFSET_X 12
#define TEXT_OFFSET_Y 12
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint);//设置窗口无边框
    this->setFixedSize(width(),height());//固定大小

    //构建右键菜单
    mExitMenu =new QMenu(this);
    mExitAct =new QAction(this);
    connect(mExitAct,&QAction::triggered,this,&MainWindow::close);
    mExitAct->setText("退出");
    mExitAct->setIcon(QIcon(":/icon/res/close.png"));
    mExitMenu->addAction(mExitAct);

    mNetAccessManager= new QNetworkAccessManager(this);
    connect(mNetAccessManager,&QNetworkAccessManager::finished, this,
            &MainWindow::onReplied);


    //将控件添加到数组
    mWeekList<< ui->lblWeek0<< ui->lblWeek1<< ui->lblWeek2<< ui->lblWeek3<<
        ui->lblWeek4<< ui->lblWeek5;
    mDateList<< ui->lblDate0<< ui->lblDate1<< ui->lblDate2<< ui->lblDate3<<
        ui->lblDate4<< ui->lblDate5;
    mTypeIconList<< ui->lblTypeIcon0<< ui->lblTypeIcon1<< ui->lblTypeIcon2<< ui->lblTypeIcon3<<
        ui->lblTypeIcon4<< ui->lblTypeIcon5;
    mTypeList<< ui->lblType0<< ui->lblType1<< ui->lblType2<< ui->lblType3<<
        ui->lblType4<< ui->lblType5;
    mAqiList<< ui->lblQuality0<< ui->lblQuality1<< ui->lblQuality2<< ui->lblQuality3<<
        ui->lblQuality4<< ui->lblQuality5;
    mFxList<< ui->lblFx0<< ui->lblFx1<< ui->lblFx2<< ui->lblFx3<< ui->lblFx4<< ui->lblFx5;
    mFlList<< ui->lblFl0<< ui->lblFl1<< ui->lblFl2<< ui->lblFl3<< ui->lblFl4<< ui->lblFl5;

    mTypeMap.insert("暴雪",":/icon/res/type/BaoXue.png");
    mTypeMap.insert("暴雨",":/icon/res/type/BaoYu.png");
    mTypeMap.insert("暴雨到大暴雨",":/icon/res/type/BaoYuDaoDaBaoYu.png");
    mTypeMap.insert("大暴雨",":/icon/res/type/DaBaoYu.png");
    mTypeMap.insert("大到暴雪",":/icon/res/type/DaDaoBaoXue.png");
    mTypeMap.insert("大暴雨到特大暴雨",":/icon/res/type/DaBaoYuDaoTeDaBaoYu.png");
    mTypeMap.insert("大到暴雨",":/icon/res/type/DaDaoBaoYu.png");
    mTypeMap.insert("大雪",":/icon/res/type/DaXue.png");
    mTypeMap.insert("大雨",":/icon/res/type/DaYu.png");
    mTypeMap.insert("冻雨",":/icon/res/type/DongYu.png");
    mTypeMap.insert("多云",":/icon/res/type/DuoYun.png");
    mTypeMap.insert("浮沉",":/icon/res/type/FuChen.png");
    mTypeMap.insert("雷阵雨",":/icon/res/type/LeiZhenYu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/icon/res/type/LeiZhenYuBanYouBingBao.png");
    mTypeMap.insert("霾",":/icon/res/type/Mai.png");
    mTypeMap.insert("强沙尘暴",":/icon/res/type/QiangShaChenBao.png");
    mTypeMap.insert("晴",":/icon/res/type/Qing.png");
    mTypeMap.insert("沙尘暴",":/icon/res/type/ShaChenBao.png");
    mTypeMap.insert("特大暴雨",":/icon/res/type/TeDaBaoYu.png");
    mTypeMap.insert("undefined",":/icon/res/type/undefined.png");
    mTypeMap.insert("雾",":/icon/res/type/Wu.png");
    mTypeMap.insert("小到中雪",":/icon/res/type/XiaoDaoZhongXue.png");
    mTypeMap.insert("小到中雨",":/icon/res/type/XiaoDaoZhongYu.png");
    mTypeMap.insert("小雪",":/icon/res/type/XiaoYu.png");
    mTypeMap.insert("雪",":/icon/res/type/Xue.png");
    mTypeMap.insert("扬沙",":/icon/res/type/YangSha.png");
    mTypeMap.insert("阴",":/icon/res/type/Yin.png");
    mTypeMap.insert("雨",":/icon/res/type/Yu.png");
    mTypeMap.insert("雨夹雪",":/icon/res/type/YuJiaXue.png");
    mTypeMap.insert("阵雨",":/icon/res/type/ZhenYu.png");
    mTypeMap.insert("阵雪",":/icon/res/type/ZhenXue.png");
    mTypeMap.insert("中到大雪",":/icon/res/type/ZhongDaoDaXue.png");
    mTypeMap.insert("中到大雨",":/icon/res/type/ZhongDaoDaYu.png");
    mTypeMap.insert("中雪",":/icon/res/type/ZhongXue.png");
    mTypeMap.insert("中雨",":/icon/res/type/ZhongYu.png");
    mTypeMap.insert("小雨",":/icon/res/type/XiaoYu.png");

    //getWeatherInfo("101010100");//北京的天气信息(打开app时先获取北京的天气并显示在界面上)
    //getWeatherInfo("101280101");
    getWeatherInfo("南昌");

    //给标签添加事件过滤器
    ui->lblHighCurve->installEventFilter(this);//参数指定为this，也就是当前窗口对象为MainWindow
    ui->lblLowCurve->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onReplied(QNetworkReply *reply)
{
    //qDebug() << "onReplied sussess";
    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(reply->error() != QNetworkReply::NoError || status_code != 200)
    {//如果程序有错误或者状态码不等于200
        QMessageBox::warning(this,"天气","请求数据失败");
    }
    else
    {
        QByteArray byteArray= reply->readAll();
        //qDebug()<<"read all:"<< byteArray.data();
        parseJson(byteArray);
    }
    reply->deleteLater();
}

void MainWindow::getWeatherInfo(QString cityName)
{//获取天气信息
    //qDebug()<<cityName;
    QString cityCode= WeatherTool::getCityCode(cityName);
    //qDebug()<<cityCode;
    if(cityCode.isEmpty())
    {
        QMessageBox::warning(this,"天气","请检查输入是否正确");
        return;
    }
    QUrl url("http://t.weather.itboy.net/api/weather/city/"+cityCode);
    mNetAccessManager->get(QNetworkRequest(url));
}

void MainWindow::parseJson(QByteArray &byteArray)
{//解析JSON
    QJsonParseError err;
    QJsonDocument doc= QJsonDocument::fromJson(byteArray,&err);
    if(err.error != QJsonParseError::NoError)
    {
        return;
    }
    QJsonObject rootobj= doc.object();
    QJsonValue value=rootobj.value("message");
    //qDebug()<<value.toString();
    //1.解析日期和城市
    mToday.date= rootobj.value("date").toString();
    mToday.city= rootobj.value("cityInfo").toObject().value("city").toString();

    //2.解析yesterday
    QJsonObject objData= rootobj.value("data").toObject();
    QJsonObject objYesterday= objData.value("yesterday").toObject();
    mDay[0].week= objYesterday.value("week").toString();
    mDay[0].date= objYesterday.value("ymd").toString();
    mDay[0].type= objYesterday.value("type").toString();

    QString s= objYesterday.value("high").toString().split(" ").at(1);
    s= s.left(s.length()-1);
    mDay[0].high=s.toInt();

    s= objYesterday.value("low").toString().split(" ").at(1);
    s= s.left(s.length()-1);
    mDay[0].low=s.toInt();

    mDay[0].fx=objYesterday.value("fx").toString();

    mDay[0].fl=objYesterday.value("fl").toString();

    mDay[0].api= objYesterday.value("aqi").toDouble();
    //3.解析forcast中5天的数据
    QJsonArray forecastArr= objData.value("forecast").toArray();
    for(int i=0;i<5;i++)
    {
        QJsonObject objforecast= forecastArr[i].toObject();
        mDay[i+1].week= objforecast.value("week").toString();
        mDay[i+1].date= objforecast.value("ymd").toString();
        mDay[i+1].type= objforecast.value("type").toString();

        s= objforecast.value("high").toString().split(" ").at(1);
        s= s.left(s.length()-1);
        mDay[i+1].high=s.toInt();
        s= objforecast.value("low").toString().split(" ").at(1);
        s= s.left(s.length()-1);
        mDay[i+1].low=s.toInt();
        mDay[i+1].fx=objforecast.value("fx").toString();

        mDay[i+1].fl=objforecast.value("fl").toString();

        mDay[i+1].api= objforecast.value("aqi").toDouble();
    }

    //4.解析今天的数据
    mToday.ganmao= objData.value("ganmao").toString();
    //bool conversionSuccess= false;
    mToday.wendu= objData.value("wendu").toString();
    // if (!conversionSuccess) {
    //     // 转换失败，可能是因为字符串中包含非数字字符
    //     qDebug() << "Conversion failed for wendu"<< objData.value("wendu").toString();
    // }
    // qDebug()<<mToday.wendu;

    mToday.shidu= objData.value("shidu").toString();
    mToday.pm25= objData.value("pm25").toInt();
    mToday.quality= objData.value("quality").toString();
    //5.forecast中第一个数组元素也是今天的数据
    mToday.type=mDay[1].type;
    mToday.fx=mDay[1].fx;
    mToday.fl=mDay[1].fl;
    mToday.high=mDay[1].high;
    mToday.low=mDay[1].low;
    //6.1更新UI界面
    updateUI();
    //6.2绘制温度曲线
    ui->lblHighCurve->update();
    ui->lblLowCurve->update();//刷新QLabel标签，切换城市时刷新温度曲线
}

void MainWindow::updateUI()
{//更新UI界面
    //右上角日期
    ui->lblDate->setText(QDateTime::fromString(mToday.date,"yyyyMMdd").toString("yyyy/MM/dd")+" "+mDay[1].week);
    //更新城市
    ui->lblCity->setText(mToday.city);

    //更新今天的数据
    ui->lblTempIcon->setPixmap(mTypeMap[mToday.type]);
    ui->lblTemp->setText(mToday.wendu+"°");
    ui->lblType->setText(mToday.type);
    ui->lblLowHigh->setText(QString::number(mToday.low)+"°~"+QString::number(mToday.high)+"°C");
    ui->lblGanMao->setText("感冒指数:"+mToday.ganmao);
    ui->lblFx->setText(mToday.fx);
    ui->lblFl->setText(mToday.fl);
    ui->lblPMValue->setText(QString::number(mToday.pm25));
    ui->lblHumidityValue->setText(mToday.shidu);
    ui->lblAPIValue->setText(mToday.quality);

    //更新6天的数据
    for(int i=0; i<6; i++)
    {
        mWeekList[i]->setText("周"+mDay[i].week.right(1));
        ui->lblWeek0->setText("昨天");
        ui->lblWeek1->setText("今天");
        ui->lblWeek2->setText("明天");
        mDateList[i]->setText(QDateTime::fromString(mDay[i].date,"yyyy-MM-dd").toString("MM/dd"));
        mTypeList[i]->setText(mDay[i].type);
        if(mDay[i].api >=0 && mDay[i].api<= 50)
        {
            mAqiList[i]->setText("优");
            mAqiList[i]->setStyleSheet("background-color:rgb(0, 176, 0)");
        }
        else if(mDay[i].api >50 && mDay[i].api<= 100)
        {
            mAqiList[i]->setText("良");
            mAqiList[i]->setStyleSheet("background-color:rgb(255, 170, 0)");
        }
        else if(mDay[i].api >100 && mDay[i].api<= 150)
        {
            mAqiList[i]->setText("轻度");
            mAqiList[i]->setStyleSheet("background-color:rgb(197, 66, 99)");
        }
        else if(mDay[i].api >150 && mDay[i].api<= 200)
        {
            mAqiList[i]->setText("中度");
            mAqiList[i]->setStyleSheet("background-color:rgb(184, 0, 0)");
        }
        else if(mDay[i].api >200 && mDay[i].api<= 250)
        {
            mAqiList[i]->setText("重度");
            mAqiList[i]->setStyleSheet("background-color:rgb(170, 0, 0)");
        }
        else
        {
            mAqiList[i]->setText("严重");
            mAqiList[i]->setStyleSheet("background-color:rgb(85, 0, 0)");
        }
        mFxList[i]->setText(mDay[i].fx);
        mFlList[i]->setText(mDay[i].fl);

        mTypeIconList[i]->setPixmap(mTypeMap[mDay[i].type]);
    }
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{//重写菜单虚函数
    mExitMenu->exec(QCursor::pos());//在右键按下的地方弹出
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    mOffest = event->globalPos()-this->pos();
    event->accept();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()- mOffest);
    event->accept();
}

void MainWindow::on_btnSearch_clicked()
{
    QString cityName= ui->leCity->text().trimmed();
    getWeatherInfo(cityName);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{//父窗口事件过滤器
    if(watched == ui->lblHighCurve && event->type() == QEvent::Paint)
    {//如果事件对象是高温曲线
        paintHighCurve();
    }
    if(watched == ui->lblLowCurve && event->type() == QEvent::Paint)
    {//如果事件对象是高温曲线
        paintLowCurve();
    }
    return QWidget::eventFilter(watched,event);
}

void MainWindow::paintHighCurve()
{//绘制高温曲线
    QPainter painter(ui->lblHighCurve);

    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);
    //1.获取x坐标(每个QLabel坐标的中心位置)
    int pointX[6]={0};//先初始化为0
    for(int i=0; i<6; i++){
        pointX[i]= (mAqiList[i]->pos().x() + mAqiList[i]->width()/2)-10;
    }
    //2.获取y坐标(思路:求6天的平均值，温度高于平均值1°，该点就往上移3个像素，
                    //低于平均值1°，该点就往下移3个像素)
    int tempSum = 0;//6天温度的总和
    int tempAverage = 0;//6天的平均温度
    for(int i=0; i<6; i++)
    {
        tempSum += mDay[i].high;
    }
    tempAverage= tempSum/6;//高温的平均值
    //计算y轴坐标
    int pointY[6]= {0};
    int yCenter= ui->lblHighCurve->height()/2;
    for(int i=0; i<6; i++)
    {
        pointY[i]= yCenter - ((mDay[i].high - tempAverage)* INCREMENT);//(y轴正方向往下，所以是-)
    }
    //3.开始绘制
    //3.1初始化画笔和画刷
    QPen pen= painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(255,170,0));
    painter.setPen(pen);
    painter.setBrush(QColor(255,170,0));
    //3.2画点，写文本
    for( int i=0; i<6; i++)
    {
        //显示点
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);
        //显示温度文本
        painter.drawText(QPoint(pointX[i],pointY[i]-20),QString::asprintf("%d°C",mDay[i].high));
    }
    //3.3绘制曲线
    for(int i=0; i<5; i++)
    {
        if(i == 0)
        {
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
            painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
        }
        else
        {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
        }

    }
}

void MainWindow::paintLowCurve()
{//绘制低温曲线
    QPainter painter(ui->lblLowCurve);

    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);
    //1.获取x坐标(每个QLabel坐标的中心位置)
    int pointX[6]={0};//先初始化为0
    for(int i=0; i<6; i++){
        pointX[i]= (mAqiList[i]->pos().x() + mAqiList[i]->width()/2)-10;
    }
    //2.获取y坐标(思路:求6天的平均值，温度高于平均值1°，该点就往上移3个像素，
    //低于平均值1°，该点就往下移3个像素)
    int tempSum = 0;//6天温度的总和
    int tempAverage = 0;//6天的平均温度
    for(int i=0; i<6; i++)
    {
        tempSum += mDay[i].low;
    }
    tempAverage= tempSum/6;//高温的平均值
    //计算y轴坐标
    int pointY[6]= {0};
    int yCenter= ui->lblLowCurve->height()/2;
    for(int i=0; i<6; i++)
    {
        pointY[i]= yCenter - ((mDay[i].low - tempAverage)* INCREMENT);//(y轴正方向往下，所以是-)
    }
    //3.开始绘制
    //3.1初始化画笔和画刷
    QPen pen= painter.pen();
    pen.setWidth(1);
    pen.setColor(QColor(0,255,255));
    painter.setPen(pen);
    painter.setBrush(QColor(0,255,255));
    //3.2画点，写文本
    for( int i=0; i<6; i++)
    {
        //显示点
        painter.drawEllipse(QPoint(pointX[i],pointY[i]),POINT_RADIUS,POINT_RADIUS);
        //显示温度文本
        painter.drawText(QPoint(pointX[i],pointY[i]-20),QString::asprintf("%d°C",mDay[i].low));
    }
    //3.3绘制曲线
    for(int i=0; i<5; i++)
    {
        if(i == 0)
        {
            pen.setStyle(Qt::DotLine);
            painter.setPen(pen);
            painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
        }
        else
        {
            pen.setStyle(Qt::SolidLine);
            painter.setPen(pen);
            painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
        }

    }
}

