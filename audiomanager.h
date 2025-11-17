#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QHash>
#include <QString>

enum class SoundEffect {
    PieceAttacked,
    PieceEvolved,
    AlertSize,
    AlertGeneric,
    AlertVictory,
    GameStart,
    RecruitPiece,
    MovePiece
};

class AudioManager : public QObject
{
    Q_OBJECT
public:
    static AudioManager& instance();

    void initialize();
    void playBackgroundMusic();
    void stopBackgroundMusic();
    void pauseBackgroundMusic();

    void playSoundEffect(SoundEffect effect);

    void setMusicVolume(int volume);
    void setSFXVolume(int volume);
    void setMasterVolume(int volume);

    int getMusicVolume() const { return musicVolume; }
    int getSFXVolume() const { return sfxVolume; }
    int getMasterVolume() const { return masterVolume; }

    bool isMusicMuted() const { return musicMuted; }
    bool isSFXMuted() const { return sfxMuted; }
    bool isMasterMuted() const { return masterMuted; }

    void setMusicMuted(bool muted);
    void setSFXMuted(bool muted);
    void setMasterMuted(bool muted);

    void saveSettings();
    void loadSettings();

signals:
    void volumeChanged();

private:
    AudioManager();
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void updateMusicVolume();
    void updateSFXVolume();
    void loadSoundEffects();

    QMediaPlayer* backgroundMusic;
    QAudioOutput* musicOutput;

    QHash<SoundEffect, QMediaPlayer*> soundEffects;
    QHash<SoundEffect, QAudioOutput*> sfxOutputs;

    int musicVolume;
    int sfxVolume;
    int masterVolume;

    bool musicMuted;
    bool sfxMuted;
    bool masterMuted;
};

#endif
