#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "weatherdata.h"
#include <QMainWindow>
#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<QLabel>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void onReplied(QNetworkReply* reply);
    void getWeatherInfo(QString cityName);//获取天气信息
    void parseJson(QByteArray& byteArray);//解析JSON
    void updateUI();//更新ui
private:
    Ui::MainWindow *ui;
    QMenu *mExitMenu;//右键退出的菜单
    QAction *mExitAct;//退出的行为，菜单项
    QPoint mOffest;//窗口移动时，鼠标与窗口左上角的偏移
    QNetworkAccessManager *mNetAccessManager;//网络请求管理器
    ToDay mToday;
    Day mDay[6];
    QList<QLabel*>mWeekList;//星期几
    QList<QLabel*>mDateList;//日期
    QList<QLabel*>mTypeList;//天气类型
    QList<QLabel*>mTypeIconList;//天气类型图标
    QList<QLabel*>mAqiList;//天气质量
    QList<QLabel*>mFxList;//风向
    QList<QLabel*>mFlList;//风力

    QMap<QString, QString> mTypeMap;
    // QWidget interface
protected:
    void contextMenuEvent(QContextMenuEvent *event);//重写菜单虚函数

    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
private slots:
    void on_btnSearch_clicked();

    // QObject interface
public:
    //重写父类的eventFileter方法
    bool eventFilter(QObject *watched, QEvent *event);

    //绘制高低温曲线
    void paintHighCurve();
    void paintLowCurve();
};
#endif // MAINWINDOW_H
