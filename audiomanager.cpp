#include "audiomanager.h"
#include <QSettings>
#include <QUrl>

AudioManager& AudioManager::instance()
{
    static AudioManager instance;
    return instance;
}

AudioManager::AudioManager()
    : backgroundMusic(nullptr), musicOutput(nullptr),
    musicVolume(70), sfxVolume(80), masterVolume(100),
    musicMuted(false), sfxMuted(false), masterMuted(false)
{
}

AudioManager::~AudioManager()
{
    saveSettings();

    if (backgroundMusic) {
        backgroundMusic->stop();
        delete backgroundMusic;
    }
    if (musicOutput) {
        delete musicOutput;
    }

    for (auto* player : soundEffects.values()) {
        if (player) {
            player->stop();
            delete player;
        }
    }
    for (auto* output : sfxOutputs.values()) {
        if (output) {
            delete output;
        }
    }
}

void AudioManager::initialize()
{
    loadSettings();

    backgroundMusic = new QMediaPlayer(this);
    musicOutput = new QAudioOutput(this);
    backgroundMusic->setAudioOutput(musicOutput);

    backgroundMusic->setSource(QUrl("qrc:/music/background_music.mp3"));
    backgroundMusic->setLoops(QMediaPlayer::Infinite);

    updateMusicVolume();

    loadSoundEffects();
}

void AudioManager::loadSoundEffects()
{
    QHash<SoundEffect, QString> effectFiles = {
        {SoundEffect::PieceAttacked, "qrc:/music/sfx_attack.wav"},
        {SoundEffect::PieceEvolved, "qrc:/music/sfx_evolve.wav"},
        {SoundEffect::AlertSize, "qrc:/music/sfx_alert_size.wav"},
        {SoundEffect::AlertGeneric, "qrc:/music/sfx_alert.wav"},
        {SoundEffect::AlertVictory, "qrc:/music/sfx_victory.wav"},
        {SoundEffect::GameStart, "qrc:/music/sfx_gamestart.wav"},
        {SoundEffect::RecruitPiece, "qrc:/music/sfx_recruit.wav"},
        {SoundEffect::MovePiece, "qrc:/music/sfx_move.wav"}
    };

    for (auto it = effectFiles.constBegin(); it != effectFiles.constEnd(); ++it) {
        QMediaPlayer* player = new QMediaPlayer(this);
        QAudioOutput* output = new QAudioOutput(this);
        player->setAudioOutput(output);
        player->setSource(QUrl(it.value()));

        soundEffects[it.key()] = player;
        sfxOutputs[it.key()] = output;
    }

    updateSFXVolume();
}

void AudioManager::playBackgroundMusic()
{
    if (backgroundMusic && !musicMuted && !masterMuted) {
        backgroundMusic->play();
    }
}

void AudioManager::stopBackgroundMusic()
{
    if (backgroundMusic) {
        backgroundMusic->stop();
    }
}

void AudioManager::pauseBackgroundMusic()
{
    if (backgroundMusic) {
        backgroundMusic->pause();
    }
}

void AudioManager::playSoundEffect(SoundEffect effect)
{
    if (sfxMuted || masterMuted) {
        return;
    }

    QMediaPlayer* player = soundEffects.value(effect, nullptr);
    if (player) {
        player->setPosition(0);
        player->play();
    }
}

void AudioManager::setMusicVolume(int volume)
{
    musicVolume = qBound(0, volume, 100);
    updateMusicVolume();
    saveSettings();
    emit volumeChanged();
}

void AudioManager::setSFXVolume(int volume)
{
    sfxVolume = qBound(0, volume, 100);
    updateSFXVolume();
    saveSettings();
    emit volumeChanged();
}

void AudioManager::setMasterVolume(int volume)
{
    masterVolume = qBound(0, volume, 100);
    updateMusicVolume();
    updateSFXVolume();
    saveSettings();
    emit volumeChanged();
}

void AudioManager::setMusicMuted(bool muted)
{
    musicMuted = muted;
    if (musicMuted) {
        stopBackgroundMusic();
    } else {
        playBackgroundMusic();
    }
    saveSettings();
    emit volumeChanged();
}

void AudioManager::setSFXMuted(bool muted)
{
    sfxMuted = muted;
    saveSettings();
    emit volumeChanged();
}

void AudioManager::setMasterMuted(bool muted)
{
    masterMuted = muted;
    if (masterMuted) {
        stopBackgroundMusic();
    } else {
        playBackgroundMusic();
    }
    saveSettings();
    emit volumeChanged();
}

void AudioManager::updateMusicVolume()
{
    if (musicOutput) {
        float volume = (musicVolume / 100.0f) * (masterVolume / 100.0f);
        musicOutput->setVolume(volume);
    }
}

void AudioManager::updateSFXVolume()
{
    float volume = (sfxVolume / 100.0f) * (masterVolume / 100.0f);

    for (auto* output : sfxOutputs.values()) {
        if (output) {
            output->setVolume(volume);
        }
    }
}

void AudioManager::saveSettings()
{
    QSettings settings("RindallOfDaick", "AudioSettings");
    settings.setValue("musicVolume", musicVolume);
    settings.setValue("sfxVolume", sfxVolume);
    settings.setValue("masterVolume", masterVolume);
    settings.setValue("musicMuted", musicMuted);
    settings.setValue("sfxMuted", sfxMuted);
    settings.setValue("masterMuted", masterMuted);
}

void AudioManager::loadSettings()
{
    QSettings settings("RindallOfDaick", "AudioSettings");
    musicVolume = settings.value("musicVolume", 30).toInt();
    sfxVolume = settings.value("sfxVolume", 30).toInt();
    masterVolume = settings.value("masterVolume", 30).toInt();
    musicMuted = settings.value("musicMuted", false).toBool();
    sfxMuted = settings.value("sfxMuted", false).toBool();
    masterMuted = settings.value("masterMuted", false).toBool();
}
