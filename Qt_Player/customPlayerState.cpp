
#include "customPlayerState.h"


CustomMediaPlayer::CustomMediaPlayer(QObject *parent)
    : QMediaPlayer(parent), customState(IdleState)
{
}


void CustomMediaPlayer::setCustomState(CustomPlayerState newState) {
    customState = newState;
}

CustomPlayerState CustomMediaPlayer::getCustomState() const {
    return customState;
}

