#ifndef CUSTOMPLAYERSTATE_H
#define CUSTOMPLAYERSTATE_H

#include "qmediaplayer.h"

#include <QMediaPlayer>

enum CustomPlayerState {
    IdleState,
    PlayingState,
    PausedState,
};

class CustomMediaPlayer : public QMediaPlayer
{
    Q_OBJECT



public:
    CustomMediaPlayer(QObject *parent = nullptr);
    void setCustomState(CustomPlayerState newState) ;
    CustomPlayerState getCustomState() const;
    void customSetVolume(int volume);

private:
CustomPlayerState customState;
    //void onPositionchanged(qint64 position);
   /* CustomMediaPlayer *player*/;


public slots:
void plays();
};
#endif // CUSTOMPLAYERSTATE_H
