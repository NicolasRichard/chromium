/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 * Copyright (C) 2012 Intel Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @constructor
 * @extends {WebInspector.Object}
 * @param {!WebInspector.TimelineModel} model
 */
WebInspector.TimelinePresentationModel = function(model)
{
    this._model = model;
    this._filters = [];
    this._bindings = new WebInspector.TimelinePresentationModel.InterRecordBindings();
    this.reset();
}

/**
 * @param {!Array.<*>} recordsArray
 * @param {?function(*)|?function(*,number)} preOrderCallback
 * @param {function(*)|function(*,number)=} postOrderCallback
 */
WebInspector.TimelinePresentationModel.forAllRecords = function(recordsArray, preOrderCallback, postOrderCallback)
{
    if (!recordsArray)
        return;
    var stack = [{array: recordsArray, index: 0}];
    while (stack.length) {
        var entry = stack[stack.length - 1];
        var records = entry.array;
        if (entry.index < records.length) {
             var record = records[entry.index];
             if (preOrderCallback && preOrderCallback(record, stack.length))
                 return;
             if (record.children)
                 stack.push({array: record.children, index: 0, record: record});
             else if (postOrderCallback && postOrderCallback(record, stack.length))
                return;
             ++entry.index;
        } else {
            if (entry.record && postOrderCallback && postOrderCallback(entry.record, stack.length))
                return;
            stack.pop();
        }
    }
}

WebInspector.TimelinePresentationModel._hiddenRecords = { };
WebInspector.TimelinePresentationModel._hiddenRecords[WebInspector.TimelineModel.RecordType.MarkDOMContent] = 1;
WebInspector.TimelinePresentationModel._hiddenRecords[WebInspector.TimelineModel.RecordType.MarkLoad] = 1;
WebInspector.TimelinePresentationModel._hiddenRecords[WebInspector.TimelineModel.RecordType.MarkFirstPaint] = 1;
WebInspector.TimelinePresentationModel._hiddenRecords[WebInspector.TimelineModel.RecordType.GPUTask] = 1;
WebInspector.TimelinePresentationModel._hiddenRecords[WebInspector.TimelineModel.RecordType.ScheduleStyleRecalculation] = 1;
WebInspector.TimelinePresentationModel._hiddenRecords[WebInspector.TimelineModel.RecordType.InvalidateLayout] = 1;
WebInspector.TimelinePresentationModel._hiddenRecords[WebInspector.TimelineModel.RecordType.RequestMainThreadFrame] = 1;
WebInspector.TimelinePresentationModel._hiddenRecords[WebInspector.TimelineModel.RecordType.ActivateLayerTree] = 1;
WebInspector.TimelinePresentationModel._hiddenRecords[WebInspector.TimelineModel.RecordType.DrawFrame] = 1;
WebInspector.TimelinePresentationModel._hiddenRecords[WebInspector.TimelineModel.RecordType.BeginFrame] = 1;

WebInspector.TimelinePresentationModel._coalescingRecords = { };
WebInspector.TimelinePresentationModel._coalescingRecords[WebInspector.TimelineModel.RecordType.Paint] = 1;
WebInspector.TimelinePresentationModel._coalescingRecords[WebInspector.TimelineModel.RecordType.Rasterize] = 1;
WebInspector.TimelinePresentationModel._coalescingRecords[WebInspector.TimelineModel.RecordType.DecodeImage] = 1;
WebInspector.TimelinePresentationModel._coalescingRecords[WebInspector.TimelineModel.RecordType.ResizeImage] = 1;

WebInspector.TimelinePresentationModel.prototype = {
    /**
     * @param {!WebInspector.TimelinePresentationModel.Filter} filter
     */
    addFilter: function(filter)
    {
        this._filters.push(filter);
    },

    /**
     * @param {?WebInspector.TimelinePresentationModel.Filter} filter
     */
    setSearchFilter: function(filter)
    {
        this._searchFilter = filter;
    },

    /**
     * @return {!WebInspector.TimelinePresentationModel.Record}
     */
    rootRecord: function()
    {
        return this._rootRecord;
    },

    reset: function()
    {
        var rootPayload = { type: WebInspector.TimelineModel.RecordType.Root };
        this._rootRecord = new WebInspector.TimelinePresentationModel.Record(this._model, this._bindings, /** @type {!TimelineAgent.TimelineEvent} */ (rootPayload), null);
        this._eventDividerRecords = [];
        this._minimumRecordTime = -1;
        this._coalescingBuckets = {};
        this._mergingBuffer = new WebInspector.TimelineMergingRecordBuffer();

        /** @type {!Array.<!TimelineAgent.TimelineEvent>} */
        this._mainThreadTasks =  ([]);
        /** @type {!Array.<!TimelineAgent.TimelineEvent>} */
        this._gpuThreadTasks = ([]);

        this._bindings._reset();
    },

    /**
     * @return {number}
     */
    minimumRecordTime: function()
    {
        return this._minimumRecordTime;
    },

    /**
     * @return {number}
     */
    maximumRecordTime: function()
    {
        return this._maximumRecordTime;
    },

    /**
     * @return {!Array.<!TimelineAgent.TimelineEvent>}
     */
    mainThreadTasks: function()
    {
        return this._mainThreadTasks;
    },

    /**
     * @return {!Array.<!TimelineAgent.TimelineEvent>}
     */
    gpuThreadTasks: function()
    {
        return this._gpuThreadTasks;
    },

    /**
     * @param {!TimelineAgent.TimelineEvent} record
     * @return {!Array.<!WebInspector.TimelinePresentationModel.Record>}
     */
    addRecord: function(record)
    {
        if (record.type === WebInspector.TimelineModel.RecordType.Program)
            this._mainThreadTasks.push(record);
        if (record.type === WebInspector.TimelineModel.RecordType.GPUTask) {
            this._gpuThreadTasks.push(record);
            return [];
        }

        var startTime = record.startTime;
        var endTime = record.endTime;
        if (this._minimumRecordTime === -1 || startTime < this._minimumRecordTime)
            this._minimumRecordTime = startTime;
        if (this._maximumRecordTime === -1 || endTime > this._maximumRecordTime)
            this._maximumRecordTime = endTime;

        var records;
        if (record.type === WebInspector.TimelineModel.RecordType.Program)
            records = record.children;
        else
            records = [record];
        var mergedRecords = this._mergingBuffer.process(record.thread, records);
        var result = [];
        for (var i = 0; i < mergedRecords.length; ++i) {
            var formattedRecord = this._innerAddRecord(this._rootRecord, mergedRecords[i]);
            if (formattedRecord)
                result.push(formattedRecord);
        }
        return result;
    },

    /**
     * @param {!WebInspector.TimelinePresentationModel.Record} parentRecord
     * @param {!TimelineAgent.TimelineEvent} record
     * @return {?WebInspector.TimelinePresentationModel.Record}
     */
    _innerAddRecord: function(parentRecord, record)
    {
        const recordTypes = WebInspector.TimelineModel.RecordType;

        var origin = parentRecord;
        var coalescingBucket;

        // On main thread, only coalesce if the last event is of same type.
        if (parentRecord === this._rootRecord)
            coalescingBucket = record.thread ? record.type : "mainThread";
        var coalescedRecord = this._findCoalescedParent(record, parentRecord, coalescingBucket);
        if (coalescedRecord)
            parentRecord = coalescedRecord;

        if (WebInspector.TimelineUIUtils.isEventDivider(record))
            this._eventDividerRecords.push(record);

        var formattedRecord = new WebInspector.TimelinePresentationModel.Record(this._model, this._bindings, record, parentRecord);
        if (record.type in WebInspector.TimelinePresentationModel._hiddenRecords) {
            parentRecord.children.pop();
            return null;
        }

        formattedRecord.collapsed = parentRecord === this._rootRecord;
        if (coalescingBucket)
            this._coalescingBuckets[coalescingBucket] = formattedRecord;

        for (var i = 0; record.children && i < record.children.length; ++i)
            this._innerAddRecord(formattedRecord, record.children[i]);

        formattedRecord.calculateAggregatedStats();
        if (parentRecord.coalesced)
            this._updateCoalescingParent(formattedRecord);

        origin._selfTime -= formattedRecord.endTime - formattedRecord.startTime;
        return formattedRecord;
    },

    /**
     * @param {!TimelineAgent.TimelineEvent} record
     * @param {!Object} newParent
     * @param {string=} bucket
     * @return {?WebInspector.TimelinePresentationModel.Record}
     */
    _findCoalescedParent: function(record, newParent, bucket)
    {
        const coalescingThresholdMillis = 5;

        var lastRecord = bucket ? this._coalescingBuckets[bucket] : newParent.children.peekLast();
        if (lastRecord && lastRecord.coalesced)
            lastRecord = lastRecord.children.peekLast();
        var startTime = record.startTime;
        var endTime = record.endTime;
        if (!lastRecord)
            return null;
        if (lastRecord.type !== record.type)
            return null;
        if (!WebInspector.TimelinePresentationModel._coalescingRecords[record.type])
            return null;
        if (lastRecord.endTime + coalescingThresholdMillis < startTime)
            return null;
        if (endTime + coalescingThresholdMillis < lastRecord.startTime)
            return null;
        if (lastRecord.parent.coalesced)
            return lastRecord.parent;
        return this._replaceWithCoalescedRecord(lastRecord);
    },

    /**
     * @param {!WebInspector.TimelinePresentationModel.Record} record
     * @return {!WebInspector.TimelinePresentationModel.Record}
     */
    _replaceWithCoalescedRecord: function(record)
    {
        var rawRecord = {
            type: record._record.type,
            startTime: record._record.startTime,
            endTime: record._record.endTime,
            data: { }
        };
        if (record._record.thread)
            rawRecord.thread = "aggregated";
        if (record.type === WebInspector.TimelineModel.RecordType.TimeStamp)
            rawRecord.data.message = record.data.message;

        var coalescedRecord = new WebInspector.TimelinePresentationModel.Record(this._model, this._bindings, rawRecord, null);
        var parent = record.parent;

        coalescedRecord.coalesced = true;
        coalescedRecord.collapsed = true;
        coalescedRecord._children.push(record);
        record.parent = coalescedRecord;
        if (record.hasWarnings() || record.childHasWarnings())
            coalescedRecord._childHasWarnings = true;

        coalescedRecord.parent = parent;
        parent._children[parent._children.indexOf(record)] = coalescedRecord;
        WebInspector.TimelineUIUtils.aggregateTimeByCategory(coalescedRecord._aggregatedStats, record._aggregatedStats);

        return coalescedRecord;
    },

    /**
     * @param {!WebInspector.TimelinePresentationModel.Record} record
     */
    _updateCoalescingParent: function(record)
    {
        var parentRecord = record.parent;
        WebInspector.TimelineUIUtils.aggregateTimeByCategory(parentRecord._aggregatedStats, record._aggregatedStats);
        if (parentRecord.startTime > record._record.startTime)
            parentRecord._record.startTime = record._record.startTime;
        if (parentRecord.endTime < record._record.endTime) {
            parentRecord._record.endTime = record._record.endTime;
            parentRecord.lastChildEndTime = parentRecord.endTime;
        }
    },

    invalidateFilteredRecords: function()
    {
        delete this._filteredRecords;
    },

    /**
     * @return {!Array.<!WebInspector.TimelinePresentationModel.Record>}
     */
    filteredRecords: function()
    {
        if (this._filteredRecords)
            return this._filteredRecords;

        var recordsInWindow = [];
        var stack = [{children: this._rootRecord.children, index: 0, parentIsCollapsed: false, parentRecord: {}}];
        var revealedDepth = 0;

        function revealRecordsInStack() {
            for (var depth = revealedDepth + 1; depth < stack.length; ++depth) {
                if (stack[depth - 1].parentIsCollapsed) {
                    stack[depth].parentRecord.parent._expandable = true;
                    return;
                }
                stack[depth - 1].parentRecord.collapsed = false;
                recordsInWindow.push(stack[depth].parentRecord);
                stack[depth].windowLengthBeforeChildrenTraversal = recordsInWindow.length;
                stack[depth].parentIsRevealed = true;
                revealedDepth = depth;
            }
        }

        while (stack.length) {
            var entry = stack[stack.length - 1];
            var records = entry.children;
            if (records && entry.index < records.length) {
                var record = records[entry.index];
                ++entry.index;

                if (this.isVisible(record)) {
                    record.parent._expandable = true;
                    if (this._searchFilter)
                        revealRecordsInStack();
                    if (!entry.parentIsCollapsed) {
                        recordsInWindow.push(record);
                        revealedDepth = stack.length;
                        entry.parentRecord.collapsed = false;
                    }
                }

                record._expandable = false;

                stack.push({children: record.children,
                            index: 0,
                            parentIsCollapsed: (entry.parentIsCollapsed || (record.collapsed && (!this._searchFilter || record.clicked))),
                            parentRecord: record,
                            windowLengthBeforeChildrenTraversal: recordsInWindow.length});
            } else {
                stack.pop();
                revealedDepth = Math.min(revealedDepth, stack.length - 1);
                entry.parentRecord._visibleChildrenCount = recordsInWindow.length - entry.windowLengthBeforeChildrenTraversal;
            }
        }

        this._filteredRecords = recordsInWindow;
        return recordsInWindow;
    },

    /**
     * @return {!Array.<!TimelineAgent.TimelineEvent>}
     */
    eventDividerRecords: function()
    {
        return this._eventDividerRecords;
    },

    /**
     * @return {boolean}
     */
    isVisible: function(record)
    {
        for (var i = 0; i < this._filters.length; ++i) {
            if (!this._filters[i].accept(record))
                return false;
        }
        return !this._searchFilter || this._searchFilter.accept(record);
    },

    __proto__: WebInspector.Object.prototype
}

/**
 * @constructor
 */
WebInspector.TimelinePresentationModel.InterRecordBindings = function()
{
    this._reset();
}

WebInspector.TimelinePresentationModel.InterRecordBindings.prototype = {
    _reset: function()
    {
        this._sendRequestRecords = {};
        this._timerRecords = {};
        this._requestAnimationFrameRecords = {};
        this._layoutInvalidateStack = {};
        this._lastScheduleStyleRecalculation = {};
        this._webSocketCreateRecords = {};
    }
}

/**
 * @constructor
 * @param {!WebInspector.TimelineModel} model
 * @param {!WebInspector.TimelinePresentationModel.InterRecordBindings} bindings
 * @param {!TimelineAgent.TimelineEvent} record
 * @param {?WebInspector.TimelinePresentationModel.Record} parentRecord
 */
WebInspector.TimelinePresentationModel.Record = function(model, bindings, record, parentRecord)
{
    this._model = model;
    this._aggregatedStats = {};
    this._record = /** @type {!TimelineAgent.TimelineEvent} */ (record);
    this._children = [];
    if (parentRecord) {
        this.parent = parentRecord;
        parentRecord.children.push(this);
    }

    this._selfTime = this.endTime - this.startTime;
    this._lastChildEndTime = this.endTime;
    this._startTimeOffset = this.startTime - model.minimumRecordTime();

    if (record.data) {
        if (record.data["url"])
            this.url = record.data["url"];
        if (record.data["rootNode"])
            this._relatedBackendNodeId = record.data["rootNode"];
        else if (record.data["elementId"])
            this._relatedBackendNodeId = record.data["elementId"];
        if (record.data["scriptName"]) {
            this.scriptName = record.data["scriptName"];
            this.scriptLine = record.data["scriptLine"];
        }
    }

    if (parentRecord && parentRecord.callSiteStackTrace)
        this.callSiteStackTrace = parentRecord.callSiteStackTrace;

    var recordTypes = WebInspector.TimelineModel.RecordType;
    switch (record.type) {
    case recordTypes.ResourceSendRequest:
        // Make resource receive record last since request was sent; make finish record last since response received.
        bindings._sendRequestRecords[record.data["requestId"]] = this;
        break;

    case recordTypes.ResourceReceiveResponse:
        var sendRequestRecord = bindings._sendRequestRecords[record.data["requestId"]];
        if (sendRequestRecord) // False if we started instrumentation in the middle of request.
            this.url = sendRequestRecord.url;
        break;

    case recordTypes.ResourceReceivedData:
    case recordTypes.ResourceFinish:
        var sendRequestRecord = bindings._sendRequestRecords[record.data["requestId"]];
        if (sendRequestRecord) // False for main resource.
            this.url = sendRequestRecord.url;
        break;

    case recordTypes.TimerInstall:
        this.timeout = record.data["timeout"];
        this.singleShot = record.data["singleShot"];
        bindings._timerRecords[record.data["timerId"]] = this;
        break;

    case recordTypes.TimerFire:
        var timerInstalledRecord = bindings._timerRecords[record.data["timerId"]];
        if (timerInstalledRecord) {
            this.callSiteStackTrace = timerInstalledRecord.stackTrace;
            this.timeout = timerInstalledRecord.timeout;
            this.singleShot = timerInstalledRecord.singleShot;
        }
        break;

    case recordTypes.RequestAnimationFrame:
        bindings._requestAnimationFrameRecords[record.data["id"]] = this;
        break;

    case recordTypes.FireAnimationFrame:
        var requestAnimationRecord = bindings._requestAnimationFrameRecords[record.data["id"]];
        if (requestAnimationRecord)
            this.callSiteStackTrace = requestAnimationRecord.stackTrace;
        break;

    case recordTypes.ConsoleTime:
        var message = record.data["message"];
        break;

    case recordTypes.ScheduleStyleRecalculation:
        bindings._lastScheduleStyleRecalculation[this.frameId] = this;
        break;

    case recordTypes.RecalculateStyles:
        var scheduleStyleRecalculationRecord = bindings._lastScheduleStyleRecalculation[this.frameId];
        if (!scheduleStyleRecalculationRecord)
            break;
        this.callSiteStackTrace = scheduleStyleRecalculationRecord.stackTrace;
        break;

    case recordTypes.InvalidateLayout:
        // Consider style recalculation as a reason for layout invalidation,
        // but only if we had no earlier layout invalidation records.
        var styleRecalcStack;
        if (!bindings._layoutInvalidateStack[this.frameId]) {
            if (parentRecord.type === recordTypes.RecalculateStyles)
                styleRecalcStack = parentRecord.callSiteStackTrace;
        }
        bindings._layoutInvalidateStack[this.frameId] = styleRecalcStack || this.stackTrace;
        break;

    case recordTypes.Layout:
        var layoutInvalidateStack = bindings._layoutInvalidateStack[this.frameId];
        if (layoutInvalidateStack)
            this.callSiteStackTrace = layoutInvalidateStack;
        if (this.stackTrace)
            this.addWarning(WebInspector.UIString("Forced synchronous layout is a possible performance bottleneck."));

        bindings._layoutInvalidateStack[this.frameId] = null;
        this.highlightQuad = record.data.root || WebInspector.TimelinePresentationModel.quadFromRectData(record.data);
        this._relatedBackendNodeId = record.data["rootNode"];
        break;

    case recordTypes.AutosizeText:
        if (record.data.needsRelayout && parentRecord.type === recordTypes.Layout)
            parentRecord.addWarning(WebInspector.UIString("Layout required two passes due to text autosizing, consider setting viewport."));
        break;

    case recordTypes.Paint:
        this.highlightQuad = record.data.clip || WebInspector.TimelinePresentationModel.quadFromRectData(record.data);
        break;

    case recordTypes.WebSocketCreate:
        this.webSocketURL = record.data["url"];
        if (typeof record.data["webSocketProtocol"] !== "undefined")
            this.webSocketProtocol = record.data["webSocketProtocol"];
        bindings._webSocketCreateRecords[record.data["identifier"]] = this;
        break;

    case recordTypes.WebSocketSendHandshakeRequest:
    case recordTypes.WebSocketReceiveHandshakeResponse:
    case recordTypes.WebSocketDestroy:
        var webSocketCreateRecord = bindings._webSocketCreateRecords[record.data["identifier"]];
        if (webSocketCreateRecord) { // False if we started instrumentation in the middle of request.
            this.webSocketURL = webSocketCreateRecord.webSocketURL;
            if (typeof webSocketCreateRecord.webSocketProtocol !== "undefined")
                this.webSocketProtocol = webSocketCreateRecord.webSocketProtocol;
        }
        break;

    case recordTypes.EmbedderCallback:
        this.embedderCallbackName = record.data["callbackName"];
        break;
    }
}

WebInspector.TimelinePresentationModel.Record.prototype = {
    get lastChildEndTime()
    {
        return this._lastChildEndTime;
    },

    set lastChildEndTime(time)
    {
        this._lastChildEndTime = time;
    },

    get selfTime()
    {
        return this.coalesced ? this._lastChildEndTime - this.startTime : this._selfTime;
    },

    get cpuTime()
    {
        return this._cpuTime;
    },

    /**
     * @return {boolean}
     */
    isRoot: function()
    {
        return this.type === WebInspector.TimelineModel.RecordType.Root;
    },

    /**
     * @return {!Array.<!WebInspector.TimelinePresentationModel.Record>}
     */
    get children()
    {
        return this._children;
    },

    /**
     * @return {number}
     */
    get visibleChildrenCount()
    {
        return this._visibleChildrenCount || 0;
    },

    /**
     * @return {boolean}
     */
    get expandable()
    {
        return !!this._expandable;
    },

    /**
     * @return {!WebInspector.TimelineCategory}
     */
    get category()
    {
        return WebInspector.TimelineUIUtils.categoryForRecord(this._record);
    },

    /**
     * @return {string}
     */
    title: function()
    {
        return WebInspector.TimelineUIUtils.recordTitle(this._record);
    },

    /**
     * @return {number}
     */
    get startTime()
    {
        return this._record.startTime;
    },

    /**
     * @return {number}
     */
    get startTimeOffset()
    {
        return this._startTimeOffset;
    },

    /**
     * @return {number}
     */
    get endTime()
    {
        return this._record.endTime || this._record.startTime;
    },

    /**
     * @return {boolean}
     */
    isBackground: function()
    {
        return !!this._record.thread;
    },

    /**
     * @return {!Object}
     */
    get data()
    {
        return this._record.data;
    },

    /**
     * @return {string}
     */
    get type()
    {
        return this._record.type;
    },

    /**
     * @return {string}
     */
    get frameId()
    {
        return this._record.frameId || "";
    },

    /**
     * @return {number}
     */
    get usedHeapSizeDelta()
    {
        return this._record.usedHeapSizeDelta || 0;
    },

    /**
     * @return {number}
     */
    get jsHeapSizeUsed()
    {
        return this._record.counters ? this._record.counters.jsHeapSizeUsed || 0 : 0;
    },

    /**
     * @return {?Array.<!ConsoleAgent.CallFrame>}
     */
    get stackTrace()
    {
        if (this._record.stackTrace && this._record.stackTrace.length)
            return this._record.stackTrace;
        return null;
    },

    /**
     * @return {boolean}
     */
    containsTime: function(time)
    {
        return this.startTime <= time && time <= this.endTime;
    },

    /**
     * @param {string} key
     * @return {?Object}
     */
    getUserObject: function(key)
    {
        if (!this._userObjects)
            return null;
        return this._userObjects.get(key);
    },

    /**
     * @param {string} key
     * @param {?Object|undefined} value
     */
    setUserObject: function(key, value)
    {
        if (!this._userObjects)
            this._userObjects = new StringMap();
        this._userObjects.put(key, value);
    },

    /**
     * @return {number} nodeId
     */
    relatedBackendNodeId: function()
    {
        return this._relatedBackendNodeId;
    },

    calculateAggregatedStats: function()
    {
        this._aggregatedStats = {};
        this._cpuTime = this._selfTime;

        for (var index = this._children.length; index; --index) {
            var child = this._children[index - 1];
            for (var category in child._aggregatedStats)
                this._aggregatedStats[category] = (this._aggregatedStats[category] || 0) + child._aggregatedStats[category];
        }
        for (var category in this._aggregatedStats)
            this._cpuTime += this._aggregatedStats[category];
        this._aggregatedStats[this.category.name] = (this._aggregatedStats[this.category.name] || 0) + this._selfTime;
    },

    get aggregatedStats()
    {
        return this._aggregatedStats;
    },

    /**
     * @param {string} message
     */
    addWarning: function(message)
    {
        if (this._warnings)
            this._warnings.push(message);
        else
            this._warnings = [message];
        for (var parent = this.parent; parent && !parent._childHasWarnings; parent = parent.parent)
            parent._childHasWarnings = true;
    },

    /**
     * @return {boolean}
     */
    hasWarnings: function()
    {
        return !!this._warnings;
    },

    /**
     * @return {!Object}
     */
    warnings: function()
    {
        return this._warnings;
    },

    /**
     * @return {boolean}
     */
    childHasWarnings: function()
    {
        return this._childHasWarnings;
    },

    /**
     * @param {!RegExp} regExp
     * @return {boolean}
     */
    testContentMatching: function(regExp)
    {
        var tokens = Object.values(this._record.data);
        tokens.push(this.title());
        return regExp.test(tokens.join("|"));
    }
}

/**
 * @param {!Array.<number>} quad
 * @return {number}
 */
WebInspector.TimelinePresentationModel.quadWidth = function(quad)
{
    return Math.round(Math.sqrt(Math.pow(quad[0] - quad[2], 2) + Math.pow(quad[1] - quad[3], 2)));
}

/**
 * @param {!Array.<number>} quad
 * @return {number}
 */
WebInspector.TimelinePresentationModel.quadHeight = function(quad)
{
    return Math.round(Math.sqrt(Math.pow(quad[0] - quad[6], 2) + Math.pow(quad[1] - quad[7], 2)));
}

/**
 * @param {!Object} data
 * @return {?Array.<number>}
 */
WebInspector.TimelinePresentationModel.quadFromRectData = function(data)
{
    if (typeof data["x"] === "undefined" || typeof data["y"] === "undefined")
        return null;
    var x0 = data["x"];
    var x1 = data["x"] + data["width"];
    var y0 = data["y"];
    var y1 = data["y"] + data["height"];
    return [x0, y0, x1, y0, x1, y1, x0, y1];
}

/**
 * @interface
 */
WebInspector.TimelinePresentationModel.Filter = function()
{
}

WebInspector.TimelinePresentationModel.Filter.prototype = {
    /**
     * @param {!WebInspector.TimelinePresentationModel.Record} record
     * @return {boolean}
     */
    accept: function(record) { return false; }
}
