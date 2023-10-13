#include "mainwindow.h"
#include "ui_mainwindow.h"

CUSTOMPLAYERSTATE_H

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    count = 0;
    mode = 0;
    setWindowIcon(QPixmap(":/res/logo.png"));
    setWindowTitle("视频播放器");
    resize(800,500);
    //安装过滤器
    ui->sencewidget->installEventFilter(this);

    //设置按钮的图片
    ui->fileBtn->setIcon(QPixmap(":/res/files.png"));
    ui->preBtn->setIcon(QPixmap(":/res/pre.png"));
    ui->playBtn->setIcon(QPixmap(":/res/play.png"));
    ui->nextBtn->setIcon(QPixmap(":/res/next.png"));
    ui->listBtn->setIcon(QPixmap(":/res/list.png"));
    ui->voiceBtn->setIcon(QPixmap(":/res/voice.png"));
    ui->modelBtn->setIcon(QPixmap(":/res/night.png"));
    ui->LRBtn->setIcon(QPixmap(":/res/blackLarge.png"));
    ui->fileBtn->setIconSize(QSize(25,25));
    ui->preBtn->setIconSize(QSize(25,25));
    ui->playBtn->setIconSize(QSize(25,25));
    ui->nextBtn->setIconSize(QSize(25,25));
    ui->listBtn->setIconSize(QSize(25,25));
    ui->voiceBtn->setIconSize(QSize(25,25));
    ui->modelBtn->setIconSize(QSize(50,25));
    ui->LRBtn->setIconSize(QSize(50,25));

    //设置背景颜色
    QPainter painter(this);
    QPixmap pixmap(":/res/black.jpg");
    pixmap = pixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QString styleSheet = QString("background-image: url(%1);").arg(QUrl::fromLocalFile(pixmap.toImage().pixelColor(0, 0).name()).toString());
    setStyleSheet(styleSheet);
    ui->sencewidget->setStyleSheet("background:black");
    ui->barWidget->setStyleSheet("background-color:gray");

    //qss美化音量进度条
        ui->volume->setStyleSheet(
        "QSlider::groove:horizontal {"
        "border:1px solid skyblue;"
        "background-color:skyblue;"
        "height:10px;"
        "border-radius:5px;"
        "}"

        "QSlider::handle:horizontal {"
        "background:qradialgradient(spread:pad,cx:0.5,cy:0.5,radius:0.5,fx:0.5,fy:0.5,stop:0.7 white,stop:0.8 gray);"
        "width:20px;"
        "border-radius:10px;"
        "margin-top:-5px;"
        "margin-bottom:-5px;}"

        "#horizontalSlider::sub-page:horizontal{"
        "background:black;"
        "margin:5px"
        "border-radius:5px;}"
    );

    //qss美化视频进度条
    ui->progressBar->setStyleSheet(
        "QSlider::groove:horizontal {"
        "border:1px solid skyblue;"
        "background-color:skyblue;"
        "height:10px;"
        "border-radius:5px;"
        "}"

        "QSlider::handle:horizontal {"
        "background:qradialgradient(spread:pad,cx:0.5,cy:0.5,radius:0.5,fx:0.5,fy:0.5,stop:0.7 white,stop:0.8 gray);"
        "width:20px;"
        "border-radius:10px;"
        "margin-top:-5px;"
        "margin-bottom:-5px;}"

        "#horizontalSlider::sub-page:horizontal{"
        "background:black;"
        "margin:5px"
        "border-radius:5px;}"
    );

    //拉取文本中的历史
    QFile historyFile(filePath);
    if (historyFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&historyFile);
        while(!stream.atEnd())
        {
            QString lines = stream.readAll().trimmed();
            QStringList lineLists =lines.split("\n");
            for(const auto &linelist:lineLists){
                QFileInfo FileInfo(linelist);
                QString name = FileInfo.fileName();
                item = new QListWidgetItem(name);
                item->setForeground(Qt::white);
                ui-> listWidget->setStyleSheet("QListWidget::item { color: white; }");
                ui->listWidget->setStyleSheet("QListWidget::item:hover { background-color:rgb(47,210,245); }");
                ui->listWidget->addItem(item);
                }
        }
        historyFile.close();
    }

    //实例化对象
    player = new CustomMediaPlayer(this);
    audioOP = new QAudioOutput(this);
    sence = new QGraphicsScene(this);
    ui->graphicsView->setScene(sence);
    videoitem = new QGraphicsVideoItem;
    //设置限制
    videoitem->setFlags(QGraphicsVideoItem::ItemIsSelectable |
                        QGraphicsVideoItem::ItemIsFocusable);
    sence->addItem(videoitem);
    player->setVideoOutput(videoitem);
    player->setAudioOutput(audioOP);
    ui->volume->setSliderPosition(100);
    player->setPosition(0);
    //设置list按钮的展开与隐藏
    QObject::connect(ui->listBtn, &QPushButton::clicked, [=]()
    {
        setFocus();
        if (ui->listWidget->isHidden()) {
            ui->listWidget->show();
            adjustVideoSize();
        } else {
            ui->listWidget->hide();
            adjustVideoSize();
        }
    });

    //自动播放下一个视频
    connect(player,&CustomMediaPlayer::mediaStatusChanged,this,[=](){
        if (player->mediaStatus() == QMediaPlayer::EndOfMedia) {
            QListWidgetItem *current = ui->listWidget->currentItem();
            int currentIndex = ui->listWidget->row(current);
            int nextIndex = currentIndex + 1;
            if(nextIndex >= ui->listWidget->count())
            {
                nextIndex = 0;
            }
            nextItem(current);
            QListWidgetItem *nextItem = ui->listWidget->item(nextIndex);
            ui->listWidget->setCurrentItem(nextItem);
        }
    });

    //音量按钮，实现切换静音的效果
    bool isMuted = false;
    connect(ui->voiceBtn,&QPushButton::clicked,[=]() mutable{
        setFocus();
        if(isMuted)
        {
            player->setAudioOutput(audioOP);
            ui->voiceBtn->setIcon(QPixmap(":/res/voice.png"));
            isMuted = false;
        } else
        {
            player->setAudioOutput(NULL);
            ui->voiceBtn->setIcon(QPixmap(":/res/mute.png"));
            isMuted = true;
        }
    });

    //声音改变与音量的函数变化
    connect(ui->volume, &QSlider::valueChanged, this, &MainWindow::setVolume);

    //播放长度信号变化
    connect(player,SIGNAL(durationChanged(qint64)),this,SLOT(durationChanged(qint64)));

    //动态调整当窗口大小改变时，调节视频视窗大小
    connect(this, &MainWindow::resizeEvent, this, &MainWindow::resizeEvent);

    //连接播放与按钮信号与槽
    connect(ui->playBtn,&QPushButton::clicked,player,&CustomMediaPlayer::plays);

    //下一个视频的按钮
    connect(ui->nextBtn,&QPushButton::clicked,this,[=](){
        QListWidgetItem *current = ui->listWidget->currentItem();
        int currentIndex = ui->listWidget->row(current);
        int nextIndex = currentIndex + 1;
        if(nextIndex >= ui->listWidget->count())
        {
            nextIndex = 0;
        }
        nextItem(current);
        QListWidgetItem *nextItem = ui->listWidget->item(nextIndex);
        ui->listWidget->setCurrentItem(nextItem);
    });

    //上一个视频
    connect(ui->preBtn,&QPushButton::clicked,this,[=](){
        QListWidgetItem *current = ui->listWidget->currentItem();
        int currentIndex = ui->listWidget->row(current);
        int preIndex = currentIndex - 1;
        if(preIndex < 0)
        {
            preIndex = ui->listWidget->count()-1;
        }
        preItem(current);
        QListWidgetItem *preItem = ui->listWidget->item(preIndex);
        ui->listWidget->setCurrentItem(preItem);
    });


    //打开文件按钮
    connect(ui->fileBtn,&QPushButton::clicked,[=](){
        auto path = QFileDialog::getOpenFileName(this,"打开视频文件","C:\\Users\\Videos","MP4 Files(*.mp4);;All Files(*.*)");
        bool pathExists = false;
        //判断是否存在文本文件
        QFile file(filePath);
        if (!file.exists()) {
            if (file.open(QIODevice::WriteOnly)) {
                file.close();
            }
        }
        //写入路径并判断写入的路径是否存在，如果存在就不写入
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString line;
            QTextStream stream(&file);
            while(!stream.atEnd())
            {
                line = stream.readLine().trimmed();
                if(line == path)
                {
                    pathExists = true;
                    break;
                }
            }
            file.close();
            if(!pathExists)
            {
                QFile writeFile(filePath);
                if(writeFile.open(QIODevice::Append | QIODevice::Text))
                {
                    QTextStream writeStream(&writeFile);
                    // 写入path到文件
                    writeStream << path << Qt::endl;
                    writeFile.close();
                }
            }
        }
        if(path.isEmpty()){
            QMessageBox::information(this,"错误","打开视频文件失败！",QMessageBox::Yes);
            return;
        }
        //获取文件信息
        QFileInfo FileInfo(path);
        QString name = FileInfo.fileName();
        QList<QListWidgetItem*> existingItems = ui->listWidget->findItems(name, Qt::MatchExactly);
        //判断放入的视频名称是否已存在
        if (existingItems.isEmpty())
        {
        item = new QListWidgetItem(name);
        item->setForeground(Qt::white);
        ui->listWidget->addItem(item);
        }
        ui-> listWidget->setStyleSheet("QListWidget::item { color: white; }");
        ui->listWidget->setStyleSheet("QListWidget::item:hover { background-color:rgb(47,210,245);");
        openPlayer(path);
        audioOP->setVolume(100);
        ui->listWidget->show();
        ui->volume->setSliderPosition(100);
        ui->playBtn->setIcon(QIcon(":/res/pause.png"));
    });
    //缩放视频
    connect(ui->LRBtn,&QPushButton::clicked,this,[=]()
    {
        setFocus();
        if (windowState() & Qt::WindowFullScreen)
        {
            setWindowState(windowState() & ~Qt::WindowFullScreen);
            ui->LRBtn->setIcon(QIcon(":/res/blackLarge.png"));
        }
        else
        {
            setWindowState(windowState() | Qt::WindowFullScreen);
            ui->LRBtn->setIcon(QIcon(":/res/BlackReduce.png"));
        }
    });
    //实时更新进度条进度
    connect(player, &CustomMediaPlayer::positionChanged, this, &MainWindow::updateProgressBar);
    //设置项目改变时，当前项目进入聚焦点
    connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(handleItemSelection()));
    //设置进度条
    void(QAbstractSlider:: *barMove)(int) = &QAbstractSlider::sliderMoved;
    void(MainWindow:: *setPos)(qint64) = &MainWindow::setPosition;
    connect(ui->progressBar,barMove,this,setPos);

}


MainWindow::~MainWindow()
{
    delete ui;
}

    //重写resizeEvent函数，实现动态调整函数
    void MainWindow::resizeEvent(QResizeEvent* event)
    {
        QMainWindow::resizeEvent(event);
        adjustVideoSize();
    }
    //调整graphisc视窗大小的函数
    void MainWindow::adjustVideoSize() {
    if (videoitem && ui->graphicsView) {
        QRectF videoRect = videoitem->boundingRect();
        ui->graphicsView->fitInView(videoRect, Qt::KeepAspectRatio);
        ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }
    //定义一个播放状态的函数
    void CustomMediaPlayer::plays()
    {
        switch (getCustomState())
        {

        case CustomPlayerState::PlayingState:
            pause();
            setCustomState(CustomPlayerState(PausedState));
            break;
        case CustomPlayerState::PausedState:
            play();
            setCustomState(CustomPlayerState(PlayingState));
            break;
        case IdleState:
            break;
        }
    }
    //随时更新进度条的函数
    void MainWindow::updateProgressBar(qint64 position)
    {
        ui->progressBar->setValue(position);
    }
    //设置进度条位置的函数
    void MainWindow::setPosition(qint64 position)
    {
        player->setPosition(position);
    }

    //设置进度条的范围的函数
    void MainWindow::durationChanged(qint64 duration)
    {
        ui->progressBar->setRange(0,duration);
    }

    //play按钮的点击事件，同时切换图片的函数
    void MainWindow::on_playBtn_clicked()
    {
        setFocus();
        if (player->getCustomState() == PlayingState) {
            ui->playBtn->setToolTip("暂停");
            ui->playBtn->setIcon(QIcon(":/res/play.png"));
        } else {
            ui->playBtn->setToolTip("播放");
            ui->playBtn->setIcon(QIcon(":/res/pause.png"));
        }
    }
    //用事件过滤器来隐藏显示下列进度条与控件的函数
    bool MainWindow::eventFilter(QObject *watched, QEvent *event)
    {

        if (event->type() == QEvent::Enter)
        {

            // 鼠标离开小部件，隐藏小部件
            QTimer::singleShot(7000, this, [this]() {
                // 延时2秒后执行的代码
                ui->componentWidgrt->hide();
                ui->progressBar->hide();
            });
        }
        else if (event->type() == QEvent::Leave)
        {
            // 鼠标进入小部件，显示小部件（取消隐藏）

            ui->componentWidgrt->show();
            ui->progressBar->show();
        }
        return QWidget::eventFilter(watched, event);
    }
    //设置声音大小的函数
    void MainWindow::setVolume(int value)
    {
        // 将滑块的值映射到0.0-1.0之间
        qreal volume = value / 100.0;
        audioOP->setVolume(volume);
    }
    //list窗体上的视频名称的点击事件函数
    void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
    {
        QString savedAllFilePath,fileNameSp;
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            fileNameSp = item->text();
            savedAllFilePath = in.readAll().trimmed();
            QStringList words = savedAllFilePath.split("\n");
            for(const auto &splitWord:words){
                if(splitWord.contains(fileNameSp))
                {
                    openPlayer(splitWord);
                    ui->listWidget->show();
                    file.close();
                    break;
                }
            }
        }
        selectedItem();
        ui->playBtn->setIcon(QIcon(":/res/pause.png"));
    }
    //播放视频，并调整大小函数
    void MainWindow::openPlayer(QString path)
    {
        player->setSource(QUrl::fromLocalFile(path));
        player->play();
        player->setCustomState(PlayingState);
        //调整画面大小到适合GraphicsView
        adjustVideoSize();
        selectedItem();
        if(count % 2 !=1 )
        {
            ui-> listWidget->setStyleSheet("QListWidget::item { color: white; }");
        }else
        {
            ui-> listWidget->setStyleSheet("QListWidget::item { color: black; }");
        }
    }
    //下一个视频的函数
    void MainWindow::nextItem(QListWidgetItem *item)
    {
        int nowcount = 0,count;
        QString allPath = attainPath();
        QString nextPath;
        QStringList splitPaths = allPath.split("\n");
        count = splitPaths.size();
        for(const auto &splitPath : splitPaths)
        {
            ++nowcount;
            if(splitPath.contains(item->text()))
            {
                if(nowcount+1 <= count)
                {
                    nextPath = splitPaths[nowcount];
                } else
                {
                    nextPath = splitPaths[0];
                }
                openPlayer(nextPath);
                selectedItem();
                break;
            }
        }
    }
    //上一个视频的函数
    void MainWindow::preItem(QListWidgetItem *item)
    {
        int nowcount = 0,count;
        QString allPath = attainPath();
        QString prePath;
        QStringList splitPaths = allPath.split("\n");
        count = splitPaths.size();
        for(const auto &splitPath : splitPaths)
        {
            ++nowcount;
            if(splitPath.contains(item->text()))
            {
                if(nowcount <= 1)
                {
                    prePath = splitPaths[count-1];
                } else
                {
                    prePath = splitPaths[nowcount-2];
                }
                openPlayer(prePath);
                selectedItem();
                break;
            }
        }
    }
    //获取文本文件中的所有路径的函数
    QString MainWindow::attainPath()
    {
        QString path;
        QFile file(filePath);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
        QTextStream read(&file);
            path = read.readAll().trimmed();
         qDebug()<<path;
        }
        return path;
    }
    //被选中的视频的美化函数
    void MainWindow::selectedItem()
    {

        ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->listWidget->setStyleSheet("QListWidget::item:selected { background-color: rgb(47,210,245); }");
        if(count % 2 !=1 )
        {
        }else
        {
            ui-> listWidget->setStyleSheet("QListWidget::item { color: black; }");
        }
    }

    //切换模式函数
    void MainWindow::on_modelBtn_clicked()
    {
        ++count;
        setFocus();
        if (count % 2 != 1)
        {
            ui->modelBtn->setIcon(QPixmap(":/res/night"));
            QPixmap pixmap(":/res/black.jpg");
            pixmap = pixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            QString styleSheet = QString("background-image: url(%1);").arg(QUrl::fromLocalFile(pixmap.toImage().pixelColor(0, 0).name()).toString());
            setStyleSheet(styleSheet);
            ui->sencewidget->setStyleSheet("background:black");
            ui->listWidget->setStyleSheet("QListWidget::item { color: white; }"
                                          "QListWidget::item:hover { background-color: rgb(47, 210, 245); }");
            ui->barWidget->setStyleSheet("background-color: gray;");
            ui->componentWidgrt->setStyleSheet("QPushButton { width: 30px; height: 30px; border-radius: 8px; background-color: gray; }"
                                               "QPushButton:hover { width: 30px; height: 30px; border-radius: 8px; background-color: rgb(47, 210, 245); }"
                                                "* {background-color:gray;}");
            this->setStyleSheet( " QMainWindow::titleBar { background-color: gray; }");
            adjustVideoSize();
            selectedItem();
        }
        else
        {
            ui->modelBtn->setIcon(QPixmap(":/res/whiteday.png"));
            QPixmap pixmap(":/res/white.jpg");
            pixmap = pixmap.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            QString styleSheet = QString("background-image: url(%1);").arg(QUrl::fromLocalFile(pixmap.toImage().pixelColor(0, 0).name()).toString());
            setStyleSheet(styleSheet);
            ui->sencewidget->setStyleSheet("background:white");
            ui->listWidget->setStyleSheet("QListWidget::item { color: black; }");
            ui->barWidget->setStyleSheet("background-color: white;");
            ui->componentWidgrt->setStyleSheet("QPushButton { width:30px; height: 30px; border-radius: 8px; background-color: white; }"
                                               "QPushButton:hover { width: 30px; height: 30px; border-radius: 8px; background-color: rgb(47, 210, 245); }"
                                               "* {background-color: white;}");
            adjustVideoSize();
            selectedItem();
        }
    }

    void MainWindow::handleItemSelection()
    {
        // 设置焦点在主窗口上，以便处理键盘事件
        setFocus();
        QKeyEvent* keyEvent = QApplication::keyboardModifiers() == Qt::NoModifier ?
                                  new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier) :
                                  new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
        QApplication::postEvent(this, keyEvent);
    }

    void MainWindow::keyPressEvent(QKeyEvent *event)
    {
        if(event->key() == Qt::Key_Escape)
        {
            //按ESC键
            if(windowState() & Qt::WindowFullScreen)
            {
                setWindowState(windowState() & ~Qt::WindowFullScreen);
            }
        }
        // 按下左键
        if (event->key() == Qt::Key_Left) {
            qint64 newPosition = player->position() - 5000;
            player->setPosition(newPosition);
        }
        // 按下右键
        else if (event->key() == Qt::Key_Right) {
            qint64 newPosition = player->position() + 5000;
            player->setPosition(newPosition);
        }

        QMainWindow::keyPressEvent(event);
    }
