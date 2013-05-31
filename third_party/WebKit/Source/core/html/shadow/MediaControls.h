/*
 * Copyright (C) 2011, 2012 Apple Inc. All rights reserved.
 * Copyright (C) 2011, 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MediaControls_h
#define MediaControls_h

#include "HTMLNames.h"
#include "core/dom/MouseEvent.h"
#include "core/dom/Text.h"
#include "core/html/HTMLDivElement.h"
#include "core/html/HTMLMediaElement.h"
#include "core/html/shadow/MediaControlElements.h"
#include "core/page/Chrome.h"
#include "core/page/Page.h"
#include "core/rendering/RenderTheme.h"
#include <wtf/RefPtr.h>

#include "core/html/track/TextTrackCue.h"

namespace WebCore {

class Document;
class Event;
class Page;
class MediaPlayer;

class RenderBox;
class RenderMedia;

// An abstract class with the media control elements that all ports support.
class MediaControls : public HTMLDivElement {
  public:
    virtual ~MediaControls() {}

    // This function is to be implemented in your port-specific media
    // controls implementation since it will return a child instance.
    static PassRefPtr<MediaControls> create(Document*);

    virtual void setMediaController(MediaControllerInterface*);

    virtual void reset();
    virtual void reportedError();
    virtual void loadedMetadata();

    virtual void show();
    virtual void hide();
    virtual void makeOpaque();
    virtual void makeTransparent();
    virtual bool shouldHideControls();

    virtual void bufferingProgressed();
    virtual void playbackStarted();
    virtual void playbackProgressed();
    virtual void playbackStopped();

    virtual void updateStatusDisplay() { };
    virtual void updateCurrentTimeDisplay() = 0;
    virtual void showVolumeSlider();

    virtual void changedMute();
    virtual void changedVolume();

    virtual void changedClosedCaptionsVisibility();
    virtual void refreshClosedCaptionsButtonVisibility();
    virtual void closedCaptionTracksChanged();

    virtual void enteredFullscreen();
    virtual void exitedFullscreen();

    virtual bool willRespondToMouseMoveEvents() OVERRIDE { return true; }

    virtual void hideFullscreenControlsTimerFired(Timer<MediaControls>*);
    virtual void startHideFullscreenControlsTimer();
    virtual void stopHideFullscreenControlsTimer();

    virtual void createTextTrackDisplay();
    virtual void showTextTrackDisplay();
    virtual void hideTextTrackDisplay();
    virtual void updateTextTrackDisplay();
    virtual void textTrackPreferencesChanged();

protected:
    explicit MediaControls(Document*);

    virtual void defaultEventHandler(Event*);

    virtual bool containsRelatedTarget(Event*);

    MediaControllerInterface* m_mediaController;

    // Container for the media control elements.
    MediaControlPanelElement* m_panel;

    // Container for the text track cues.
    MediaControlTextTrackContainerElement* m_textDisplayContainer;

    // Media control elements.
    MediaControlPlayButtonElement* m_playButton;
    MediaControlCurrentTimeDisplayElement* m_currentTimeDisplay;
    MediaControlTimelineElement* m_timeline;
    MediaControlPanelMuteButtonElement* m_panelMuteButton;
    MediaControlPanelVolumeSliderElement* m_volumeSlider;
    MediaControlToggleClosedCaptionsButtonElement* m_toggleClosedCaptionsButton;
    MediaControlFullscreenButtonElement* m_fullScreenButton;

    Timer<MediaControls> m_hideFullscreenControlsTimer;
    bool m_isFullscreen;
    bool m_isMouseOverControls;

private:
    virtual bool isMediaControls() const { return true; }

    virtual const AtomicString& shadowPseudoId() const;
};

inline MediaControls* toMediaControls(Node* node)
{
    ASSERT_WITH_SECURITY_IMPLICATION(!node || node->isMediaControls());
    return static_cast<MediaControls*>(node);
}

// This will catch anyone doing an unneccessary cast.
void toMediaControls(const MediaControls*);

}

#endif
