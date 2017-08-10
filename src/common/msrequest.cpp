/*
    CloudCross: Opensource program for syncronization of local files and folders with clouds

    Copyright (C) 2016-2017  Vladimir Kamensky
    Copyright (C) 2016-2017  Master Soft LLC.
    All rights reserved.


  BSD License

  Redistribution and use in source and binary forms, with or without modification, are
  permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, this list of
    conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice, this list
    of conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.
  - Neither the name of the "Vladimir Kamensky" or "Master Soft LLC." nor the names of
    its contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY E
  XPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES O
  F MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SH
  ALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENT
  AL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROC
  UREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS I
  NTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRI
  CT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF T
  HE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "include/msrequest.h"
#include <QDebug>




MSRequest::MSRequest(QNetworkProxy *proxy)
{
    // init members on create object


    this->url=new QUrl();
    this->manager=new QNetworkAccessManager();
    this->query=new QUrlQuery();

    //this->cookieJar=0;

    this->lastReply=0;
    this->outFile=0;

    this->requesProcessed = false;

    this->replyError= QNetworkReply::NetworkError::NoError;

    this->loop=new QEventLoop(this);


    connect((QObject*)this->manager,SIGNAL(finished(QNetworkReply*)),(QObject*)this,SLOT(requestFinished(QNetworkReply*)));

    if(proxy != 0){
        this->setProxy(proxy);
    }
}


MSRequest::~MSRequest(){

    delete(this->loop);
    delete(this->manager);

    //QNetworkRequest::~QNetworkRequest();
}


void MSRequest::setRequestUrl(const QString &url){

    this->url->setUrl( url);

}


void MSRequest::addQueryItem(const QString &itemName, const QString &itemValue){

        this->query->addQueryItem(itemName,itemValue);

}


bool MSRequest::setMethod(const QString &method){
    if((method=="post")||(method=="get")||(method=="put")){
        this->requestMethod=method;
        return true;
    }
    else{
        return false;
    }
}


void MSRequest::addHeader(const QString &headerName, const QString &headerValue){

    //this->setRawHeader(QByteArray::fromStdString(headerName.toStdString())  ,QByteArray::fromStdString(headerValue.toStdString()));
    this->setRawHeader(QByteArray(headerName.toStdString().c_str()),QByteArray(headerValue.toStdString().c_str()));

}


void MSRequest::addHeader(const QByteArray &headerName, const QByteArray &headerValue){

    this->setRawHeader(headerName,headerValue);

}


void MSRequest::methodCharger(QNetworkRequest req){

#ifdef CCROSS_LIB

    QtCUrl cUrl;
    //cUrl.setTextCodec("cp-1251");

    QStringList headers;

    QtCUrl::Options opt;
    opt[CURLOPT_URL] = toUrlEncoded(req.url().url());//QUrl("https://mastersoft24.ru/img/applications.png");// //(QUrl) *(this->url); //this->query->toString();;


    if(this->requestMethod == "get"){

        opt[CURLOPT_URL] = (req.url().url()); // this is correct for  GD

        //opt[CURLOPT_HTTPGET] = 1;
        //opt[CURLOPT_URL] = toUrlEncoded(req.url().toString() + this->query->toString() );
        //opt[CURLOPT_URL] = toUrlEncoded(req.url().toString()  );
        // possibly may be needed add query params to url string
    }

    if(this->requestMethod == "post"){

        opt[CURLOPT_POST] = true;

        QString qs = "";

        if(this->query->queryItems().size() > 0){

//            for(int i=0; i < this->query->queryItems().size();  i++){


//                QPair<QString,QString> p = this->query->queryItems().at(i);
//                qs += p.first+"="+p.second+"&";

//            }

//            qs = qs.left(qs.length()-1);

            QString erere = QUrl::toPercentEncoding(this->query->toString() ,QByteArray("/,,/,\\,?,:,@,&,=,+,$,#,-,_,.,!,~,*,',(,)")) ;;// qs ;

            opt[CURLOPT_POSTFIELDS] = QUrl::toPercentEncoding(this->query->toString() ,QByteArray("/,,/,\\,?,:,@,&,=,+,$,#,-,_,.,!,~,*,',(,)")) ;;// qs ;
            //opt[CURLOPT_POSTFIELDSIZE] = this->query->toString().size();// this->toUrlEncoded(this->query->toString()).size();

           // headers << "Content-Length:    "+QString::number(this->query->toString().size());

            //qDebug() << qs;

        }
        else{
            opt[CURLOPT_POSTFIELDS] = "";
            opt[CURLOPT_POSTFIELDSIZE] = 0;
        }

        if(!req.hasRawHeader("Content-Type")){

            if(!this->notUseContentType){
                //headers << "Content-Type:    application/x-www-form-urlencoded";
            }
        }

    }

    opt[CURLOPT_FOLLOWLOCATION] = 1;
    opt[CURLOPT_FAILONERROR] = 1;

    if(req.rawHeaderList().size() > 0){


        for(int i = 0; i < req.rawHeaderList().size(); i++){


            QString ss= QString(req.rawHeaderList().at(i)) + QString(":  ") + req.rawHeader(req.rawHeaderList().at(i));
            headers << ss;
        }

    }

    opt[CURLOPT_HTTPHEADER] = headers;

//    opt[CURLOPT_SSL_VERIFYPEER] = 0;
//    opt[CURLOPT_SSL_VERIFYHOST] = 0;


    cUrl.exec(opt);

    if (cUrl.lastError().isOk()) {

        this->replyError = QNetworkReply::NetworkError::NoError;
        this->replyText = QByteArray(cUrl.buffer(), cUrl.buffer().size());

    }
    else {
        this->replyError = QNetworkReply::NetworkError::ContentGoneError;
        qDebug() << (QString("MSRequest - CHARDGER Error: %1\nBuffer: %2").arg(cUrl.lastError().text()).arg(cUrl.errorBuffer()));
    }


    return;

#endif


#ifndef CCROSS_LIB
    QNetworkReply* replySync=0;

    if(this->requestMethod=="get"){

        replySync=this->manager->get(req);
    }


    if(this->requestMethod=="post"){

        if(!req.hasRawHeader("Content-Type")){

            if(!this->notUseContentType){
                req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
            }
        }



        QByteArray ba;
        ba+=this->query->toString();
        replySync=this->manager->post(req,ba);
    }

#ifdef PRINT_DEBUG_INFO
    this->printDebugInfo_request(req);
#endif

//    connect(replySync,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(doDownloadProgress(qint64,qint64)));
//    connect(replySync,SIGNAL(readyRead()),this,SLOT(doReadyRead()));


//    QEventLoop loop;
    connect(replySync, SIGNAL(finished()),this->loop, SLOT(quit()));
    this->loop->exec();

    delete(replySync);
    //this->loop->exit(0);
    //loop.deleteLater();

#endif
}


void MSRequest::printDebugInfo_request(const QNetworkRequest &req){

    qDebug()<<"";
    qDebug()<<"=========== Begin Query Debug Info Block =============";

    qDebug()<<"Request URL: "<< this->url->toString();
    qDebug()<<"Request Method: "<< this->requestMethod;
    qDebug()<<"Request Headers: ";

    QList<QByteArray> hl=req.rawHeaderList();

    for(int i=0; i< hl.size();i++){
        qDebug()<<hl.at(i)<<": "<<req.rawHeader(hl.at(i));

    }



    qDebug()<<"";
    qDebug()<<"Query parameters: ";
    QList<QPair<QString, QString> >  qi=this->query->queryItems();

    for(int i = 0;i<qi.size();i++){

        qDebug()<<qi.at(i).first<<": "<<qi.at(i).second;
    }

    qDebug()<<"=========== End Query Debug Info Block =============";
    qDebug()<<"";
}



void MSRequest::printDebugInfo_response(QNetworkReply *reply){

    qDebug()<<"";
    qDebug()<<"=========== Begin Response Debug Info Block =============";


    qDebug()<<"Response Headers: ";

    QList<QByteArray> hl=reply->rawHeaderList();

    for(int i=0; i< hl.size();i++){
        qDebug()<<hl.at(i)<<": "<<reply->rawHeader(hl.at(i));

    }

    qDebug()<<"";

    qDebug()<<"Response Body: ";
    qDebug()<<this->replyText;

    qDebug()<<"=========== End Response Debug Info Block =============";
    qDebug()<<"";
}


#ifdef CCROSS_LIB

QString MSRequest::toUrlEncoded(QString p){


    return QUrl::toPercentEncoding(p ,QByteArray("/,,/,\\,?,:,@,&,=,+,$,#,-,_,.,!,~,*,',(,)")) ;;// qs ;
}

#endif



void MSRequest::methodCharger(QNetworkRequest req, const QString &path){

    // fix warning message

    Q_UNUSED(path);

#ifdef CCROSS_LIB

    QtCUrl cUrl;
    //cUrl.setTextCodec("cp-1251");

    QUrl url("https://ya.ru");
    //url.addQueryItem("id", "42");

    QStringList headers;

    QtCUrl::Options opt;
    opt[CURLOPT_URL] = this->toUrlEncoded(req.url().url());//QUrl("https://mastersoft24.ru/img/applications.png");// //(QUrl) *(this->url); //this->query->toString();;


    if(this->requestMethod == "get"){

        opt[CURLOPT_URL] = (req.url().url()); // this is correct for  GD

//        opt[CURLOPT_HTTPGET] = true;
//        opt[CURLOPT_URL] = toUrlEncoded(req.url().toString());
        // possibly may be needed add query params to url string
    }

    if(this->requestMethod == "post"){
        opt[CURLOPT_POST] = 1;

        QString qs = "";

        if(this->query->queryItems().size() > 0){

//            for(int i=0; i < this->query->queryItems().size();  i++){


//                QPair<QString,QString> p = this->query->queryItems().at(i);
//                qs += p.first+"="+p.second+"&";

//            }

//            qs = qs.left(qs.length()-1);

            opt[CURLOPT_POSTFIELDS] = QUrl::toPercentEncoding(this->query->toString() ,QByteArray("/,,/,\\,?,:,@,&,=,+,$,#,-,_,.,!,~,*,',(,)")) ;;// qs ;
            //opt[CURLOPT_POSTFIELDSIZE] = this->toUrlEncoded(this->query->toString()).size();
            //qDebug() << qs;

        }
        else{
            opt[CURLOPT_POSTFIELDS] = "";
            opt[CURLOPT_POSTFIELDSIZE] = 0;
        }

        if(!req.hasRawHeader("Content-Type")){

            if(!this->notUseContentType){
                headers << "Content-Type:    application/x-www-form-urlencoded";
            }
        }
    }

    opt[CURLOPT_FOLLOWLOCATION] = 1;
    opt[CURLOPT_FAILONERROR] = true;

    if(req.rawHeaderList().size() > 0){

        for(int i = 0; i < req.rawHeaderList().size(); i++){


            QString ss= QString(req.rawHeaderList().at(i)) + QString(":  ") + req.rawHeader(req.rawHeaderList().at(i));
            headers << ss;
        }

    }

    opt[CURLOPT_HTTPHEADER] = headers;

//    opt[CURLOPT_SSL_VERIFYPEER] = 0;
//    opt[CURLOPT_SSL_VERIFYHOST] = 0;


    cUrl.exec(opt);

//    log(this->query->toString());


//    log(QString::number(cUrl.buffer().size()));

    if (cUrl.lastError().isOk()) {

        this->replyError = QNetworkReply::NetworkError::NoError;
        this->outFile->write(cUrl.buffer(), cUrl.buffer().size());
        this->outFile->close();
    }
    else {
        this->replyError = QNetworkReply::NetworkError::ContentGoneError;
        qDebug() << (QString("Error: %1\nBuffer: %2").arg(cUrl.lastError().text()).arg(cUrl.errorBuffer()));
    }


    return;

#endif

#ifndef CCROSS_LIB

    QNetworkReply* replySync=0;

    if(this->requestMethod=="get"){
        replySync=this->manager->get(req);
    }


    if(this->requestMethod=="post"){
        req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

        QByteArray ba;
        ba+=this->query->toString();
        replySync=this->manager->post(req,ba);
    }

    this->currentReply=replySync;

//    connect(replySync,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(doDownloadProgress(qint64,qint64)));
//    connect(replySync,SIGNAL(readyRead()),this,SLOT(doReadyRead()));
//    connect(replySync,SIGNAL(finished()),this,SLOT(doRequestFinished()));

    //this->requesProcessed =true;
    //QEventLoop loop;
    connect(replySync, SIGNAL(finished()),this->loop, SLOT(quit()));
    this->loop->exec();

    delete(replySync);
    //this->loop->exit(0);
    //loop.deleteLater();

#endif
}



void MSRequest::raw_exec(const QString &reqestURL){

#ifdef CCROSS_LIB

    QtCUrl cUrl;
    QtCUrl::Options opt;

    opt[CURLOPT_URL] = toUrlEncoded(reqestURL);
    opt[CURLOPT_FOLLOWLOCATION] = 1;
    opt[CURLOPT_FAILONERROR] = 1;

    cUrl.exec(opt);

    if (cUrl.lastError().isOk()) {

        this->replyError = QNetworkReply::NetworkError::NoError;
        this->replyText = QByteArray(cUrl.buffer(), cUrl.buffer().size());

    }
    else {
        this->replyError = QNetworkReply::NetworkError::ContentGoneError;
        qDebug() << (QString("Error: %1\nBuffer: %2").arg(cUrl.lastError().text()).arg(cUrl.errorBuffer()));
    }
#endif

#ifndef CCROSS_LIB
    QUrl r(reqestURL);

    QString s=QUrl::toPercentEncoding(r.path().toUtf8(),"(){}/","");

    QString url=r.scheme()+"://"+r.host()+""+ s;
    this->setRequestUrl(url);

    this->query->setQuery(r.query());
    this->setMethod("get");
    this->exec();
#endif
}



void MSRequest::post(const QByteArray &data){

#ifdef CCROSS_LIB

    QtCUrl cUrl;
    cUrl.setTextCodec("utf-8");
    QStringList headers;
    QtCUrl::Options opt;

    opt[CURLOPT_POST] = 1;

    opt[CURLOPT_URL] = (this->url->url());

    if(this->query->queryItems().size() > 0){

        opt[CURLOPT_POSTFIELDS] = this->toUrlEncoded(this->query->toString());

    }

    if(!this->hasRawHeader("Content-Type")){

        if(!this->notUseContentType){
            headers << "Content-Type:    application/x-www-form-urlencoded";
        }
    }

    if(this->rawHeaderList().size() > 0){


        for(int i = 0; i < this->rawHeaderList().size(); i++){


            QString ss= QString(this->rawHeaderList().at(i)) + QString(":  ") + this->rawHeader(this->rawHeaderList().at(i));
            headers << ss;//this->toUrlEncoded(ss);
        }

    }

    opt[CURLOPT_HTTPHEADER] = headers;
    opt[CURLOPT_FOLLOWLOCATION] = 1;
    opt[CURLOPT_FAILONERROR] = 1;

    opt[CURLOPT_POSTFIELDS] = (data);
    opt[CURLOPT_POSTFIELDSIZE] = (data).size();

    cUrl.exec(opt);

    if (cUrl.lastError().isOk()) {

        this->replyError = QNetworkReply::NetworkError::NoError;
        this->replyText = QByteArray(cUrl.buffer(), cUrl.buffer().size());

    }
    else {
        this->replyError = QNetworkReply::NetworkError::ContentGoneError;
        qDebug() << (QString("MSRequest - POST Error: %1\nBuffer: %2").arg(cUrl.lastError().text()).arg(cUrl.errorBuffer()));
    }

#endif

#ifndef CCROSS_LIB
    QNetworkReply* replySync;

    this->url->setQuery(*this->query);
    this->setUrl(*this->url);

    replySync=this->manager->post(*this,data);

    //QEventLoop loop;
    connect(replySync, SIGNAL(finished()),this->loop, SLOT(quit()));
    loop->exec();

    delete(replySync);
    //this->loop->exit(0);
    //this->loop.deleteLater();
#endif
}


void MSRequest::download(const QString &url){

    //this->setUrl(*this->url);
    this->setUrl(QUrl(url));


    methodCharger(*this);

    // 301 redirect handling
    while(true){

        QVariant possible_redirected_url= this->replyAttribute;//this->lastReply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        if(!possible_redirected_url.isNull()) {

            QUrl rdrUrl=possible_redirected_url.value<QUrl>();

           // QString reqStr=rdrUrl.scheme()+"://"+rdrUrl.host()+"/"+rdrUrl.path()+"?"+rdrUrl.query();
            QString reqStr=rdrUrl.scheme()+"://"+rdrUrl.host()+""+rdrUrl.path()+"?"+rdrUrl.query();

            methodCharger(QNetworkRequest(reqStr));

        }
        else{
            break;
        }
    }
}



void MSRequest::download(const QString &url, const QString &path){

    this->setUrl(QUrl(url));

    this->outFile= new QFile(path);
    bool e=this->outFile->open(QIODevice::WriteOnly );

    if(e == false){
        this->replyError = QNetworkReply::NetworkError::UnknownContentError;
        return;
    }




    methodCharger(*this,path);

#ifndef CCROSS_LIB

    // 301 redirect handling
    while(true){

        QVariant possible_redirected_url= this->replyAttribute;//this->lastReply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        if(!possible_redirected_url.isNull()) {

            QUrl rdrUrl=possible_redirected_url.value<QUrl>();

           // QString reqStr=rdrUrl.scheme()+"://"+rdrUrl.host()+"/"+rdrUrl.path()+"?"+rdrUrl.query();
            QString reqStr=rdrUrl.scheme()+"://"+rdrUrl.host()+""+rdrUrl.path()+"?"+rdrUrl.query();

            methodCharger(QNetworkRequest(reqStr),path);

        }
        else{
            break;
        }
    }
#endif

}


void MSRequest::put(const QByteArray &data){
#ifdef CCROSS_LIB

    QtCUrl cUrl;
    QStringList headers;
    QtCUrl::Options opt;

    QString sss = toUrlEncoded(this->url->url());

//    if(this->query->queryItems().size() > 0){

        opt[CURLOPT_URL] = this->url->toString() + "?" +(this->query->toString());
//    }
//    else{
//        opt[CURLOPT_URL] = toUrlEncoded(this->url->url());
//    }

    opt[CURLOPT_CUSTOMREQUEST] = "PUT";
//    opt[CURLOPT_UPLOAD] = 1;
    //opt[CURLOPT_INFILESIZE_LARGE] = data.size();

    opt[CURLOPT_POSTFIELDS] = (data);
    opt[CURLOPT_POSTFIELDSIZE] = (data).size();


//    if(this->query->queryItems().size() > 0){

//        opt[CURLOPT_POSTFIELDS] = this->toUrlEncoded(this->query->toString());

//    }

    if(!this->hasRawHeader("Content-Type")){

        if(!this->notUseContentType){
            //headers << "Content-Type:    application/x-www-form-urlencoded";
        }
    }

    if(this->rawHeaderList().size() > 0){


        for(int i = 0; i < this->rawHeaderList().size(); i++){


            QString ss= QString(this->rawHeaderList().at(i)) + QString(":  ") + this->rawHeader(this->rawHeaderList().at(i));
            headers << ss;
        }

    }

    opt[CURLOPT_HTTPHEADER] = headers;


    cUrl.exec(opt);

    if (cUrl.lastError().isOk()) {

        this->replyError = QNetworkReply::NetworkError::NoError;
        this->replyText = QByteArray(cUrl.buffer(), cUrl.buffer().size());

    }
    else {
        this->replyError = QNetworkReply::NetworkError::ContentGoneError;
        qDebug() << (QString("MSRequest - PUT Error: %1\nBuffer: %2").arg(cUrl.lastError().text()).arg(cUrl.errorBuffer()));
        qDebug()<< cUrl.buffer();
    }

#endif

#ifndef CCROSS_LIB
    QNetworkReply* replySync;

    this->url->setQuery(*this->query);
    this->setUrl(*this->url);

    replySync=this->manager->put(*this,data);

    //QEventLoop loop;
    connect(replySync, SIGNAL(finished()),this->loop, SLOT(quit()));
    this->loop->exec();

    delete(replySync);
    //this->loop->exit(0);
    //loop.deleteLater();

#endif
}

#ifdef CCROSS_LB
size_t MSRequest::readCallback(void *ptr, size_t size, size_t nmemb, void *stream){
//  size_t retcode;
//  curl_off_t nread;

//  /* in real-world cases, this would probably get this data differently
//     as this fread() stuff is exactly what the library already would do
//     by default internally */
//  retcode = fread(ptr, size, nmemb, stream);

//  nread = (curl_off_t)retcode;


//  return retcode;
}
#endif

void MSRequest::put(QIODevice *data){

#ifdef CCROSS_LIB

    QtCUrl cUrl;
    QStringList headers;
    QtCUrl::Options opt;

    opt[CURLOPT_URL] = (this->url->url());
    opt[CURLOPT_READFUNCTION] = readCallback;
    opt[CURLOPT_UPLOAD] = 1;
    opt[CURLOPT_PUT] = 1;
    opt[CURLOPT_INFILESIZE_LARGE] = data->size();
    opt[CURLOPT_READDATA] = QVariant::fromValue(data);

//    opt[CURLOPT_POSTFIELDS] = data;
//    opt[CURLOPT_POSTFIELDSIZE] = data.size();

    if(!this->hasRawHeader("Content-Type")){

        if(!this->notUseContentType){
            headers << "Content-Type:    application/x-www-form-urlencoded";
        }
    }

    if(this->rawHeaderList().size() > 0){


        for(int i = 0; i < this->rawHeaderList().size(); i++){


            QString ss= QString(this->rawHeaderList().at(i)) + QString(":  ") + this->rawHeader(this->rawHeaderList().at(i));
            headers << ss;
        }

    }

    opt[CURLOPT_HTTPHEADER] = headers;


    cUrl.exec(opt);

    if (cUrl.lastError().isOk()) {

        this->replyError = QNetworkReply::NetworkError::NoError;
        this->replyText = QByteArray(cUrl.buffer(), cUrl.buffer().size());

    }
    else {
        this->replyError = QNetworkReply::NetworkError::ContentGoneError;
        qDebug() << (QString("Error: %1\nBuffer: %2").arg(cUrl.lastError().text()).arg(cUrl.errorBuffer()));
    }

#endif

#ifndef CCROSS_LIB
    QNetworkReply* replySync;

    this->url->setQuery(*this->query);
    this->setUrl(*this->url);

    replySync=this->manager->put(*this,data);

    //QEventLoop loop;
    connect(replySync, SIGNAL(finished()),this->loop, SLOT(quit()));
    this->loop->exec();

    delete(replySync);
    //this->loop->exit(0);
    //loop.deleteLater();
#endif
}



void MSRequest::deleteResource(){

#ifdef CCROSS_LIB

    QtCUrl cUrl;
    QStringList headers;
    QtCUrl::Options opt;

    opt[CURLOPT_URL] = toUrlEncoded(this->url->url());

    opt[CURLOPT_CUSTOMREQUEST] = "DELETE";
    //opt[CURLOPT_UPLOAD] = 1;
    //opt[CURLOPT_INFILESIZE_LARGE] = data.size();

    opt[CURLOPT_POSTFIELDS] = this->toUrlEncoded(this->query->toString());
    opt[CURLOPT_POSTFIELDSIZE] = this->toUrlEncoded(this->query->toString()).size();

    if(!this->hasRawHeader("Content-Type")){

        if(!this->notUseContentType){
            headers << "Content-Type:    application/x-www-form-urlencoded";
        }
    }

    if(this->rawHeaderList().size() > 0){


        for(int i = 0; i < this->rawHeaderList().size(); i++){


            QString ss= QString(this->rawHeaderList().at(i)) + QString(":  ") + this->rawHeader(this->rawHeaderList().at(i));
            headers << ss;
        }

    }

    opt[CURLOPT_HTTPHEADER] = headers;


    cUrl.exec(opt);

    if (cUrl.lastError().isOk()) {

        this->replyError = QNetworkReply::NetworkError::NoError;
        this->replyText = QByteArray(cUrl.buffer(), cUrl.buffer().size());

    }
    else {
        this->replyError = QNetworkReply::NetworkError::ContentGoneError;
        qDebug() << (QString("Error: %1\nBuffer: %2").arg(cUrl.lastError().text()).arg(cUrl.errorBuffer()));
    }


#endif

#ifndef CCROSS_LIB
    QNetworkReply* replySync;

    this->url->setQuery(*this->query);
    this->setUrl(*this->url);

    replySync=this->manager->deleteResource(*this);

    //QEventLoop loop;
    connect(replySync, SIGNAL(finished()),this->loop, SLOT(quit()));
    this->loop->exec();

    delete(replySync);
    //this->loop->exit(0);
    //loop.deleteLater();

#endif
}




void MSRequest::exec(){

    this->url->setQuery(*this->query);
    this->setUrl(*this->url);

    methodCharger(*this);

    // 301 redirect handling
    while(true){

        QVariant possible_redirected_url= this->replyAttribute;//this->lastReply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        if(!possible_redirected_url.isNull()) {

            QUrl rdrUrl=possible_redirected_url.value<QUrl>();

           // QString reqStr=rdrUrl.scheme()+"://"+rdrUrl.host()+"/"+rdrUrl.path()+"?"+rdrUrl.query();
            QString reqStr=rdrUrl.scheme()+"://"+rdrUrl.host()+""+rdrUrl.path()+"?"+rdrUrl.query();

            methodCharger(QNetworkRequest(reqStr));

        }
        else{
            break;
        }
    }

}


void MSRequest::requestFinished(QNetworkReply *reply){


    //log("REQUEST FINISHED SLOT ");

    if(this->outFile!=0){

        this->outFile->write(this->currentReply->readAll());
        this->outFile->close();
        delete(this->outFile);
        this->outFile=0;
    }

    this->lastReply=reply;
    this->replyHeaders=reply->rawHeaderPairs();
    this->replyText=reply->readAll();
    this->replyError=reply->error();
    this->replyErrorText=reply->errorString();
    this->replyAttribute=reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    this->replyURL=reply->url().toString();

#ifdef PRINT_DEBUG_INFO
    this->printDebugInfo_response(reply);
#endif

//    delete(reply);

}



void MSRequest::doDownloadProgress(qint64 avail, qint64 total){

   // fix warning message
    if(avail==total){
        return;
    }

    log("doDownloadProgress was entered");

}


void MSRequest::doReadyRead(){

    this->outFile->write(this->currentReply->readAll());

    //this->loop->exit(0);

    log("doReadyRead was entered");
}




void MSRequest::doRequestFinished(){

//    QProcess process;
//    process.start("echo -e \"\a\" ");

    this->requesProcessed = false;
    this->loop->quit();
    log("Request finished");
}



QByteArray MSRequest::readReplyText(){
    return this->replyText;
}


bool MSRequest::replyOK(){

    if(this->replyError == QNetworkReply::NetworkError::NoError){
        return true;
    }
    else{
        return false;
    }
}


void MSRequest::printReplyError(){

    qStdOut() << this->replyErrorText << endl;
}



void MSRequest::MSsetCookieJar(QNetworkCookieJar* cookie)
{

//    if(*cookie == 0){
//        *cookie=new QNetworkCookieJar();
//        this->cookieJar=*cookie;
//    }
//    else{
//        this->cookieJar=*cookie;
//    }


    this->cookieJar=cookie;
    this->manager->setCookieJar(cookie);//this->cookieJar

}



QJsonObject MSRequest::cookieToJSON()
{
    QJsonObject c;
    //QList<QNetworkCookie> cl=this->manager->cookieJar()->cookiesForUrl(*this->url);
    QList<QNetworkCookie> cl=this->manager->cookieJar()->cookiesForUrl( QUrl(this->replyURL));


    for(int i=0;i < cl.size();i++){

        c.insert(cl[i].name(),QJsonValue::fromVariant(QVariant(cl[i].value())));
    }

    return c;
}



bool MSRequest::cookieFromJSON(const QJsonObject &cookie)
{
    //cookie=QJsonObject();
    Q_UNUSED(cookie);
return true;
}


void MSRequest::setProxy(QNetworkProxy *proxy){

    this->manager->setProxy(*proxy);

}


void MSRequest::log(QString mes){

    FILE* lf=fopen("/tmp/ccfw.log","a+");
    if(lf != NULL){

        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];

        time (&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer,sizeof(buffer),"%T - ",timeinfo);

        mes = QString(buffer)+"MSRequest : "+mes+" \n";
        fputs(mes.toStdString().c_str(),lf);
        fclose(lf);
    }

//    string ns="echo "+mes+" >> /tmp/ccfw.log ";
//    system(ns.c_str());
    return;
}
