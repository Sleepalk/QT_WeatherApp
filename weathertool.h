#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H
#include<QString>
#include<QMap>
#include<QFile>
#include<QJsonDocument>
#include<QJsonArray>
#include<QJsonObject>
#include<QJsonValue>
#include<QJsonParseError>
#include<QMessageBox>
//天气工具类,获取城市编码
class WeatherTool{
private:
    static QMap<QString,QString>mCityMap;
    static void initCityMap(){//初始化城市Map
        //1.读取文件
        QFile file("D:/Qtitems/WeatherApp/weather_citylist.json");
        if(!file.open(QFile::ReadOnly | QFile::Text))
            qDebug()<<"文件没有打开";
        QByteArray json=file.readAll();
        //qDebug()<<"json数据:"<<json;
        file.close();
        //2.解析并写入到Map
        QJsonParseError err;
        QJsonDocument doc= QJsonDocument::fromJson(json,&err);
        if(err.error != QJsonParseError::NoError)
            return;
        if(!doc.isArray()){
            return;
        }
        QJsonArray cites= doc.array();
        for(int i=0; i<cites.size(); i++)
        {
            QString cityName= cites[i].toObject().value("cityName").toString();
            QString cityCode= cites[i].toObject().value("cityCode").toString();
            if(cityCode.size() > 0)
            {mCityMap.insert(cityName,cityCode);}
        }
    }
public:
    static QString getCityCode(QString cityName){
        if(mCityMap.isEmpty())
        {//如果mcitymap为空，初始化城市map
            initCityMap();
        }
        QMap<QString,QString>::iterator it= mCityMap.find(cityName);
        if(it == mCityMap.end()){
            it = mCityMap.find(cityName+"市");
        }
        if(it != mCityMap.end())
            return it.value();
        return "";

    }
};
QMap<QString,QString> WeatherTool::mCityMap={};//类成员，初始化为空
#endif // WEATHERTOOL_H
