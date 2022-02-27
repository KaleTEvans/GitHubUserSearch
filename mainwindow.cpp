#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    setWindowIcon(QIcon(":/Images/GitHub.png"));
    ui->setupUi(this);
    netManager = new QNetworkAccessManager(this);
    netReply = nullptr;
    repoReply = nullptr;
    img = new QPixmap();
}

void MainWindow::clearValues()
{
    ui->labelImage->clear();
    ui->lineEditName->clear();
    ui->lineEditUsername->clear();
    ui->lineEditRepo->clear();
    ui->textEditRepo->clear();
    ui->textEditBio->clear();
    ui->lineEditFollowers->clear();
    ui->lineEditFollowing->clear();
    ui->lineEditAccount->clear();
    dataBuffer.clear();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_searchButton_clicked()
{
    auto username = QInputDialog::getText(this, "Github Username", "Enter a GitHub Username");
    if (!username.isEmpty()) {
        // clear all of the data from the previous search
        clearValues();
        // these are the api calls, one to the user data, and one to the users repos
        QNetworkRequest req{QUrl(QString("https://api.github.com/users/%1").arg(username))};
        QNetworkRequest repoReq{QUrl(QString("https://api.github.com/users/%1/repos").arg(username))};

        // connect and read the data for the user
        netReply = netManager->get(req);
        connect(netReply, &QNetworkReply::readyRead, this, &MainWindow::readData);
        connect(netReply, &QNetworkReply::finished, this, &MainWindow::finishReading);

        // connect and read the data for the user's repos
        repoReply = netManager->get(repoReq);
        connect(repoReply, &QNetworkReply::readyRead, this, &MainWindow::readDataForRepo);
        connect(repoReply, &QNetworkReply::finished, this, &MainWindow::finishedGettingRepos);
    }
}

// Read data custom slot
void MainWindow::readData()
{
    dataBuffer.append(netReply->readAll());
}

// Finished reading custom slot
void MainWindow::finishReading()
{
    if (netReply->error() != QNetworkReply::NoError) {
        // output the netReply error with qDebug
        qDebug() << "Error: " << netReply->errorString();
        QMessageBox::warning(this, "Error", QString("Request[Error] : %1"));
    }  else {
        // Convert the data from a JSON doc to a JSON object
        QJsonObject userJsonInfo = QJsonDocument::fromJson(dataBuffer).object();

        // Set username
        QString login = userJsonInfo.value("login").toString();
        ui->lineEditUsername->setText(login);

        // Set display name
        QString name = userJsonInfo.value("name").toString();
        ui->lineEditName->setText(name);

        // Set bio
        auto bio = userJsonInfo.value("bio").toString();
        ui->textEditBio->setText(bio);

        // Set follower and following count
        auto follower = userJsonInfo.value("followers").toInt();
        auto following = userJsonInfo.value("following").toInt();
        ui->lineEditFollowers->setValue(follower);
        ui->lineEditFollowing->setValue(following);

        // Set account type
        QString type = userJsonInfo.value("type").toString();
        ui->lineEditAccount->setText(type);

        // Set picture
        auto picLink = userJsonInfo.value("avatar_url").toString();
        QNetworkRequest link{QUrl(picLink)};
        netReply = netManager->get(link);
        connect(netReply, &QNetworkReply::finished, this, &MainWindow::setUserImage);
        dataBuffer.clear();
    }
}

// Set the user image
void MainWindow::setUserImage()
{
    img->loadFromData(netReply->readAll());
    QPixmap temp = img->scaled(ui->labelImage->size());
    ui->labelImage->setPixmap(temp);
}

void MainWindow::readDataForRepo()
{
    dataBuffer.append(repoReply->readAll());
}

void MainWindow::finishedGettingRepos()
{
    if(repoReply->error() != QNetworkReply::NoError){
        qDebug() << "Error Getting List of Repo: " << netReply->errorString();
        QMessageBox::warning(this,"Error",QString("Request[Error] : %1").arg(netReply->errorString()));
    }else{
        QJsonArray repoInfo = QJsonDocument::fromJson(dataBuffer).array();
        ui->lineEditRepo->setValue(repoInfo.size());
        for(int i{0}; i < ui->lineEditRepo->value(); ++i){
            auto repo = repoInfo.at(i).toObject();
            QString repoName = repo.value("name").toString();
            ui->textEditRepo->addItem(repoName);
        }
    }
}

