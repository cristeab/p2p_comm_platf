#import <AVFoundation/AVAudioSession.h>
#include <QDebug>

void initAudioSession()
{
    AVAudioSession *sess = [AVAudioSession sharedInstance];

    bool err = false;
    if ([sess respondsToSelector:@selector(setCategory:withOptions:error:)]) {
        err = [sess setCategory:AVAudioSessionCategoryPlayAndRecord
                withOptions:AVAudioSessionCategoryOptionAllowBluetooth
                error:nil] != YES;
    } else {
        err = [sess setCategory:AVAudioSessionCategoryPlayAndRecord
                error:nil] != YES;
    }
    if (err) {
        qWarning() << "Failed settting audio session category";
    }

    if ([sess respondsToSelector:@selector(setMode:error:)] &&
            [sess setMode:AVAudioSessionModeVoiceChat error:nil] != YES) {
        qWarning() << "Failed settting audio mode";
    }

    qInfo() << "Audio session init";
}

void enableAudioSession()
{
    initAudioSession();
    AVAudioSession *session = [AVAudioSession sharedInstance];
    if (YES == [session setActive:YES error:nil]) {
        qInfo() << "Audio session enabled";
    } else {
        qWarning() << "Cannot enable audio session";
    }
}

void disableAudioSession()
{
    AVAudioSession *session = [AVAudioSession sharedInstance];
    if (YES == [session setActive:NO error:nil]) {
        qInfo() << "Audio session disabled";
    } else {
        qWarning() << "Cannot disable audio session";
    }
}

void audioOutputSpeaker(bool enabled)
{
    AVAudioSession *session = [AVAudioSession sharedInstance];
    NSError *error = nil;
    [session setCategory:AVAudioSessionCategoryPlayAndRecord error:&error];
    [session setMode:AVAudioSessionModeVoiceChat error:&error];
    if (enabled) { // Enable speaker
        [session overrideOutputAudioPort:AVAudioSessionPortOverrideSpeaker error:&error];
    }
    else { // Disable speaker
        [session overrideOutputAudioPort:AVAudioSessionPortOverrideNone error:&error];
    }
    [session setActive:YES error:&error];
}
