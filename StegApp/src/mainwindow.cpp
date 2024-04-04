#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QUuid>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QFile f(":qdarkstyle/dark/darkstyle.qss");
    if (!f.exists())   {
        printf("Unable to set stylesheet, file not found\n");
    }
    else   {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }

    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_EncodeImageUploadBtn_clicked()
{
    //opens a file input that filters for images only
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image"), "",
                                                   tr("Image Files (*.png *.jpg *.jpeg *.bmp *.gif)"));
    if (!fileName.isEmpty()) {
        // Scale the image while preserving the aspect ratio and then set it on the label
        QPixmap pixmap;
        if (pixmap.load(fileName)){
            ui->ImageLabel->setPixmap(pixmap.scaled(ui->ImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->ImageFrame->setStyleSheet("");
            this->image = new Image(fileName.toStdString().c_str());
        }
        else{
            qDebug() << "pixmap failed to load";
        }

    }

}

void MainWindow::on_TextUploadBtn_clicked()
{
    //opens a file input that filters for txt only
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Text"), "",
                                                   tr("*.txt"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString fileContent = in.readAll();
            file.close();

            ui->TextInputBox->setText(fileContent);
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Could not open the file"));
        }
    }
}

void MainWindow::on_EncodeBtn_clicked()
{
    //TODO - Implement noise feature
    QString input = ui->TextInputBox->toPlainText();
    if(input.length()>0 && this->image != nullptr){
        switch(ui->LabelSelect->currentIndex()){
        case(0):
            EncodeNoID(input);
            break;
        case(1):
            EncodeIncremental(input);
            break;
        case(2):
            EncodeUniqueID(input);
            break;
        }

        ui->ImageFrame->setStyleSheet("QFrame { border: 2px solid green; }");
    }
    else{
        QMessageBox::critical(this, tr("Error"), tr("Please upload an image and add text before generating"));
    }
}

void MainWindow::EncodeNoID(QString input){
    for (int i = 0; i < ui->NumImages->value(); ++i){
        this->image->encode((input.toStdString().c_str()),(QString::number(i)).toStdString().c_str());
    }
}

void MainWindow::EncodeIncremental(QString input){
    QString inputId;
    for (int i = 0; i < ui->NumImages->value(); ++i){
        inputId = input + QString::number(i);
        this->image->encode((inputId.toStdString().c_str()),(QString::number(i)).toStdString().c_str());
    }
}

void MainWindow::EncodeUniqueID(QString input){
    QString uniqueId;
    for (int i = 0; i < ui->NumImages->value(); ++i){
        uniqueId =(QUuid::createUuid().toString());
        this->image->encode(((input+uniqueId).toStdString().c_str()),(uniqueId).toStdString().c_str());
    }
}

void MainWindow::on_DecodeBtn_clicked()
{
    QString message = this->image->decode();
    if(message.length() > 0) {
        ui->OutputBox->setText(message);

    }
    else{
        QMessageBox::critical(this, tr("Error"), tr("Please upload an encoded image"));
    }
}
