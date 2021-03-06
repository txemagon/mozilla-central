/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <string>

#include "CSFLog.h"

#include "nspr.h"
#include "cc_constants.h"

#include "nricectx.h"
#include "nricemediastream.h"
#include "PeerConnectionImpl.h"
#include "PeerConnectionMedia.h"
#include "AudioConduit.h"
#include "VideoConduit.h"
#include "runnable_utils.h"

#ifdef MOZILLA_INTERNAL_API
#include "MediaStreamList.h"
#include "nsIScriptGlobalObject.h"
#include "jsapi.h"
#endif

using namespace mozilla;

namespace sipcc {

static const char* logTag = "PeerConnectionMedia";
static const mozilla::TrackID TRACK_AUDIO = 0;
static const mozilla::TrackID TRACK_VIDEO = 1;

/* If the ExpectAudio hint is on we will add a track at the default first
 * audio track ID (0)
 * FIX - Do we need to iterate over the tracks instead of taking these hints?
 */
void
LocalSourceStreamInfo::ExpectAudio(const mozilla::TrackID aID)
{
  mAudioTracks.AppendElement(aID);
}

// If the ExpectVideo hint is on we will add a track at the default first
// video track ID (1).
void
LocalSourceStreamInfo::ExpectVideo(const mozilla::TrackID aID)
{
  mVideoTracks.AppendElement(aID);
}

unsigned
LocalSourceStreamInfo::AudioTrackCount()
{
  return mAudioTracks.Length();
}

unsigned
LocalSourceStreamInfo::VideoTrackCount()
{
  return mVideoTracks.Length();
}

PeerConnectionImpl* PeerConnectionImpl::CreatePeerConnection()
{
  PeerConnectionImpl *pc = new PeerConnectionImpl();

  CSFLogDebug(logTag, "Created PeerConnection: %p", pc);

  return pc;
}


nsresult PeerConnectionMedia::Init(const std::vector<NrIceStunServer>& stun_servers)
{
  // TODO(ekr@rtfm.com): need some way to set not offerer later
  // Looks like a bug in the NrIceCtx API.
  mIceCtx = NrIceCtx::Create("PC:" + mParent->GetHandle(), true);
  if(!mIceCtx) {
    CSFLogError(logTag, "%s: Failed to create Ice Context", __FUNCTION__);
    return NS_ERROR_FAILURE;
  }
  nsresult rv = mIceCtx->SetStunServers(stun_servers);
  if (NS_FAILED(rv)) {
    return rv;
  }
  mIceCtx->SignalGatheringCompleted.connect(this,
                                            &PeerConnectionMedia::IceGatheringCompleted);
  mIceCtx->SignalCompleted.connect(this,
                                   &PeerConnectionMedia::IceCompleted);

  // Create three streams to start with.
  // One each for audio, video and DataChannel
  // TODO: this will be re-visited
  RefPtr<NrIceMediaStream> audioStream = mIceCtx->CreateStream("stream1", 2);
  RefPtr<NrIceMediaStream> videoStream = mIceCtx->CreateStream("stream2", 2);
  RefPtr<NrIceMediaStream> dcStream = mIceCtx->CreateStream("stream3", 2);

  if (!audioStream) {
    CSFLogError(logTag, "%s: audio stream is NULL", __FUNCTION__);
    return NS_ERROR_FAILURE;
  } else {
    mIceStreams.push_back(audioStream);
  }

  if (!videoStream) {
    CSFLogError(logTag, "%s: video stream is NULL", __FUNCTION__);
    return NS_ERROR_FAILURE;
  } else {
    mIceStreams.push_back(videoStream);
  }

  if (!dcStream) {
    CSFLogError(logTag, "%s: datachannel stream is NULL", __FUNCTION__);
    return NS_ERROR_FAILURE;
  } else {
    mIceStreams.push_back(dcStream);
  }

  // TODO(ekr@rtfm.com): This is not connected to the PCCimpl.
  // Will need to do that later.
  for (std::size_t i=0; i<mIceStreams.size(); i++) {
    mIceStreams[i]->SignalReady.connect(this, &PeerConnectionMedia::IceStreamReady);
  }

  // Start gathering
  nsresult res;
  mIceCtx->thread()->Dispatch(WrapRunnableRet(
    mIceCtx, &NrIceCtx::StartGathering, &res), NS_DISPATCH_SYNC
  );

  if (NS_FAILED(res)) {
    CSFLogError(logTag, "%s: StartGathering failed: %u",
      __FUNCTION__, static_cast<uint32_t>(res));
    return res;
  }

  return NS_OK;
}

nsresult
PeerConnectionMedia::AddStream(nsIDOMMediaStream* aMediaStream, uint32_t *stream_id)
{
  if (!aMediaStream) {
    CSFLogError(logTag, "%s - aMediaStream is NULL", __FUNCTION__);
    return NS_ERROR_FAILURE;
  }

  DOMMediaStream* stream = static_cast<DOMMediaStream*>(aMediaStream);

  CSFLogDebug(logTag, "%s: MediaStream: %p",
    __FUNCTION__, aMediaStream);

  // Adding tracks here based on nsDOMMediaStream expectation settings
  uint32_t hints = stream->GetHintContents();

  if (!(hints & (DOMMediaStream::HINT_CONTENTS_AUDIO |
        DOMMediaStream::HINT_CONTENTS_VIDEO))) {
    CSFLogDebug(logTag, "Empty Stream !!");
    return NS_OK;
  }

  // Now see if we already have a stream of this type, since we only
  // allow one of each.
  // TODO(ekr@rtfm.com): remove this when multiple of each stream
  // is allowed
  mozilla::MutexAutoLock lock(mLocalSourceStreamsLock);
  for (uint32_t u = 0; u < mLocalSourceStreams.Length(); u++) {
    nsRefPtr<LocalSourceStreamInfo> localSourceStream = mLocalSourceStreams[u];

    if (localSourceStream->GetMediaStream()->GetHintContents() & hints) {
      CSFLogError(logTag, "Only one stream of any given type allowed");
      return NS_ERROR_FAILURE;
    }
  }

  // OK, we're good to add
  nsRefPtr<LocalSourceStreamInfo> localSourceStream =
    new LocalSourceStreamInfo(stream);
  *stream_id = mLocalSourceStreams.Length();

  if (hints & DOMMediaStream::HINT_CONTENTS_AUDIO) {
    localSourceStream->ExpectAudio(TRACK_AUDIO);
  }

  if (hints & DOMMediaStream::HINT_CONTENTS_VIDEO) {
    localSourceStream->ExpectVideo(TRACK_VIDEO);
  }

  mLocalSourceStreams.AppendElement(localSourceStream);

  return NS_OK;
}

nsresult
PeerConnectionMedia::RemoveStream(nsIDOMMediaStream* aMediaStream, uint32_t *stream_id)
{
  MOZ_ASSERT(aMediaStream);

  DOMMediaStream* stream = static_cast<DOMMediaStream*>(aMediaStream);

  CSFLogDebug(logTag, "%s: MediaStream: %p",
    __FUNCTION__, aMediaStream);

  mozilla::MutexAutoLock lock(mLocalSourceStreamsLock);
  for (uint32_t u = 0; u < mLocalSourceStreams.Length(); u++) {
    nsRefPtr<LocalSourceStreamInfo> localSourceStream = mLocalSourceStreams[u];
    if (localSourceStream->GetMediaStream() == stream) {
      *stream_id = u;
      return NS_OK;
    }
  }

  return NS_ERROR_ILLEGAL_VALUE;
}

void
PeerConnectionMedia::SelfDestruct()
{
   CSFLogDebug(logTag, "%s: Disconnecting media streams from PC", __FUNCTION__);

   DisconnectMediaStreams();

   CSFLogDebug(logTag, "%s: Disconnecting transport", __FUNCTION__);
   // Shutdown the transport.
   RUN_ON_THREAD(mParent->GetSTSThread(), WrapRunnable(
       this, &PeerConnectionMedia::ShutdownMediaTransport), NS_DISPATCH_SYNC);

  CSFLogDebug(logTag, "%s: Media shut down", __FUNCTION__);

  // This should destroy the object.
  this->Release();
}

void
PeerConnectionMedia::DisconnectMediaStreams()
{
  for (uint32_t i=0; i < mLocalSourceStreams.Length(); ++i) {
    mLocalSourceStreams[i]->Detach();
  }

  for (uint32_t i=0; i < mRemoteSourceStreams.Length(); ++i) {
    mRemoteSourceStreams[i]->Detach();
  }

  mLocalSourceStreams.Clear();
  mRemoteSourceStreams.Clear();
}

void
PeerConnectionMedia::ShutdownMediaTransport()
{
  disconnect_all();
  mTransportFlows.clear();
  mIceStreams.clear();
  mIceCtx = NULL;
}

LocalSourceStreamInfo*
PeerConnectionMedia::GetLocalStream(int aIndex)
{
  if(aIndex < 0 || aIndex >= (int) mLocalSourceStreams.Length()) {
    return NULL;
  }

  MOZ_ASSERT(mLocalSourceStreams[aIndex]);
  return mLocalSourceStreams[aIndex];
}

RemoteSourceStreamInfo*
PeerConnectionMedia::GetRemoteStream(int aIndex)
{
  if(aIndex < 0 || aIndex >= (int) mRemoteSourceStreams.Length()) {
    return NULL;
  }

  MOZ_ASSERT(mRemoteSourceStreams[aIndex]);
  return mRemoteSourceStreams[aIndex];
}

nsresult
PeerConnectionMedia::AddRemoteStream(nsRefPtr<RemoteSourceStreamInfo> aInfo,
  int *aIndex)
{
  MOZ_ASSERT(aIndex);

  *aIndex = mRemoteSourceStreams.Length();

  mRemoteSourceStreams.AppendElement(aInfo);

  return NS_OK;
}


void
PeerConnectionMedia::IceGatheringCompleted(NrIceCtx *aCtx)
{
  MOZ_ASSERT(aCtx);
  SignalIceGatheringCompleted(aCtx);
}

void
PeerConnectionMedia::IceCompleted(NrIceCtx *aCtx)
{
  MOZ_ASSERT(aCtx);
  SignalIceCompleted(aCtx);
}

void
PeerConnectionMedia::IceStreamReady(NrIceMediaStream *aStream)
{
  MOZ_ASSERT(aStream);

  CSFLogDebug(logTag, "%s: %s", __FUNCTION__, aStream->name().c_str());
}


void
LocalSourceStreamInfo::StorePipeline(int aTrack,
  mozilla::RefPtr<mozilla::MediaPipeline> aPipeline)
{
  MOZ_ASSERT(mPipelines.find(aTrack) == mPipelines.end());
  if (mPipelines.find(aTrack) != mPipelines.end()) {
    CSFLogError(logTag, "%s: Storing duplicate track", __FUNCTION__);
    return;
  }
  //TODO: Revisit once we start supporting multiple streams or multiple tracks
  // of same type
  mPipelines[aTrack] = aPipeline;
}

void
RemoteSourceStreamInfo::StorePipeline(int aTrack,
                                      bool aIsVideo,
                                      mozilla::RefPtr<mozilla::MediaPipeline> aPipeline)
{
  MOZ_ASSERT(mPipelines.find(aTrack) == mPipelines.end());
  if (mPipelines.find(aTrack) != mPipelines.end()) {
    CSFLogError(logTag, "%s: Request to store duplicate track %d", __FUNCTION__, aTrack);
    return;
  }
  CSFLogDebug(logTag, "%s track %d %s = %p", __FUNCTION__, aTrack, aIsVideo ? "video" : "audio",
              aPipeline.get());
  // See if we have both audio and video here, and if so cross the streams and sync them
  // XXX Needs to be adjusted when we support multiple streams of the same type
  for (std::map<int, bool>::iterator it = mTypes.begin(); it != mTypes.end(); ++it) {
    if (it->second != aIsVideo) {
      // Ok, we have one video, one non-video - cross the streams!
      mozilla::WebrtcAudioConduit *audio_conduit = static_cast<mozilla::WebrtcAudioConduit*>
                                                   (aIsVideo ?
                                                    mPipelines[it->first]->Conduit() :
                                                    aPipeline->Conduit());
      mozilla::WebrtcVideoConduit *video_conduit = static_cast<mozilla::WebrtcVideoConduit*>
                                                   (aIsVideo ?
                                                    aPipeline->Conduit() :
                                                    mPipelines[it->first]->Conduit());
      video_conduit->SyncTo(audio_conduit);
      CSFLogDebug(logTag, "Syncing %p to %p, %d to %d", video_conduit, audio_conduit,
                  aTrack, it->first);
    }
  }
  //TODO: Revisit once we start supporting multiple streams or multiple tracks
  // of same type
  mPipelines[aTrack] = aPipeline;
  //TODO: move to attribute on Pipeline
  mTypes[aTrack] = aIsVideo;
}


}  // namespace sipcc
