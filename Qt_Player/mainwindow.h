#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "customPlayerState.h"
#include "ui_mainwindow.h"


#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>//>
#include <QGraphicsVideoItem>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QPixmap>
#include <QPalette>
#include <QPainter>
#include <QGraphicsBlurEffect>
#include <QImage>
#include <QResizeEvent>
#include <QPushButton>
#include <QApplication>
#include <QEvent>
#include <QVBoxLayout>
#include <QSlider>
#include <QListWidgetItem>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QString>
#include <QGraphicsScene>
#include <QVideoWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



private:
    Ui::MainWindow *ui;;
    QListWidgetItem* item;
    CustomMediaPlayer* player;
    QAudioOutput *audioOP;
    QGraphicsVideoItem *videoitem;
    QGraphicsScene *sence;
    int currentVoice;
    int count;
    int mode;
    QString durationtime,positiontime;
    bool eventFilter(QObject *watched, QEvent *event) override;
    QString filePath = "C:/Users/Documents/vedioPath.txt";
    void openPlayer(QString path);
    void nextItem(QListWidgetItem *item);
    void preItem(QListWidgetItem *item);
    QString attainPath();
    void selectedItem();
    void keyPressEvent(QKeyEvent *event) override;


private slots:

    void setPosition(qint64 position);
    void durationChanged(qint64 poistion);
    void adjustVideoSize();
    void updateProgressBar(qint64 position);
    void on_playBtn_clicked();
    void setVolume(int value);
    void on_listWidget_itemClicked(QListWidgetItem *item);
    void on_modelBtn_clicked();
    void resizeEvent(QResizeEvent* event) override;
    void handleItemSelection();
};
#endif // MAINWINDOW_H
