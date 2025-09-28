#include "tools.h"
#include <iostream>
#include <iostream>
#include <tuple>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>
#include <QVector>
#include <QQueue>

#include <QtCharts/QChartView>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>

#include "mainwindow.h"

using namespace std;

Tools::Tools() {}

void Tools::horizontalMirror(Image &img) {
    QImage image = img.pixMap.toImage();

    if (image.format() != QImage::Format_ARGB32 && image.format() != QImage::Format_RGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    int rows = image.height();
    int cols = image.width();
    int bytesPerPixel = 4; // BGRA (ou ARGB)
    int stride = image.bytesPerLine();

    uchar *data = image.bits();

    for (int i = 0; i < rows; i++) {
        uchar *row = data + i * stride;

        // Ponteiros para o começo e fim da linha
        uchar *left  = row;
        uchar *right = row + (cols - 1) * bytesPerPixel;

        for (int j = 0; j < cols / 2; j++) {
            // Troca 4 bytes (1 pixel inteiro) de uma vez
            uchar tmp[4];
            memcpy(tmp, left, bytesPerPixel);
            memcpy(left, right, bytesPerPixel);
            memcpy(right, tmp, bytesPerPixel);

            left  += bytesPerPixel;
            right -= bytesPerPixel;
        }
    }

    img.pixMap = QPixmap::fromImage(image);
}

void Tools::verticalMirror(Image &img) {
    QImage image = img.pixMap.toImage();

    if (image.format() != QImage::Format_ARGB32 && image.format() != QImage::Format_RGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    int rows = image.height();
    int stride = image.bytesPerLine(); // número de bytes por linha
    uchar *data = image.bits();

    // Faz swap entre linhas do topo e de baixo
    std::vector<uchar> temp(stride);

    for (int i = 0; i < rows / 2; i++) {
        uchar *rowTop    = data + i * stride;
        uchar *rowBottom = data + (rows - 1 - i) * stride;

        // Troca as duas linhas inteiras
        memcpy(temp.data(), rowTop, stride);
        memcpy(rowTop, rowBottom, stride);
        memcpy(rowBottom, temp.data(), stride);
    }

    img.pixMap = QPixmap::fromImage(image);
}


void Tools::greyscale(Image &img){
    QImage image = img.pixMap.toImage();
    unsigned int cols = img.pixMap.width();
    unsigned int rows = img.pixMap.height();

    // Cria histograma
    hist = {0};

    // Para acessar os bytes da imagem diretamente
    uchar *data = image.bits();
    int stride = image.bytesPerLine();

    for (unsigned int i = 0; i < rows; i++) {
        uchar *row = data + i * stride;
        for (unsigned int j = 0; j < cols; j++) {
            // Se a imagem estiver em 32 bits, cada pixel tem 4 bytes (BGRA)
            uchar b = row[j*4 + 0];
            uchar g = row[j*4 + 1];
            uchar r = row[j*4 + 2];

            unsigned int lumPixel = static_cast<unsigned int>(0.299 * r + 0.587 * g + 0.114 * b);

            hist[lumPixel]++;

            // sobrescreve pixel no formato BGRA
            row[j*4 + 0] = lumPixel; // B
            row[j*4 + 1] = lumPixel; // G
            row[j*4 + 2] = lumPixel; // R
        }
    }
    unsigned int minShade = 0;
    while (minShade < 256 && hist[minShade] == 0) minShade++;
    unsigned int maxShade = 255;
    while (maxShade > 0 && hist[maxShade] == 0) maxShade--;

    // Salva os tons mínimos e máximos da imagem
    img.minShade = minShade;
    img.maxShade = maxShade;

    // Atualiza mapa de pixels da imagem alvo na memória
    img.lumPixMap = QPixmap::fromImage(image);
    //img.pixMap = img.lumPixMap;
}

void Tools::quantize(Image &img){

    if (img.lumPixMap.isNull()){
        Tools::greyscale(img);
    }

    // Obtém dados da imagem
    QImage image = img.lumPixMap.toImage();
    uchar *data = image.bits();
    int stride = image.bytesPerLine();

    unsigned int cols = img.lumPixMap.width();
    unsigned int rows = img.lumPixMap.height();

    unsigned int tam_int = img.maxShade - img.minShade + 1;
    unsigned int n = img.numOfShades;
    unsigned int tb = tam_int / n; // tamanho do bin

    for (unsigned int i = 0; i < rows; i++) {
        uchar *row = data + i * stride;
        for (unsigned int j = 0; j < cols; j++) {
            // Obtém RGB do pixel
            uchar b = row[j*4 + 0]; // B
            uchar g = row[j*4 + 1]; // G
            uchar r = row[j*4 + 2]; // R

            // Luminância do pixel
            unsigned int t_orig = static_cast<int>(0.299 * r + 0.587 * g + 0.114 * b);

            // Se o número de tons for menor que o tamanho do intervalo do bin
            if (n < tam_int){
                // Computa o índice do bin correspondente para a luminância t_orig.
                int binIndex = static_cast<int>(((t_orig - img.minShade) / tb));
                if (binIndex >= (int)n) binIndex = n - 1; // garante que não arredonda pra cima

                double lower = img.minShade - 0.5 + binIndex * tb;
                double upper = img.minShade - 0.5 + (binIndex+1) * tb;

                int centralShadeValue = std::round((lower + upper) / 2.0);

                row[j*4 + 0] = centralShadeValue; // B
                row[j*4 + 1] = centralShadeValue; // G
                row[j*4 + 2] = centralShadeValue; // R
            }
        }
    }

    // Atualiza mapa de pixels da imagem alvo na memória
    img.pixMap = QPixmap::fromImage(image);
    img.lumPixMap = QPixmap::fromImage(image);
}

QChartView* Tools::lumHistogram(Image &img) {

    histogramView = new QChartView();
    histogramView->setRenderHint(QPainter::Antialiasing);

    // Normaliza histograma e atribui ao conjunto de barras do gráfico
    QBarSet *set0 = new QBarSet("");
    for (int i = 0; i < 256; i++) {
        *set0 << static_cast<double>(hist[i]) / img.maxShade;
    }

    set0->setColor(Qt::white); // barras pretas

    QBarSeries *series = new QBarSeries();
    series->append(set0);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->hide();
    chart->setBackgroundVisible(false);
    chart->setMargins(QMargins(0,0,0,0));

    // Configura eixo x (0-255)
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    QStringList categories;
    for(int i = 0; i < 256; ++i) categories << QString::number(i);
    axisX->append(categories);
    axisX->setLabelsVisible(false);
    axisX->setGridLineVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // Atualizar chartView existente
    histogramView->setChart(chart);

    return histogramView;
}

void Tools::updateBright(int bias, Image &img){
    //if (img.lumPixMap.isNull()){
    //    Tools::greyscale(img);
    //}

    // Converte para RGB32, garante cores
    QImage image = img.pixMap.toImage();

    unsigned int cols = img.pixMap.width();
    unsigned int rows = img.pixMap.height();

    // Para acessar os bytes da imagem diretamente
    uchar *data = image.bits();
    int stride = image.bytesPerLine();

    hist.fill(0); // zera histograma

    for (unsigned int i = 0; i < rows; i++) {
        uchar* row = data + i * stride;
        for (unsigned int j = 0; j < cols; j++) {
            int b = static_cast<int>(row[j*4 + 0]) + bias;
            int g = static_cast<int>(row[j*4 + 1]) + bias;
            int r = static_cast<int>(row[j*4 + 2]) + bias;

            // Saturação 0..255
            unsigned char final_b = static_cast<uchar>(std::min(std::max(b, 0), 255));
            unsigned char final_g = static_cast<uchar>(std::min(std::max(g, 0), 255));
            unsigned char final_r = static_cast<uchar>(std::min(std::max(r, 0), 255));

            // Sobrescreve pixel
            row[j*4 + 0] = final_b;
            row[j*4 + 1] = final_g;
            row[j*4 + 2] = final_r;

            // Histograma por luminância usando os valores finais
            int lum = static_cast<int>(0.299*final_r + 0.587*final_g + 0.114*final_b);
            hist[lum]++;
        }
    }

    // Atualiza mapa de pixels da imagem alvo na memória
    img.pixMap = QPixmap::fromImage(image);
}

void Tools::updateContrast(double gain, Image &img){
    if (img.lumPixMap.isNull()){
        Tools::greyscale(img);
    }

    QImage image = img.lumPixMap.toImage();
    unsigned int cols = img.lumPixMap.width();
    unsigned int rows = img.lumPixMap.height();

    // Para acessar os bytes da imagem diretamente
    uchar *data = image.bits();
    int stride = image.bytesPerLine();

    double lumPixel = 0.0;
    double newPixel = 0;

    for (unsigned int i = 0; i < rows; i++) {
        uchar *row = data + i * stride;
        for (unsigned int j = 0; j < cols; j++) {
            // Se a imagem estiver em 32 bits, cada pixel tem 4 bytes (BGRA)
            uchar b = row[j*4 + 0];
            uchar g = row[j*4 + 1];
            uchar r = row[j*4 + 2];

            lumPixel = static_cast<float>(0.299 * r + 0.587 * g + 0.114 * b);
            newPixel = gain * lumPixel;
            newPixel = std::min(std::max(newPixel, 0.0), 255.0);

            hist[static_cast<unsigned int>(newPixel)]++;

            // sobrescreve pixel no formato BGRA
            row[j*4 + 0] = newPixel; // B
            row[j*4 + 1] = newPixel; // G
            row[j*4 + 2] = newPixel; // R
        }
    }
    unsigned int minShade = 0;
    while (minShade < 256 && hist[minShade] == 0) minShade++;
    unsigned int maxShade = 255;
    while (maxShade > 0 && hist[maxShade] == 0) maxShade--;

    // Salva os tons mínimos e máximos da imagem
    img.minShade = minShade;
    img.maxShade = maxShade;

    // Atualiza mapa de pixels da imagem alvo na memória
    img.lumPixMap = QPixmap::fromImage(image);
    img.pixMap = img.lumPixMap;
}

void Tools::updateNegative(Image &img){
    QImage image = img.pixMap.toImage();

    unsigned int cols = img.pixMap.width();
    unsigned int rows = img.pixMap.height();

    // Para acessar os bytes da imagem diretamente
    uchar *data = image.bits();
    int stride = image.bytesPerLine();

    hist.fill(0); // zera histograma

    for (unsigned int i = 0; i < rows; i++) {
        uchar* row = data + i * stride;
        for (unsigned int j = 0; j < cols; j++) {
            int b = 255 - static_cast<int>(row[j*4 + 0]);
            int g = 255 - static_cast<int>(row[j*4 + 1]);
            int r = 255 - static_cast<int>(row[j*4 + 2]);

            // Saturação 0..255
            unsigned char final_b = static_cast<uchar>(std::min(std::max(b, 0), 255));
            unsigned char final_g = static_cast<uchar>(std::min(std::max(g, 0), 255));
            unsigned char final_r = static_cast<uchar>(std::min(std::max(r, 0), 255));

            // Sobrescreve pixel
            row[j*4 + 0] = final_b;
            row[j*4 + 1] = final_g;
            row[j*4 + 2] = final_r;

            // Histograma por luminância usando os valores finais
            int lum = static_cast<int>(0.299*final_r + 0.587*final_g + 0.114*final_b);
            hist[lum]++;
        }
    }

    // Atualiza mapa de pixels da imagem alvo na memória
    img.pixMap = QPixmap::fromImage(image);
}
