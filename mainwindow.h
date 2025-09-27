#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Image {
public:
    QString fileName;
    QPixmap pixMap;
    unsigned int numOfShades = 255;
    unsigned int minShade;
    unsigned int maxShade;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Image loadedImage;
    Image targetImage;

private slots:
    void on_loadImage_clicked();
    void on_horizMirror_clicked();
    void on_vertMirror_clicked();
    void on_toGreyscale_clicked();
    void on_reloadImage_clicked();
    void on_quantizerBtn_clicked();
    void on_numOfShadesSpin_valueChanged(int numOfShades);

    void on_saveImage_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
