#include "headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QUuid>

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
                                                   tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));
    if (!fileName.isEmpty()) {
        // Scale the image while preserving the aspect ratio and then set it on the label
        QPixmap pixmap;
        if (pixmap.load(fileName)){
            ui->ImageLabel->setPixmap(pixmap.scaled(ui->ImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->ImageFrame->setStyleSheet("");
            ui->OutputBox->setText("");
            this->image = Image(fileName.toUtf8().constData());
        }
        else{
            QMessageBox::critical(this, tr("Error"), tr("Could not open the file"));
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
    QString input = ui->TextInputBox->toPlainText();
    //makes sure the user has given an image and text
    if(input.length()>0 && this->image.isEmpty() == false){
        //checks if redundacny box is checked
        bool redundancy = false;
        if (ui->RedundancyCheck->checkState() == Qt::Checked){
            redundancy = true;
        }
        //gets the noise
        int noise = 0;
        switch(ui->NoiseSelect->currentIndex()){
        case(0):
            noise = 1;
            break;
        case(1):
            noise = 3;
            break;
        case(2):
            noise = 4;
            break;
        case(3):
            noise = 6;
            break;
        case(4):
            noise = 8;
            QMessageBox::StandardButton reply;
            reply = QMessageBox::warning(this, tr("Warning"), tr("Are you sure you want to use full noise?\n It may leave the image unrecognisable with long messages."),QMessageBox::Yes|QMessageBox::No);
            if(reply == QMessageBox::No){
                return;
            }
            break;
        }
        //runs with selected identifier type
        bool success = false;
        switch(ui->LabelSelect->currentIndex()){
        case(0):
            success = EncodeNoID(input, noise, redundancy);
            break;
        case(1):
            success = EncodeIncremental(input, noise, redundancy);
            break;
        case(2):
            success = EncodeUniqueID(input, noise, redundancy);
            break;
        }
        //if the encode functions returned true, change the border to green to signify
        //else change it to red and show an error box informing the user
        if(success){
            ui->ImageFrame->setStyleSheet("QFrame { border: 2px solid green; }");
            //Image has been edited for encoding so needs to be reloaded to avoid encoding over previous work
            QString fileName = this->image.filename;
            this->image =  Image(fileName.toUtf8().constData());
        }
        else{
            ui->ImageFrame->setStyleSheet("QFrame { border: 2px solid red; }");
            //Reload image here for safety, not sure if neccessary
            QString fileName = this->image.filename;
            this->image = Image(fileName.toUtf8().constData());

            QMessageBox::critical(this, tr("Error"), tr("You have entered too much text for this image, please shorten the input or increase the noise"));
        }

    }
    else{
        QMessageBox::critical(this, tr("Error"), tr("Please upload an image and add text before generating"));
    }
}

bool MainWindow::EncodeNoID(QString input, int noise, bool redundancy){
    for (int i = 0; i < ui->NumImages->value(); ++i){
        if(!(this->image.encode((input.toUtf8().constData()),(QString::number(i)).toStdString().c_str(),noise, redundancy))){
            return false;
            break;
        }
    }
    return true;
}

bool MainWindow::EncodeIncremental(QString input, int noise, bool redundancy){
    QString inputId;
    for (int i = 0; i < ui->NumImages->value(); ++i){
        inputId = input + QString::number(i);
        if(!(this->image.encode((inputId.toUtf8().constData()),(QString::number(i)).toStdString().c_str(),noise, redundancy))){
            return false;
            break;
        }
    }
    return true;
}

bool MainWindow::EncodeUniqueID(QString input, int noise, bool redundancy){
    QString uniqueId;
    for (int i = 0; i < ui->NumImages->value(); ++i){
        uniqueId =(QUuid::createUuid().toString());
        if(!(this->image.encode(((input+uniqueId).toUtf8().constData()),(uniqueId).toStdString().c_str(),noise, redundancy))){
            return false;
            break;
        }
    }
    return true;
}

void MainWindow::on_DecodeBtn_clicked()
{
    //decodes to get the message
    QString message = this->image.decode();
    //if it is zero then decoding has failed
    if(message.length() > 0) {
        //displays encoded text
        ui->OutputBox->setText(message);
        //writes to file if selected
        if(ui->saveText->checkState() == Qt::Checked){
            if(ui->FilenameBox->text() == ""){
                QFile file("../StegApp/output.txt");

                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QMessageBox::critical(this, tr("Error"), tr("Could not write to file"));
                    return;
                }

                QTextStream out(&file);
                out << message;
                file.close();
            }
            //uses name if given
            else{
                QFile file("../StegApp/" + ui->FilenameBox->text() + ".txt");

                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QMessageBox::critical(this, tr("Error"), tr("could not right to file"));
                    return;
                }

                QTextStream out(&file);
                out << message;
                file.close();
            }
        }
    }
    else{
        QMessageBox::critical(this, tr("Error"), tr("Please upload an encoded image"));
    }
}
