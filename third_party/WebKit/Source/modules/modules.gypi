{
  'variables': {
    'modules_include_dirs': [
      'battery',
      'donottrack',
      'filesystem',
      'gamepad',
      'geolocation',
      'indexeddb',
      'indexeddb/chromium',
      'inputmethod',
      'mediasource',
      'mediastream',
      'navigatorcontentutils',
      'notifications',
      'quota',
      'speech',
      'webaudio',
      'webdatabase',
      'webdatabase/chromium',
      'webmidi',
      'websockets',
    ],
    'modules_idl_files': [
      'battery/BatteryManager.idl',
      'battery/NavigatorBattery.idl',
      'donottrack/NavigatorDoNotTrack.idl',
      'filesystem/DOMFileSystem.idl',
      'filesystem/DOMFileSystemSync.idl',
      'filesystem/DOMWindowFileSystem.idl',
      'filesystem/DataTransferItemFileSystem.idl',
      'filesystem/DirectoryEntry.idl',
      'filesystem/DirectoryEntrySync.idl',
      'filesystem/DirectoryReader.idl',
      'filesystem/DirectoryReaderSync.idl',
      'filesystem/EntriesCallback.idl',
      'filesystem/Entry.idl',
      'filesystem/EntryArray.idl',
      'filesystem/EntryArraySync.idl',
      'filesystem/EntryCallback.idl',
      'filesystem/EntrySync.idl',
      'filesystem/ErrorCallback.idl',
      'filesystem/FileCallback.idl',
      'filesystem/FileEntry.idl',
      'filesystem/FileEntrySync.idl',
      'filesystem/FileSystemCallback.idl',
      'filesystem/FileWriter.idl',
      'filesystem/FileWriterCallback.idl',
      'filesystem/FileWriterSync.idl',
      'filesystem/HTMLInputElementFileSystem.idl',
      'filesystem/Metadata.idl',
      'filesystem/MetadataCallback.idl',
      'filesystem/WorkerContextFileSystem.idl',
      'gamepad/Gamepad.idl',
      'gamepad/GamepadList.idl',
      'gamepad/NavigatorGamepad.idl',
      'geolocation/Coordinates.idl',
      'geolocation/Geolocation.idl',
      'geolocation/Geoposition.idl',
      'geolocation/NavigatorGeolocation.idl',
      'geolocation/PositionCallback.idl',
      'geolocation/PositionError.idl',
      'geolocation/PositionErrorCallback.idl',
      'indexeddb/DOMWindowIndexedDatabase.idl',
      'indexeddb/IDBAny.idl',
      'indexeddb/IDBCursor.idl',
      'indexeddb/IDBCursorWithValue.idl',
      'indexeddb/IDBDatabase.idl',
      'indexeddb/IDBFactory.idl',
      'indexeddb/IDBIndex.idl',
      'indexeddb/IDBKeyRange.idl',
      'indexeddb/IDBObjectStore.idl',
      'indexeddb/IDBOpenDBRequest.idl',
      'indexeddb/IDBRequest.idl',
      'indexeddb/IDBTransaction.idl',
      'indexeddb/IDBVersionChangeEvent.idl',
      'indexeddb/WorkerContextIndexedDatabase.idl',
      'inputmethod/Composition.idl',
      'inputmethod/InputMethodContext.idl',
      'mediasource/MediaSource.idl',
      'mediasource/SourceBuffer.idl',
      'mediasource/SourceBufferList.idl',
      'mediastream/DOMWindowMediaStream.idl',
      'mediastream/MediaStream.idl',
      'mediastream/MediaStreamEvent.idl',
      'mediastream/MediaStreamTrack.idl',
      'mediastream/MediaStreamTrackEvent.idl',
      'mediastream/NavigatorMediaStream.idl',
      'mediastream/NavigatorUserMediaError.idl',
      'mediastream/NavigatorUserMediaErrorCallback.idl',
      'mediastream/NavigatorUserMediaSuccessCallback.idl',
      'mediastream/RTCDTMFSender.idl',
      'mediastream/RTCDTMFToneChangeEvent.idl',
      'mediastream/RTCDataChannel.idl',
      'mediastream/RTCDataChannelEvent.idl',
      'mediastream/RTCErrorCallback.idl',
      'mediastream/RTCIceCandidate.idl',
      'mediastream/RTCIceCandidateEvent.idl',
      'mediastream/RTCPeerConnection.idl',
      'mediastream/RTCSessionDescription.idl',
      'mediastream/RTCSessionDescriptionCallback.idl',
      'mediastream/RTCStatsCallback.idl',
      'mediastream/RTCStatsReport.idl',
      'mediastream/RTCStatsResponse.idl',
      'navigatorcontentutils/NavigatorContentUtils.idl',
      'notifications/DOMWindowNotifications.idl',
      'notifications/Notification.idl',
      'notifications/NotificationCenter.idl',
      'notifications/NotificationPermissionCallback.idl',
      'notifications/WorkerContextNotifications.idl',
      'quota/DOMWindowQuota.idl',
      'quota/NavigatorStorageQuota.idl',
      'quota/StorageErrorCallback.idl',
      'quota/StorageInfo.idl',
      'quota/StorageQuota.idl',
      'quota/StorageQuotaCallback.idl',
      'quota/StorageUsageCallback.idl',
      'quota/WorkerNavigatorStorageQuota.idl',
      'speech/DOMWindowSpeech.idl',
      'speech/DOMWindowSpeechSynthesis.idl',
      'speech/SpeechGrammar.idl',
      'speech/SpeechGrammarList.idl',
      'speech/SpeechRecognition.idl',
      'speech/SpeechRecognitionAlternative.idl',
      'speech/SpeechRecognitionError.idl',
      'speech/SpeechRecognitionEvent.idl',
      'speech/SpeechRecognitionResult.idl',
      'speech/SpeechRecognitionResultList.idl',
      'speech/SpeechSynthesis.idl',
      'speech/SpeechSynthesisEvent.idl',
      'speech/SpeechSynthesisUtterance.idl',
      'speech/SpeechSynthesisVoice.idl',
      'webaudio/AudioBuffer.idl',
      'webaudio/AudioBufferCallback.idl',
      'webaudio/AudioBufferSourceNode.idl',
      'webaudio/ChannelMergerNode.idl',
      'webaudio/ChannelSplitterNode.idl',
      'webaudio/AudioContext.idl',
      'webaudio/AudioDestinationNode.idl',
      'webaudio/GainNode.idl',
      'webaudio/AudioListener.idl',
      'webaudio/AudioNode.idl',
      'webaudio/PannerNode.idl',
      'webaudio/AudioParam.idl',
      'webaudio/AudioProcessingEvent.idl',
      'webaudio/AudioSourceNode.idl',
      'webaudio/BiquadFilterNode.idl',
      'webaudio/ConvolverNode.idl',
      'webaudio/DelayNode.idl',
      'webaudio/DOMWindowWebAudio.idl',
      'webaudio/DynamicsCompressorNode.idl',
      'webaudio/ScriptProcessorNode.idl',
      'webaudio/MediaElementAudioSourceNode.idl',
      'webaudio/MediaStreamAudioDestinationNode.idl',
      'webaudio/MediaStreamAudioSourceNode.idl',
      'webaudio/OfflineAudioCompletionEvent.idl',
      'webaudio/OfflineAudioContext.idl',
      'webaudio/OscillatorNode.idl',
      'webaudio/AnalyserNode.idl',
      'webaudio/WaveShaperNode.idl',
      'webaudio/WaveTable.idl',
      'webdatabase/DOMWindowWebDatabase.idl',
      'webdatabase/Database.idl',
      'webdatabase/DatabaseCallback.idl',
      'webdatabase/DatabaseSync.idl',
      'webdatabase/SQLError.idl',
      'webdatabase/SQLException.idl',
      'webdatabase/SQLResultSet.idl',
      'webdatabase/SQLResultSetRowList.idl',
      'webdatabase/SQLStatementCallback.idl',
      'webdatabase/SQLStatementErrorCallback.idl',
      'webdatabase/SQLTransaction.idl',
      'webdatabase/SQLTransactionCallback.idl',
      'webdatabase/SQLTransactionErrorCallback.idl',
      'webdatabase/SQLTransactionSync.idl',
      'webdatabase/SQLTransactionSyncCallback.idl',
      'webdatabase/WorkerContextWebDatabase.idl',
      'webmidi/DOMWindowWebMIDI.idl',
      'webmidi/MIDIErrorCallback.idl',
      'webmidi/MIDIMessageEvent.idl',
      'webmidi/MIDIPort.idl',
      'websockets/CloseEvent.idl',
      'websockets/DOMWindowWebSocket.idl',
      'websockets/WebSocket.idl',
      'websockets/WorkerContextWebSocket.idl',
    ],
    'modules_files': [
      'battery/BatteryClient.h',
      'battery/BatteryController.cpp',
      'battery/BatteryController.h',
      'battery/BatteryManager.cpp',
      'battery/BatteryManager.h',
      'battery/BatteryStatus.cpp',
      'battery/BatteryStatus.h',
      'battery/NavigatorBattery.cpp',
      'battery/NavigatorBattery.h',
      'donottrack/NavigatorDoNotTrack.cpp',
      'donottrack/NavigatorDoNotTrack.h',
      'filesystem/DOMFilePath.cpp',
      'filesystem/DOMFilePath.h',
      'filesystem/DOMFileSystem.cpp',
      'filesystem/DOMFileSystem.h',
      'filesystem/DOMFileSystemBase.cpp',
      'filesystem/DOMFileSystemBase.h',
      'filesystem/DOMFileSystemSync.cpp',
      'filesystem/DOMFileSystemSync.h',
      'filesystem/DOMWindowFileSystem.cpp',
      'filesystem/DOMWindowFileSystem.h',
      'filesystem/DataTransferItemFileSystem.cpp',
      'filesystem/DataTransferItemFileSystem.h',
      'filesystem/DirectoryEntry.cpp',
      'filesystem/DirectoryEntry.h',
      'filesystem/DirectoryEntrySync.cpp',
      'filesystem/DirectoryEntrySync.h',
      'filesystem/DirectoryReader.cpp',
      'filesystem/DirectoryReader.h',
      'filesystem/DirectoryReaderBase.h',
      'filesystem/DirectoryReaderSync.cpp',
      'filesystem/DirectoryReaderSync.h',
      'filesystem/DraggedIsolatedFileSystem.cpp',
      'filesystem/DraggedIsolatedFileSystem.h',
      'filesystem/EntriesCallback.h',
      'filesystem/Entry.cpp',
      'filesystem/Entry.h',
      'filesystem/EntryArray.cpp',
      'filesystem/EntryArray.h',
      'filesystem/EntryArraySync.cpp',
      'filesystem/EntryArraySync.h',
      'filesystem/EntryBase.cpp',
      'filesystem/EntryBase.h',
      'filesystem/EntryCallback.h',
      'filesystem/EntrySync.cpp',
      'filesystem/EntrySync.h',
      'filesystem/ErrorCallback.h',
      'filesystem/FileCallback.h',
      'filesystem/FileEntry.cpp',
      'filesystem/FileEntry.h',
      'filesystem/FileEntrySync.cpp',
      'filesystem/FileEntrySync.h',
      'filesystem/FileSystemCallback.h',
      'filesystem/FileSystemCallbacks.cpp',
      'filesystem/FileSystemCallbacks.h',
      'filesystem/FileSystemFlags.h',
      'filesystem/FileSystemType.h',
      'filesystem/FileWriter.cpp',
      'filesystem/FileWriter.h',
      'filesystem/FileWriterBase.cpp',
      'filesystem/FileWriterBase.h',
      'filesystem/FileWriterBaseCallback.h',
      'filesystem/FileWriterCallback.h',
      'filesystem/FileWriterSync.cpp',
      'filesystem/FileWriterSync.h',
      'filesystem/HTMLInputElementFileSystem.cpp',
      'filesystem/HTMLInputElementFileSystem.h',
      'filesystem/LocalFileSystem.h',
      'filesystem/Metadata.h',
      'filesystem/MetadataCallback.h',
      'filesystem/SyncCallbackHelper.h',
      'filesystem/WorkerContextFileSystem.cpp',
      'filesystem/WorkerContextFileSystem.h',
      'gamepad/Gamepad.cpp',
      'gamepad/Gamepad.h',
      'gamepad/GamepadList.cpp',
      'gamepad/GamepadList.h',
      'gamepad/NavigatorGamepad.cpp',
      'gamepad/NavigatorGamepad.h',
      'geolocation/Coordinates.cpp',
      'geolocation/Geolocation.cpp',
      'geolocation/GeolocationController.cpp',
      'geolocation/NavigatorGeolocation.cpp',
      'geolocation/NavigatorGeolocation.h',
      'indexeddb/DOMWindowIndexedDatabase.cpp',
      'indexeddb/DOMWindowIndexedDatabase.h',
      'indexeddb/IDBAny.cpp',
      'indexeddb/IDBAny.h',
      'indexeddb/IDBBackingStore.cpp',
      'indexeddb/IDBBackingStore.h',
      'indexeddb/IDBCallbacks.h',
      'indexeddb/IDBCursor.cpp',
      'indexeddb/IDBCursor.h',
      'indexeddb/IDBCursorBackendImpl.cpp',
      'indexeddb/IDBCursorBackendImpl.h',
      'indexeddb/IDBCursorBackendInterface.h',
      'indexeddb/IDBCursorWithValue.cpp',
      'indexeddb/IDBCursorWithValue.h',
      'indexeddb/IDBDatabase.cpp',
      'indexeddb/IDBDatabase.h',
      'indexeddb/IDBDatabaseBackendImpl.cpp',
      'indexeddb/IDBDatabaseBackendImpl.h',
      'indexeddb/IDBDatabaseBackendInterface.h',
      'indexeddb/IDBDatabaseCallbacks.h',
      'indexeddb/IDBDatabaseCallbacks.h',
      'indexeddb/IDBDatabaseCallbacksImpl.cpp',
      'indexeddb/IDBDatabaseCallbacksImpl.h',
      'indexeddb/IDBDatabaseError.h',
      'indexeddb/IDBDatabaseException.cpp',
      'indexeddb/IDBDatabaseException.h',
      'indexeddb/IDBEventDispatcher.cpp',
      'indexeddb/IDBEventDispatcher.h',
      'indexeddb/IDBFactory.cpp',
      'indexeddb/IDBFactory.h',
      'indexeddb/IDBFactoryBackendImpl.cpp',
      'indexeddb/IDBFactoryBackendImpl.h',
      'indexeddb/IDBFactoryBackendInterface.h',
      'indexeddb/IDBHistograms.h',
      'indexeddb/IDBIndex.cpp',
      'indexeddb/IDBIndex.h',
      'indexeddb/IDBKey.cpp',
      'indexeddb/IDBKey.h',
      'indexeddb/IDBKeyPath.cpp',
      'indexeddb/IDBKeyPath.h',
      'indexeddb/IDBKeyRange.cpp',
      'indexeddb/IDBKeyRange.h',
      'indexeddb/IDBLevelDBCoding.cpp',
      'indexeddb/IDBLevelDBCoding.h',
      'indexeddb/IDBMetadata.h',
      'indexeddb/IDBObjectStore.cpp',
      'indexeddb/IDBObjectStore.h',
      'indexeddb/IDBObjectStoreBackendImpl.cpp',
      'indexeddb/IDBObjectStoreBackendImpl.h',
      'indexeddb/IDBOpenDBRequest.cpp',
      'indexeddb/IDBOpenDBRequest.h',
      'indexeddb/IDBPendingTransactionMonitor.cpp',
      'indexeddb/IDBPendingTransactionMonitor.h',
      'indexeddb/IDBRequest.cpp',
      'indexeddb/IDBRequest.h',
      'indexeddb/IDBTracing.h',
      'indexeddb/IDBTransaction.cpp',
      'indexeddb/IDBTransaction.h',
      'indexeddb/IDBTransactionBackendImpl.cpp',
      'indexeddb/IDBTransactionBackendImpl.h',
      'indexeddb/IDBTransactionCoordinator.cpp',
      'indexeddb/IDBTransactionCoordinator.h',
      'indexeddb/IDBVersionChangeEvent.cpp',
      'indexeddb/IDBVersionChangeEvent.h',
      'indexeddb/IndexedDB.h',
      'indexeddb/PageGroupIndexedDatabase.cpp',
      'indexeddb/PageGroupIndexedDatabase.h',
      'indexeddb/WorkerContextIndexedDatabase.cpp',
      'indexeddb/WorkerContextIndexedDatabase.h',
      'indexeddb/chromium/IDBFactoryBackendInterfaceChromium.cpp',
      'indexeddb/chromium/IDBFactoryBackendInterfaceChromium.h',
      'inputmethod/Composition.cpp',
      'inputmethod/Composition.h',
      'inputmethod/InputMethodContext.cpp',
      'inputmethod/InputMethodContext.h',
      'mediasource/MediaSource.cpp',
      'mediasource/MediaSource.h',
      'mediasource/MediaSourceRegistry.cpp',
      'mediasource/MediaSourceRegistry.h',
      'mediasource/SourceBuffer.cpp',
      'mediasource/SourceBuffer.h',
      'mediasource/SourceBufferList.cpp',
      'mediasource/SourceBufferList.h',
      'mediastream/MediaConstraintsImpl.cpp',
      'mediastream/MediaConstraintsImpl.h',
      'mediastream/MediaStream.cpp',
      'mediastream/MediaStream.h',
      'mediastream/MediaStreamEvent.cpp',
      'mediastream/MediaStreamEvent.h',
      'mediastream/MediaStreamRegistry.cpp',
      'mediastream/MediaStreamRegistry.h',
      'mediastream/MediaStreamTrack.cpp',
      'mediastream/MediaStreamTrack.h',
      'mediastream/MediaStreamTrackEvent.cpp',
      'mediastream/MediaStreamTrackEvent.h',
      'mediastream/NavigatorMediaStream.cpp',
      'mediastream/NavigatorMediaStream.h',
      'mediastream/NavigatorUserMediaError.h',
      'mediastream/NavigatorUserMediaErrorCallback.h',
      'mediastream/NavigatorUserMediaSuccessCallback.h',
      'mediastream/RTCDTMFSender.cpp',
      'mediastream/RTCDTMFSender.h',
      'mediastream/RTCDTMFToneChangeEvent.cpp',
      'mediastream/RTCDTMFToneChangeEvent.h',
      'mediastream/RTCDataChannel.cpp',
      'mediastream/RTCDataChannel.h',
      'mediastream/RTCDataChannelEvent.cpp',
      'mediastream/RTCDataChannelEvent.h',
      'mediastream/RTCErrorCallback.h',
      'mediastream/RTCIceCandidate.cpp',
      'mediastream/RTCIceCandidate.h',
      'mediastream/RTCIceCandidateEvent.cpp',
      'mediastream/RTCIceCandidateEvent.h',
      'mediastream/RTCPeerConnection.cpp',
      'mediastream/RTCPeerConnection.h',
      'mediastream/RTCSessionDescription.cpp',
      'mediastream/RTCSessionDescription.h',
      'mediastream/RTCSessionDescriptionCallback.h',
      'mediastream/RTCSessionDescriptionRequestImpl.cpp',
      'mediastream/RTCSessionDescriptionRequestImpl.h',
      'mediastream/RTCStatsReport.cpp',
      'mediastream/RTCStatsReport.h',
      'mediastream/RTCStatsRequestImpl.cpp',
      'mediastream/RTCStatsRequestImpl.h',
      'mediastream/RTCStatsResponse.cpp',
      'mediastream/RTCStatsResponse.h',
      'mediastream/RTCVoidRequestImpl.cpp',
      'mediastream/RTCVoidRequestImpl.h',
      'mediastream/UserMediaClient.h',
      'mediastream/UserMediaController.cpp',
      'mediastream/UserMediaController.h',
      'mediastream/UserMediaRequest.cpp',
      'mediastream/UserMediaRequest.h',
      'navigatorcontentutils/NavigatorContentUtils.cpp',
      'navigatorcontentutils/NavigatorContentUtils.h',
      'navigatorcontentutils/NavigatorContentUtilsClient.h',
      'notifications/DOMWindowNotifications.cpp',
      'notifications/DOMWindowNotifications.h',
      'notifications/Notification.cpp',
      'notifications/Notification.h',
      'notifications/NotificationCenter.cpp',
      'notifications/NotificationCenter.h',
      'notifications/NotificationController.cpp',
      'notifications/NotificationController.h',
      'notifications/NotificationPermissionCallback.h',
      'notifications/WorkerContextNotifications.cpp',
      'notifications/WorkerContextNotifications.h',
      'quota/DOMWindowQuota.cpp',
      'quota/DOMWindowQuota.h',
      'quota/NavigatorStorageQuota.cpp',
      'quota/NavigatorStorageQuota.h',
      'quota/StorageInfo.cpp',
      'quota/StorageInfo.h',
      'quota/StorageErrorCallback.cpp',
      'quota/StorageErrorCallback.h',
      'quota/StorageQuota.cpp',
      'quota/StorageQuota.h',
      'quota/StorageQuotaCallback.h',
      'quota/StorageUsageCallback.h',
      'quota/WorkerNavigatorStorageQuota.cpp',
      'quota/WorkerNavigatorStorageQuota.h',
      'speech/DOMWindowSpeechSynthesis.cpp',
      'speech/DOMWindowSpeechSynthesis.h',
      'speech/SpeechGrammar.cpp',
      'speech/SpeechGrammar.h',
      'speech/SpeechGrammarList.cpp',
      'speech/SpeechGrammarList.h',
      'speech/SpeechRecognition.cpp',
      'speech/SpeechRecognition.h',
      'speech/SpeechRecognitionAlternative.cpp',
      'speech/SpeechRecognitionAlternative.h',
      'speech/SpeechRecognitionClient.h',
      'speech/SpeechRecognitionController.cpp',
      'speech/SpeechRecognitionController.h',
      'speech/SpeechRecognitionError.cpp',
      'speech/SpeechRecognitionError.h',
      'speech/SpeechRecognitionEvent.cpp',
      'speech/SpeechRecognitionEvent.h',
      'speech/SpeechRecognitionResult.cpp',
      'speech/SpeechRecognitionResult.h',
      'speech/SpeechRecognitionResultList.cpp',
      'speech/SpeechRecognitionResultList.h',
      'speech/SpeechSynthesis.cpp',
      'speech/SpeechSynthesis.h',
      'speech/SpeechSynthesisEvent.cpp',
      'speech/SpeechSynthesisEvent.h',
      'speech/SpeechSynthesisUtterance.cpp',
      'speech/SpeechSynthesisUtterance.h',
      'speech/SpeechSynthesisVoice.cpp',
      'speech/SpeechSynthesisVoice.h',
      'webaudio/AudioBasicInspectorNode.cpp',
      'webaudio/AudioBasicInspectorNode.h',
      'webaudio/AudioBasicProcessorNode.cpp',
      'webaudio/AudioBasicProcessorNode.h',
      'webaudio/AudioBuffer.cpp',
      'webaudio/AudioBuffer.h',
      'webaudio/AudioBufferCallback.h',
      'webaudio/AudioBufferSourceNode.cpp',
      'webaudio/AudioBufferSourceNode.h',
      'webaudio/ChannelMergerNode.cpp',
      'webaudio/ChannelMergerNode.h',
      'webaudio/ChannelSplitterNode.cpp',
      'webaudio/ChannelSplitterNode.h',
      'webaudio/AudioContext.cpp',
      'webaudio/AudioContext.h',
      'webaudio/AudioDestinationNode.cpp',
      'webaudio/AudioDestinationNode.h',
      'webaudio/GainNode.cpp',
      'webaudio/GainNode.h',
      'webaudio/AudioListener.cpp',
      'webaudio/AudioListener.h',
      'webaudio/AudioNode.cpp',
      'webaudio/AudioNode.h',
      'webaudio/AudioNodeInput.cpp',
      'webaudio/AudioNodeInput.h',
      'webaudio/AudioNodeOutput.cpp',
      'webaudio/AudioNodeOutput.h',
      'webaudio/PannerNode.cpp',
      'webaudio/PannerNode.h',
      'webaudio/AudioParam.cpp',
      'webaudio/AudioParam.h',
      'webaudio/AudioParamTimeline.cpp',
      'webaudio/AudioParamTimeline.h',
      'webaudio/AudioProcessingEvent.cpp',
      'webaudio/AudioProcessingEvent.h',
      'webaudio/AudioScheduledSourceNode.cpp',
      'webaudio/AudioScheduledSourceNode.h',
      'webaudio/AudioSourceNode.h',
      'webaudio/AudioSummingJunction.cpp',
      'webaudio/AudioSummingJunction.h',
      'webaudio/AsyncAudioDecoder.cpp',
      'webaudio/AsyncAudioDecoder.h',
      'webaudio/BiquadDSPKernel.cpp',
      'webaudio/BiquadDSPKernel.h',
      'webaudio/BiquadFilterNode.cpp',
      'webaudio/BiquadFilterNode.h',
      'webaudio/BiquadProcessor.cpp',
      'webaudio/BiquadProcessor.h',
      'webaudio/ConvolverNode.cpp',
      'webaudio/ConvolverNode.h',
      'webaudio/DefaultAudioDestinationNode.cpp',
      'webaudio/DefaultAudioDestinationNode.h',
      'webaudio/DelayDSPKernel.cpp',
      'webaudio/DelayDSPKernel.h',
      'webaudio/DelayNode.cpp',
      'webaudio/DelayNode.h',
      'webaudio/DelayProcessor.cpp',
      'webaudio/DelayProcessor.h',
      'webaudio/DynamicsCompressorNode.cpp',
      'webaudio/DynamicsCompressorNode.h',
      'webaudio/ScriptProcessorNode.cpp',
      'webaudio/ScriptProcessorNode.h',
      'webaudio/MediaElementAudioSourceNode.cpp',
      'webaudio/MediaElementAudioSourceNode.h',
      'webaudio/MediaStreamAudioDestinationNode.cpp',
      'webaudio/MediaStreamAudioDestinationNode.h',
      'webaudio/MediaStreamAudioSourceNode.cpp',
      'webaudio/MediaStreamAudioSourceNode.h',
      'webaudio/OfflineAudioCompletionEvent.cpp',
      'webaudio/OfflineAudioCompletionEvent.h',
      'webaudio/OfflineAudioContext.cpp',
      'webaudio/OfflineAudioContext.h',
      'webaudio/OfflineAudioDestinationNode.cpp',
      'webaudio/OfflineAudioDestinationNode.h',
      'webaudio/OscillatorNode.cpp',
      'webaudio/OscillatorNode.h',
      'webaudio/RealtimeAnalyser.cpp',
      'webaudio/RealtimeAnalyser.h',
      'webaudio/AnalyserNode.cpp',
      'webaudio/AnalyserNode.h',
      'webaudio/WaveShaperDSPKernel.cpp',
      'webaudio/WaveShaperDSPKernel.h',
      'webaudio/WaveShaperNode.cpp',
      'webaudio/WaveShaperNode.h',
      'webaudio/WaveShaperProcessor.cpp',
      'webaudio/WaveShaperProcessor.h',
      'webaudio/WaveTable.cpp',
      'webaudio/WaveTable.h',
      'webdatabase/AbstractDatabaseServer.h',
      'webdatabase/AbstractSQLStatement.h',
      'webdatabase/AbstractSQLStatementBackend.h',
      'webdatabase/ChangeVersionData.h',
      'webdatabase/ChangeVersionWrapper.cpp',
      'webdatabase/ChangeVersionWrapper.h',
      'webdatabase/Database.cpp',
      'webdatabase/DatabaseAuthorizer.cpp',
      'webdatabase/DatabaseAuthorizer.h',
      'webdatabase/DatabaseBackend.cpp',
      'webdatabase/DatabaseBackend.h',
      'webdatabase/DatabaseBackendBase.cpp',
      'webdatabase/DatabaseBackendBase.h',
      'webdatabase/DatabaseBackendContext.cpp',
      'webdatabase/DatabaseBackendContext.h',
      'webdatabase/DatabaseBackendSync.cpp',
      'webdatabase/DatabaseBackendSync.h',
      'webdatabase/DatabaseBase.cpp',
      'webdatabase/DatabaseBase.h',
      'webdatabase/DatabaseBasicTypes.h',
      'webdatabase/DatabaseCallback.h',
      'webdatabase/DatabaseContext.cpp',
      'webdatabase/DatabaseContext.h',
      'webdatabase/DatabaseError.h',
      'webdatabase/DatabaseManager.cpp',
      'webdatabase/DatabaseManager.h',
      'webdatabase/DatabaseServer.cpp',
      'webdatabase/DatabaseServer.h',
      'webdatabase/DatabaseSync.cpp',
      'webdatabase/DatabaseSync.h',
      'webdatabase/DatabaseTask.cpp',
      'webdatabase/DatabaseTask.h',
      'webdatabase/DatabaseThread.cpp',
      'webdatabase/DatabaseThread.h',
      'webdatabase/DatabaseTracker.h',
      'webdatabase/DOMWindowWebDatabase.cpp',
      'webdatabase/DOMWindowWebDatabase.h',
      'webdatabase/SQLCallbackWrapper.h',
      'webdatabase/SQLException.cpp',
      'webdatabase/SQLException.h',
      'webdatabase/SQLResultSet.cpp',
      'webdatabase/SQLResultSetRowList.cpp',
      'webdatabase/SQLStatement.cpp',
      'webdatabase/SQLStatement.h',
      'webdatabase/SQLStatementBackend.cpp',
      'webdatabase/SQLStatementBackend.h',
      'webdatabase/SQLStatementSync.cpp',
      'webdatabase/SQLStatementSync.h',
      'webdatabase/SQLTransaction.cpp',
      'webdatabase/SQLTransaction.h',
      'webdatabase/SQLTransactionBackend.cpp',
      'webdatabase/SQLTransactionBackend.h',
      'webdatabase/SQLTransactionBackendSync.cpp',
      'webdatabase/SQLTransactionBackendSync.h',
      'webdatabase/SQLTransactionClient.h',
      'webdatabase/SQLTransactionCoordinator.cpp',
      'webdatabase/SQLTransactionCoordinator.h',
      'webdatabase/SQLTransactionState.h',
      'webdatabase/SQLTransactionStateMachine.cpp',
      'webdatabase/SQLTransactionStateMachine.h',
      'webdatabase/SQLTransactionSync.cpp',
      'webdatabase/SQLTransactionSync.h',
      'webdatabase/SQLTransactionSyncCallback.h',
      'webdatabase/WorkerContextWebDatabase.cpp',
      'webdatabase/WorkerContextWebDatabase.h',
      'webdatabase/chromium/DatabaseObserver.h',
      'webdatabase/chromium/DatabaseTrackerChromium.cpp',
      'webdatabase/chromium/QuotaTracker.cpp',
      'webdatabase/chromium/QuotaTracker.h',
      'webdatabase/chromium/SQLTransactionClientChromium.cpp',
      'webmidi/MIDIErrorCallback.cpp',
      'webmidi/MIDIErrorCallback.h',
      'webmidi/MIDIMessageEvent.h',
      'webmidi/MIDIPort.cpp',
      'webmidi/MIDIPort.h',
      'websockets/CloseEvent.h',
      'websockets/MainThreadWebSocketChannel.cpp',
      'websockets/MainThreadWebSocketChannel.h',
      'websockets/ThreadableWebSocketChannelClientWrapper.cpp',
      'websockets/ThreadableWebSocketChannelClientWrapper.h',
      'websockets/WebSocket.cpp',
      'websockets/WebSocket.h',
      'websockets/WebSocketChannel.cpp',
      'websockets/WebSocketChannel.h',
      'websockets/WebSocketChannelClient.h',
      'websockets/WebSocketDeflateFramer.cpp',
      'websockets/WebSocketDeflateFramer.h',
      'websockets/WebSocketDeflater.cpp',
      'websockets/WebSocketDeflater.h',
      'websockets/WebSocketExtensionDispatcher.cpp',
      'websockets/WebSocketExtensionDispatcher.h',
      'websockets/WebSocketExtensionProcessor.h',
      'websockets/WebSocketExtensionParser.cpp',
      'websockets/WebSocketExtensionParser.h',
      'websockets/WebSocketFrame.cpp',
      'websockets/WebSocketFrame.h',
      'websockets/WebSocketHandshake.cpp',
      'websockets/WebSocketHandshake.h',
      'websockets/WebSocketHandshakeRequest.cpp',
      'websockets/WebSocketHandshakeRequest.h',
      'websockets/WebSocketHandshakeResponse.cpp',
      'websockets/WebSocketHandshakeResponse.h',
      'websockets/WorkerThreadableWebSocketChannel.cpp',
      'websockets/WorkerThreadableWebSocketChannel.h',
    ],
  },
}
