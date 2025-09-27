#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "tools.h"
#include <filesystem>
#include <iostream>
#include <tuple>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>
#include <QVector>
#include <QQueue>
using namespace std;

Tools tools;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->numOfShadesSpin->setValue(255);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loadImage_clicked()
{
    loadedImage.fileName = QFileDialog::getOpenFileName(this, tr("Abrir arquivo"), QDir::homePath(), tr("Images (*.png *.jpg)"));

    if (!loadedImage.fileName.isEmpty()){
        // Cria mapa de pixels da imagem origem
        QImage img(loadedImage.fileName);
        loadedImage.pixMap = QPixmap::fromImage(img);

        // Cria mapa de pixels da imagem alvo
        targetImage.fileName = loadedImage.fileName;
        targetImage.pixMap = QPixmap::fromImage(img);

        // Obtém as dimensões das labels das imagens
        int origin_w = ui->originImage->width();
        int origin_h = ui->originImage->height();
        int target_w = ui->targetImage->width();
        int target_h = ui->targetImage->height();

        // Atribui o mapa de pixels à label
        ui->originImage->setPixmap(loadedImage.pixMap.scaled(origin_w, origin_h, Qt::KeepAspectRatio));
        ui->targetImage->setPixmap(targetImage.pixMap.scaled(target_w, target_h, Qt::KeepAspectRatio));

        // Atribui informações de dimensão da imagem à label
        unsigned int cols = targetImage.pixMap.width();
        unsigned int rows = targetImage.pixMap.height();
        ui->imgDimensionsLbl->setText("Resolução: " + QString::number(cols) + "x" + QString::number(rows));

    }
    return;
}

void MainWindow::on_reloadImage_clicked()
{
    if (!loadedImage.fileName.isEmpty()){
        // Cria mapa de pixels da imagem origem
        QImage img(loadedImage.fileName);

        // Cria mapa de pixels da imagem alvo
        targetImage.fileName = loadedImage.fileName;
        targetImage.pixMap = QPixmap::fromImage(img);

        int target_w = ui->targetImage->width();
        int target_h = ui->targetImage->height();

        // Atribui o mapa de pixels à label
        ui->targetImage->setPixmap(targetImage.pixMap.scaled(target_w, target_h, Qt::KeepAspectRatio));
    }
    return;
}

void MainWindow::on_saveImage_clicked()
{
    if (!targetImage.fileName.isEmpty()){
        QString saveFilePath = QFileDialog::getSaveFileName(this, tr("Salvar arquivo"), QDir::homePath(), tr("Images (*.png, *jpeg)"));
        QFile file(saveFilePath);
        file.open(QIODevice::WriteOnly);
        targetImage.pixMap.save(&file, "JPEG");

        filesystem::path originImagePath = targetImage.fileName.toStdString();
        filesystem::path destinyImagePath = saveFilePath.toStdString();

        cout << "Tamanho do arquivo de origem: "
             << filesystem::file_size(originImagePath) << " bytes."
             << endl;

        cout << "Tamanho do arquivo de destino: "
             << filesystem::file_size(destinyImagePath) << " bytes."
             << endl;
    }

    return;
}

void MainWindow::on_horizMirror_clicked()
{
    if (!targetImage.pixMap.isNull()){
        tools.horizontalMirror(targetImage);

        // Renderiza a imagem na label da imagem alvo
        int target_w = ui->targetImage->width();
        int target_h = ui->targetImage->height();
        ui->targetImage->setPixmap(targetImage.pixMap.scaled(target_w, target_h, Qt::KeepAspectRatio));

    } else {
        QMessageBox::information(this, tr("Aviso"), tr("Nenhuma imagem carregada!"));
        return;
    }

    return;
}

void MainWindow::on_vertMirror_clicked()
{
    if (!targetImage.pixMap.isNull()){
        tools.verticalMirror(targetImage);

        // Renderiza a imagem na label da imagem alvo
        int target_w = ui->targetImage->width();
        int target_h = ui->targetImage->height();
        ui->targetImage->setPixmap(targetImage.pixMap.scaled(target_w, target_h, Qt::KeepAspectRatio));

    } else {
        QMessageBox::information(this, tr("Aviso"), tr("Nenhuma imagem carregada!"));
        return;
    }

    return;
}

void MainWindow::on_toGreyscale_clicked()
{
    if (!targetImage.pixMap.isNull()){
        tools.greyscale(targetImage);

        // Renderiza a imagem na label da imagem alvo
        int target_w = ui->targetImage->width();
        int target_h = ui->targetImage->height();
        ui->targetImage->setPixmap(targetImage.pixMap.scaled(target_w, target_h, Qt::KeepAspectRatio));
    }
    return;
}

void MainWindow::on_quantizerBtn_clicked()
{
    if (!targetImage.pixMap.isNull()){
        tools.quantize(targetImage);

        // Renderiza a imagem na label da imagem alvo
        int target_w = ui->targetImage->width();
        int target_h = ui->targetImage->height();
        ui->targetImage->setPixmap(targetImage.pixMap.scaled(target_w, target_h, Qt::KeepAspectRatio));

    }
    return;
}


void MainWindow::on_numOfShadesSpin_valueChanged(int numOfShades)
{
    targetImage.numOfShades = numOfShades;
    return;
}

