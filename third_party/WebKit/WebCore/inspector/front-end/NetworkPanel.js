/*
 * Copyright (C) 2007, 2008 Apple Inc.  All rights reserved.
 * Copyright (C) 2008, 2009 Anthony Ricaud <rik@webkit.org>
 * Copyright (C) 2009, 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

WebInspector.NetworkPanel = function()
{
    WebInspector.Panel.call(this, "network");

    this._resources = [];
    this._staleResources = [];
    this._resourceGridNodes = {};
    this._mainResourceLoadTime = -1;
    this._mainResourceDOMContentTime = -1;

    this._categories = WebInspector.resourceCategories;

    this._viewsContainerElement = document.createElement("div");
    this._viewsContainerElement.id = "network-views";
    this.element.appendChild(this._viewsContainerElement);

    this._createSortingFunctions();
    this._createTable();
    this._createTimelineGrid();
    this._createStatusbarButtons();
    this._createFilterStatusBarItems();
    this._createSummaryBar();

    this._popoverHelper = new WebInspector.PopoverHelper(this.element, this._getPopoverAnchor.bind(this), this._showPopover.bind(this), true);

    this.calculator = new WebInspector.NetworkTransferTimeCalculator();
    this._filter(this._filterAllElement, false);
}

WebInspector.NetworkPanel.prototype = {
    get toolbarItemLabel()
    {
        return WebInspector.UIString("Network");
    },

    get statusBarItems()
    {
        return [this._largerResourcesButton.element, this._clearButton.element, this._filterBarElement];
    },

    isCategoryVisible: function(categoryName)
    {
        return true;
    },

    elementsToRestoreScrollPositionsFor: function()
    {
        return [this.containerElement];
    },

    resize: function()
    {
        WebInspector.Panel.prototype.resize.call(this);
        this._dataGrid.updateWidths();
        this._positionSummaryBar();
    },

    _positionSummaryBar: function()
    {
        // Position the total bar.
        const rowHeight = 22;
        const summaryBarHeight = 22;
        var offsetHeight = this.element.offsetHeight;

        var parentElement = this._summaryBarElement.parentElement;

        if (this._summaryBarElement.parentElement !== this.element && offsetHeight > (this._dataGrid.children.length - 1) * rowHeight + summaryBarHeight) {
            this._dataGrid.removeChild(this._summaryBarRowNode);
            this._summaryBarElement.addStyleClass("network-summary-bar-bottom");
            delete this._summaryBarRowNode;
            this.element.appendChild(this._summaryBarElement);
            this._dataGrid.element.style.bottom = "20px";
            return;
        }

        if (!this._summaryBarRowNode && offsetHeight - summaryBarHeight < this._dataGrid.children.length * rowHeight) {
            this._summaryBarRowNode = new WebInspector.NetworkTotalGridNode(this._summaryBarElement);
            this._summaryBarElement.removeStyleClass("network-summary-bar-bottom");
            this._dataGrid.appendChild(this._summaryBarRowNode);
            this._dataGrid.element.style.bottom = 0;
            this._sortItems();
        }
    },

    _createTimelineGrid: function()
    {
        this._timelineGrid = new WebInspector.TimelineGrid();
        this._timelineGrid.element.addStyleClass("network-timeline-grid");
        this._dataGrid.element.appendChild(this._timelineGrid.element);
    },

    _createTable: function()
    {
        this.containerElement = document.createElement("div");
        this.containerElement.id = "network-container";
        this.element.appendChild(this.containerElement);

        var columns = {name: {}, method: {}, status: {}, type: {}, size: {}, time: {}, timeline: {}};
        columns.name.titleDOMFragment = this._makeHeaderFragment(WebInspector.UIString("Name"), WebInspector.UIString("Path"));
        columns.name.sortable = true;
        columns.name.width = "20%";
        columns.name.disclosure = true;
        
        columns.method.title = WebInspector.UIString("Method");
        columns.method.sortable = true;
        columns.method.width = "7%";

        columns.status.titleDOMFragment = this._makeHeaderFragment(WebInspector.UIString("Status"), WebInspector.UIString("Text"));
        columns.status.sortable = true;
        columns.status.width = "8%";

        columns.type.title = WebInspector.UIString("Type");
        columns.type.sortable = true;
        columns.type.width = "7%";

        columns.size.titleDOMFragment = this._makeHeaderFragment(WebInspector.UIString("Size"), WebInspector.UIString("Transfer"));
        columns.size.sortable = true;
        columns.size.width = "10%";
        columns.size.aligned = "right";

        columns.time.titleDOMFragment = this._makeHeaderFragment(WebInspector.UIString("Time"), WebInspector.UIString("Duration"));
        columns.time.sortable = true;
        columns.time.width = "10%";
        columns.time.aligned = "right";

        columns.timeline.title = "";
        columns.timeline.sortable = false;
        columns.timeline.width = "40%";
        columns.timeline.sort = "ascending";

        this._dataGrid = new WebInspector.DataGrid(columns);
        this.element.appendChild(this._dataGrid.element);
        this._dataGrid.addEventListener("sorting changed", this._sortItems, this);
        this._dataGrid.addEventListener("width changed", this._updateDividersIfNeeded, this);

        this._patchTimelineHeader();
    },

    _makeHeaderFragment: function(title, subtitle)
    {
        var fragment = document.createDocumentFragment();
        fragment.appendChild(document.createTextNode(title));
        var subtitleDiv = document.createElement("div");
        subtitleDiv.className = "network-header-subtitle";
        subtitleDiv.textContent = subtitle;
        fragment.appendChild(subtitleDiv);
        return fragment;
    },

    _patchTimelineHeader: function()
    {
        var timelineSorting = document.createElement("select");

        var option = document.createElement("option");
        option.value = "startTime";
        option.label = WebInspector.UIString("Timeline");
        timelineSorting.appendChild(option);

        option = document.createElement("option");
        option.value = "startTime";
        option.label = WebInspector.UIString("Start Time");
        timelineSorting.appendChild(option);

        option = document.createElement("option");
        option.value = "responseTime";
        option.label = WebInspector.UIString("Response Time");
        timelineSorting.appendChild(option);

        option = document.createElement("option");
        option.value = "endTime";
        option.label = WebInspector.UIString("End Time");
        timelineSorting.appendChild(option);

        option = document.createElement("option");
        option.value = "duration";
        option.label = WebInspector.UIString("Duration");
        timelineSorting.appendChild(option);

        option = document.createElement("option");
        option.value = "latency";
        option.label = WebInspector.UIString("Latency");
        timelineSorting.appendChild(option);

        var header = this._dataGrid.headerTableHeader("timeline");
        header.firstChild.appendChild(timelineSorting);

        timelineSorting.addEventListener("click", function(event) { event.stopPropagation() }, false);
        timelineSorting.addEventListener("change", this._sortByTimeline.bind(this), false);
        this._timelineSortSelector = timelineSorting;
    },

    _createSortingFunctions: function()
    {
        this._sortingFunctions = {};
        this._sortingFunctions.name = WebInspector.NetworkDataGridNode.NameComparator;
        this._sortingFunctions.method = WebInspector.NetworkDataGridNode.ResourcePropertyComparator.bind(null, "method", false);
        this._sortingFunctions.status = WebInspector.NetworkDataGridNode.ResourcePropertyComparator.bind(null, "statusCode", false);
        this._sortingFunctions.type = WebInspector.NetworkDataGridNode.ResourcePropertyComparator.bind(null, "mimeType", false);
        this._sortingFunctions.size = WebInspector.NetworkDataGridNode.SizeComparator;
        this._sortingFunctions.time = WebInspector.NetworkDataGridNode.ResourcePropertyComparator.bind(null, "duration", false);
        this._sortingFunctions.timeline = WebInspector.NetworkDataGridNode.ResourcePropertyComparator.bind(null, "startTime", false);
        this._sortingFunctions.startTime = WebInspector.NetworkDataGridNode.ResourcePropertyComparator.bind(null, "startTime", false);
        this._sortingFunctions.endTime = WebInspector.NetworkDataGridNode.ResourcePropertyComparator.bind(null, "endTime", false);
        this._sortingFunctions.responseTime = WebInspector.NetworkDataGridNode.ResourcePropertyComparator.bind(null, "responseReceivedTime", false);
        this._sortingFunctions.duration = WebInspector.NetworkDataGridNode.ResourcePropertyComparator.bind(null, "duration", true);
        this._sortingFunctions.latency = WebInspector.NetworkDataGridNode.ResourcePropertyComparator.bind(null, "latency", true);

        var timeCalculator = new WebInspector.NetworkTransferTimeCalculator();
        var durationCalculator = new WebInspector.NetworkTransferDurationCalculator();

        this._calculators = {};
        this._calculators.timeline = timeCalculator;
        this._calculators.startTime = timeCalculator;
        this._calculators.endTime = timeCalculator;
        this._calculators.responseTime = timeCalculator;
        this._calculators.duration = durationCalculator;
        this._calculators.latency = durationCalculator;
    },

    _sortItems: function()
    {
        var columnIdentifier = this._dataGrid.sortColumnIdentifier;
        if (columnIdentifier === "timeline") {
            this._sortByTimeline();
            return;
        }
        var sortingFunction = this._sortingFunctions[columnIdentifier];
        if (!sortingFunction)
            return;
        this._dataGrid.sortNodes(sortingFunction, this._dataGrid.sortOrder === "descending");
        this._timelineSortSelector.selectedIndex = 0;
    },

    _sortByTimeline: function()
    {
        if (this._timelineSortSelector.selectedIndex === 0)
            return;  // Do nothing - this is 'Timeline' caption.
        var selectedOption = this._timelineSortSelector[this._timelineSortSelector.selectedIndex];
        var value = selectedOption.value;

        var sortingFunction = this._sortingFunctions[value];
        this._dataGrid.sortNodes(sortingFunction);
        this.calculator = this._calculators[value];
        if (this.calculator.startAtZero)
            this._timelineGrid.hideEventDividers();
        else
            this._timelineGrid.showEventDividers();
        this._dataGrid.markColumnAsSortedBy("timeline", "ascending");
    },

    _createFilterStatusBarItems: function()
    {
        var filterBarElement = document.createElement("div");
        filterBarElement.className = "scope-bar status-bar-item";
        filterBarElement.id = "network-filter";

        function createFilterElement(category, label)
        {
            var categoryElement = document.createElement("li");
            categoryElement.category = category;
            categoryElement.className = category;
            categoryElement.appendChild(document.createTextNode(label));
            categoryElement.addEventListener("click", this._updateFilter.bind(this), false);
            filterBarElement.appendChild(categoryElement);

            return categoryElement;
        }

        this._filterAllElement = createFilterElement.call(this, "all", WebInspector.UIString("All"));

        // Add a divider
        var dividerElement = document.createElement("div");
        dividerElement.addStyleClass("scope-bar-divider");
        filterBarElement.appendChild(dividerElement);

        for (var category in this._categories)
            createFilterElement.call(this, category, this._categories[category].title);
        this._filterBarElement = filterBarElement;
    },

    _createSummaryBar: function()
    {
        this._summaryBarElement = document.createElement("div");
        this._summaryBarElement.className = "network-summary-bar";
        this.element.appendChild(this._summaryBarElement);
    },

    _updateSummaryBar: function()
    {
        this._positionSummaryBar(); // Grid is growing.
        var numRequests = this._resources.length;
        var transferSize = 0;
        var baseTime = -1;
        var maxTime = -1;
        for (var i = 0; i < this._resources.length; ++i) {
            var resource = this._resources[i];
            transferSize += resource.cached ? 0 : resource.transferSize;
            if (resource === WebInspector.mainResource)
                baseTime = resource.startTime;
            if (resource.endTime > maxTime) 
                maxTime = resource.endTime;
        }
        var text = String.sprintf(WebInspector.UIString("%d requests"), numRequests);
        text += "  \u2758  " + String.sprintf(WebInspector.UIString("%s transferred"), Number.bytesToString(transferSize));
        if (baseTime !== -1 && this._mainResourceLoadTime !== -1 && this._mainResourceDOMContentTime !== -1) {
            text += "  \u2758  " + String.sprintf(WebInspector.UIString("%s (onload: %s, DOMContentLoaded: %s)"),
                        Number.secondsToString(maxTime - baseTime),
                        Number.secondsToString(this._mainResourceLoadTime - baseTime),
                        Number.secondsToString(this._mainResourceDOMContentTime - baseTime));
        }
        this._summaryBarElement.textContent = text;
    },

    _showCategory: function(category)
    {
        this._dataGrid.element.addStyleClass("filter-" + category);
    },

    _hideCategory: function(category)
    {
        this._dataGrid.element.removeStyleClass("filter-" + category);
    },

    _updateFilter: function(e)
    {
        var isMac = WebInspector.isMac();
        var selectMultiple = false;
        if (isMac && e.metaKey && !e.ctrlKey && !e.altKey && !e.shiftKey)
            selectMultiple = true;
        if (!isMac && e.ctrlKey && !e.metaKey && !e.altKey && !e.shiftKey)
            selectMultiple = true;

        this._filter(e.target, selectMultiple);

        var searchField = document.getElementById("search");
        WebInspector.doPerformSearch(searchField.value, WebInspector.shortSearchWasForcedByKeyEvent, false, true);
    },

    _filter: function(target, selectMultiple)
    {
        function unselectAll()
        {
            for (var i = 0; i < this._filterBarElement.childNodes.length; ++i) {
                var child = this._filterBarElement.childNodes[i];
                if (!child.category)
                    continue;

                child.removeStyleClass("selected");
                this._hideCategory(child.category);
            }
        }

        if (target.category === this._filterAllElement) {
            if (target.hasStyleClass("selected")) {
                // We can't unselect All, so we break early here
                return;
            }

            // If All wasn't selected, and now is, unselect everything else.
            unselectAll.call(this);
        } else {
            // Something other than All is being selected, so we want to unselect All.
            if (this._filterAllElement.hasStyleClass("selected")) {
                this._filterAllElement.removeStyleClass("selected");
                this._hideCategory("all");
            }
        }

        if (!selectMultiple) {
            // If multiple selection is off, we want to unselect everything else
            // and just select ourselves.
            unselectAll.call(this);

            target.addStyleClass("selected");
            this._showCategory(target.category);
            return;
        }

        if (target.hasStyleClass("selected")) {
            // If selectMultiple is turned on, and we were selected, we just
            // want to unselect ourselves.
            target.removeStyleClass("selected");
            this._hideCategory(target.category);
        } else {
            // If selectMultiple is turned on, and we weren't selected, we just
            // want to select ourselves.
            target.addStyleClass("selected");
            this._showCategory(target.category);
        }
    },

    _scheduleRefresh: function()
    {
        if (this._needsRefresh)
            return;

        this._needsRefresh = true;

        if (this.visible && !("_refreshTimeout" in this))
            this._refreshTimeout = setTimeout(this.refresh.bind(this), 500);
    },

    _updateDividersIfNeeded: function(force)
    {
        this._timelineGrid.element.style.left = this._dataGrid.resizers[this._dataGrid.resizers.length - 1].style.left;
        this._timelineGrid.element.style.right = "18px";

        var proceed = true;
        if (!this.visible) {
            this._scheduleRefresh();
            proceed = false;
        } else
            proceed = this._timelineGrid.updateDividers(force, this.calculator);
        
        if (!proceed)
            return;

        if (this.calculator.startAtZero || !this.calculator.computePercentageFromEventTime) {
            // If our current sorting method starts at zero, that means it shows all
            // resources starting at the same point, and so onLoad event and DOMContent
            // event lines really wouldn't make much sense here, so don't render them.
            // Additionally, if the calculator doesn't have the computePercentageFromEventTime
            // function defined, we are probably sorting by size, and event times aren't relevant
            // in this case.
            return;
        }

        this._timelineGrid.removeEventDividers();
        if (this._mainResourceLoadTime !== -1) {
            var percent = this.calculator.computePercentageFromEventTime(this._mainResourceLoadTime);

            var loadDivider = document.createElement("div");
            loadDivider.className = "network-event-divider network-red-divider";

            var loadDividerPadding = document.createElement("div");
            loadDividerPadding.className = "network-event-divider-padding";
            loadDividerPadding.title = WebInspector.UIString("Load event fired");
            loadDividerPadding.appendChild(loadDivider);
            loadDividerPadding.style.left = percent + "%";
            this._timelineGrid.addEventDivider(loadDividerPadding);
        }
        
        if (this._mainResourceDOMContentTime !== -1) {
            var percent = this.calculator.computePercentageFromEventTime(this._mainResourceDOMContentTime);

            var domContentDivider = document.createElement("div");
            domContentDivider.className = "network-event-divider network-blue-divider";
            
            var domContentDividerPadding = document.createElement("div");
            domContentDividerPadding.className = "network-event-divider-padding";
            domContentDividerPadding.title = WebInspector.UIString("DOMContent event fired");
            domContentDividerPadding.appendChild(domContentDivider);
            domContentDividerPadding.style.left = percent + "%";
            this._timelineGrid.addEventDivider(domContentDividerPadding);
        }
    },

    _refreshIfNeeded: function()
    {
        if (this._needsRefresh)
            this.refresh();
    },

    _invalidateAllItems: function()
    {
        this._staleResources = this._resources.slice();
    },

    get calculator()
    {
        return this._calculator;
    },

    set calculator(x)
    {
        if (!x || this._calculator === x)
            return;

        this._calculator = x;
        this._calculator.reset();

        this._invalidateAllItems();
        this.refresh();
    },

    _resourceGridNode: function(resource)
    {
        return this._resourceGridNodes[resource.identifier];
    },

    revealAndSelectItem: function(resource)
    {
        var node = this._resourceGridNode(resource);
        if (node) {
            node.reveal();
            node.select(true);
        }
    },

    addEventDivider: function(divider)
    {
        this._timelineGrid.addEventDivider(divider);
    },

    get resourceTrackingEnabled()
    {
        return this._resourceTrackingEnabled;
    },

    _createStatusbarButtons: function()
    {
        this._clearButton = new WebInspector.StatusBarButton(WebInspector.UIString("Clear"), "clear-status-bar-item");
        this._clearButton.addEventListener("click", this.reset.bind(this), false);

        this._largerResourcesButton = new WebInspector.StatusBarButton(WebInspector.UIString("Use small resource rows."), "network-larger-resources-status-bar-item");
        WebInspector.applicationSettings.addEventListener("loaded", this._settingsLoaded, this);
        this._largerResourcesButton.addEventListener("click", this._toggleLargerResources.bind(this), false);
    },

    _settingsLoaded: function()
    {
        this._largerResourcesButton.toggled = WebInspector.applicationSettings.resourcesLargeRows;
        if (!WebInspector.applicationSettings.resourcesLargeRows)
            this._setLargerResources(WebInspector.applicationSettings.resourcesLargeRows);
    },

    set mainResourceLoadTime(x)
    {
        if (this._mainResourceLoadTime === x)
            return;
        
        this._mainResourceLoadTime = x || -1;
        // Update the dividers to draw the new line
        this._updateDividersIfNeeded(true);
    },

    set mainResourceDOMContentTime(x)
    {
        if (this._mainResourceDOMContentTime === x)
            return;

        this._mainResourceDOMContentTime = x || -1;
        this._updateDividersIfNeeded(true);
    },

    show: function()
    {
        WebInspector.Panel.prototype.show.call(this);

        this._refreshIfNeeded();

        var visibleView = this.visibleView;
        if (this.visibleResource) {
            this.visibleView.headersVisible = true;
            this.visibleView.show(this._viewsContainerElement);
        } else if (visibleView)
            visibleView.show();

        // Hide any views that are visible that are not this panel's current visible view.
        // This can happen when a ResourceView is visible in the Scripts panel then switched
        // to the this panel.
        var resourcesLength = this._resources.length;
        for (var i = 0; i < resourcesLength; ++i) {
            var resource = this._resources[i];
            var view = resource._resourcesView;
            if (!view || view === visibleView)
                continue;
            view.visible = false;
        }
        this._dataGrid.updateWidths();
        this._positionSummaryBar();
    },

    get searchableViews()
    {
        var views = [];

        const visibleView = this.visibleView;
        if (visibleView && visibleView.performSearch)
            views.push(visibleView);

        var resourcesLength = this._resources.length;
        for (var i = 0; i < resourcesLength; ++i) {
            var resource = this._resources[i];
            if (!this._resourceGridNode(resource) || !this._resourceGridNode(resource).selectable)
                continue;
            var resourceView = this.resourceViewForResource(resource);
            if (!resourceView.performSearch || resourceView === visibleView)
                continue;
            views.push(resourceView);
        }

        return views;
    },

    searchMatchFound: function(view, matches)
    {
        this._resourceGridNode(view.resource).searchMatches = matches;
    },

    searchCanceled: function(startingNewSearch)
    {
        WebInspector.Panel.prototype.searchCanceled.call(this, startingNewSearch);

        if (startingNewSearch || !this._resources)
            return;
    },

    performSearch: function(query)
    {
        WebInspector.Panel.prototype.performSearch.call(this, query);
    },

    get visibleView()
    {
        if (this.visibleResource)
            return this.visibleResource._resourcesView;
        return null;
    },

    refresh: function()
    {
        this._needsRefresh = false;
        if ("_refreshTimeout" in this) {
            clearTimeout(this._refreshTimeout);
            delete this._refreshTimeout;
        }

        var staleItemsLength = this._staleResources.length;

        var boundariesChanged = false;

        for (var i = 0; i < staleItemsLength; ++i) {
            var resource = this._staleResources[i];
            var node = this._resourceGridNode(resource);
            if (!node) {
                // Create the timeline tree element and graph.
                node = new WebInspector.NetworkDataGridNode(resource);
                this._resourceGridNodes[resource.identifier] = node;
                this._dataGrid.appendChild(node);
            }
            node.refreshResource();

            if (this.calculator.updateBoundaries(resource))
                boundariesChanged = true;
        }

        if (boundariesChanged) {
            // The boundaries changed, so all item graphs are stale.
            this._invalidateAllItems();
            staleItemsLength = this._staleResources.length;
        }

        for (var i = 0; i < staleItemsLength; ++i)
            this._resourceGridNode(this._staleResources[i]).refreshGraph(this.calculator);

        this._staleResources = [];
        this._sortItems();
        this._updateSummaryBar();
        this._dataGrid.updateWidths();
    },

    reset: function()
    {
        this._popoverHelper.hidePopup();
        this.closeVisibleResource();

        delete this.currentQuery;
        this.searchCanceled();

        if (this._resources) {
            var resourcesLength = this._resources.length;
            for (var i = 0; i < resourcesLength; ++i) {
                var resource = this._resources[i];

                resource.warnings = 0;
                resource.errors = 0;

                delete resource._resourcesView;
            }
        }

        // Begin reset timeline
        this.containerElement.scrollTop = 0;

        if (this._calculator)
            this._calculator.reset();

        if (this._resources) {
            var itemsLength = this._resources.length;
            for (var i = 0; i < itemsLength; ++i) {
                var item = this._resources[i];
            }
        }

        this._resources = [];
        this._staleResources = [];
        this._resourceGridNodes = {};

        this._dataGrid.removeChildren();
        delete this._summaryBarRowNode;
        this._updateDividersIfNeeded(true);
        // End reset timeline.

        this._mainResourceLoadTime = -1;
        this._mainResourceDOMContentTime = -1;
 
        this._viewsContainerElement.removeChildren();
    },

    addResource: function(resource)
    {
        this._resources.push(resource);
        this.refreshResource(resource);
    },

    refreshResource: function(resource)
    {
        this._staleResources.push(resource);
        this._scheduleRefresh();
    },

    recreateViewForResourceIfNeeded: function(resource)
    {
        if (!resource || !resource._resourcesView)
            return;

        var newView = this._createResourceView(resource);
        if (newView.__proto__ === resource._resourcesView.__proto__)
            return;

        if (!this.currentQuery && this._resourceGridNode(resource))
            this._resourceGridNode(resource).updateErrorsAndWarnings();

        var oldView = resource._resourcesView;
        var oldViewParentNode = oldView.visible ? oldView.element.parentNode : null;

        resource._resourcesView.detach();
        delete resource._resourcesView;

        resource._resourcesView = newView;

        newView.headersVisible = oldView.headersVisible;

        if (oldViewParentNode)
            newView.show(oldViewParentNode);

        WebInspector.panels.scripts.viewRecreated(oldView, newView);
    },

    canShowSourceLine: function(url, line)
    {
        return this._resourceTrackingEnabled && !!WebInspector.resourceForURL(url);
    },

    showSourceLine: function(url, line)
    {
        this.showResource(WebInspector.resourceForURL(url), line);
    },

    showResource: function(resource, line)
    {
        if (!resource)
            return;

        this._popoverHelper.hidePopup();

        this.containerElement.addStyleClass("viewing-resource");

        if (this.visibleResource && this.visibleResource._resourcesView)
            this.visibleResource._resourcesView.hide();

        var view = this.resourceViewForResource(resource);
        view.headersVisible = true;
        view.show(this._viewsContainerElement);

        if (line) {
            view.selectContentTab(true);
            if (view.revealLine)
                view.revealLine(line);
            if (view.highlightLine)
                view.highlightLine(line);
        }

        this.revealAndSelectItem(resource);

        this.visibleResource = resource;

        this.updateSidebarWidth();
    },

    showView: function(view)
    {
        if (!view)
            return;
        this.showResource(view.resource);
    },

    closeVisibleResource: function()
    {
        this.containerElement.removeStyleClass("viewing-resource");

        if (this.visibleResource && this.visibleResource._resourcesView)
            this.visibleResource._resourcesView.hide();
        delete this.visibleResource;

        if (this._lastSelectedGraphTreeElement)
            this._lastSelectedGraphTreeElement.select(true);

        this.updateSidebarWidth();
    },

    resourceViewForResource: function(resource)
    {
        if (!resource)
            return null;
        if (!resource._resourcesView)
            resource._resourcesView = this._createResourceView(resource);
        return resource._resourcesView;
    },

    sourceFrameForResource: function(resource)
    {
        var view = this.resourceViewForResource(resource);
        if (!view)
            return null;

        if (!view.setupSourceFrameIfNeeded)
            return null;

        // Setting up the source frame requires that we be attached.
        if (!this.element.parentNode)
            this.attach();

        view.setupSourceFrameIfNeeded();
        return view.sourceFrame;
    },

    _toggleLargerResources: function()
    {
        WebInspector.applicationSettings.resourcesLargeRows = !WebInspector.applicationSettings.resourcesLargeRows;
        this._setLargerResources(WebInspector.applicationSettings.resourcesLargeRows);
    },

    _setLargerResources: function(enabled)
    {
        this._largerResourcesButton.toggled = enabled;
        if (!enabled) {
            this._largerResourcesButton.title = WebInspector.UIString("Use large resource rows.");
            this._dataGrid.element.addStyleClass("small");
            this._timelineGrid.element.addStyleClass("small");
        } else {
            this._largerResourcesButton.title = WebInspector.UIString("Use small resource rows.");
            this._dataGrid.element.removeStyleClass("small");
            this._timelineGrid.element.removeStyleClass("small");
        }
    },

    _createResourceView: function(resource)
    {
        switch (resource.category) {
            case WebInspector.resourceCategories.documents:
            case WebInspector.resourceCategories.stylesheets:
            case WebInspector.resourceCategories.scripts:
            case WebInspector.resourceCategories.xhr:
                return new WebInspector.SourceView(resource);
            case WebInspector.resourceCategories.images:
                return new WebInspector.ImageView(resource);
            case WebInspector.resourceCategories.fonts:
                return new WebInspector.FontView(resource);
            default:
                return new WebInspector.ResourceView(resource);
        }
    },

    _getPopoverAnchor: function(element)
    {
        var anchor = element.enclosingNodeOrSelfWithClass("network-graph-bar") || element.enclosingNodeOrSelfWithClass("network-graph-label");
        if (!anchor)
            return null;
        var resource = anchor.parentElement.resource;
        return resource && resource.timing ? anchor : null;
    },

    _showPopover: function(anchor)
    {
        var tableElement = document.createElement("table");
        var resource = anchor.parentElement.resource;
        var rows = [];

        function addRow(title, start, end, color)
        {
            var row = {};
            row.title = title;
            row.start = start;
            row.end = end;
            rows.push(row);
        }

        if (resource.timing.proxyStart !== -1)
            addRow(WebInspector.UIString("Proxy"), resource.timing.proxyStart, resource.timing.proxyEnd);

        if (resource.timing.dnsStart !== -1)
            addRow(WebInspector.UIString("DNS Lookup"), resource.timing.dnsStart, resource.timing.dnsEnd);

        if (resource.timing.connectStart !== -1) {
            if (resource.connectionReused)
                addRow(WebInspector.UIString("Blocking"), resource.timing.connectStart, resource.timing.connectEnd);
            else {
                var connectStart = resource.timing.connectStart;
                // Connection includes DNS, subtract it here.
                if (resource.timing.dnsStart !== -1)
                    connectStart += resource.timing.dnsEnd - resource.timing.dnsStart;
                addRow(WebInspector.UIString("Connecting"), connectStart, resource.timing.connectEnd);
            }
        }

        if (resource.timing.sslStart !== -1)
            addRow(WebInspector.UIString("SSL"), resource.timing.sslStart, resource.timing.sslEnd);

        var sendStart = resource.timing.sendStart;
        if (resource.timing.sslStart !== -1)
            sendStart += resource.timing.sslEnd - resource.timing.sslStart;
        
        addRow(WebInspector.UIString("Sending"), resource.timing.sendStart, resource.timing.sendEnd);
        addRow(WebInspector.UIString("Waiting"), resource.timing.sendEnd, resource.timing.receiveHeadersEnd);
        addRow(WebInspector.UIString("Receiving"), (resource.responseReceivedTime - resource.timing.requestTime) * 1000, (resource.endTime - resource.timing.requestTime) * 1000);

        const chartWidth = 200;
        var total = (resource.endTime - resource.timing.requestTime) * 1000;
        var scale = chartWidth / total;

        for (var i = 0; i < rows.length; ++i) {
            var tr = document.createElement("tr");
            tableElement.appendChild(tr);

            var td = document.createElement("td");
            td.textContent = rows[i].title;
            tr.appendChild(td);

            td = document.createElement("td");
            td.width = chartWidth + "px";

            var row = document.createElement("div");
            row.className = "network-timing-row";
            td.appendChild(row);

            var bar = document.createElement("span");
            bar.className = "network-timing-bar";
            bar.style.left = scale * rows[i].start + "px";
            bar.style.right = scale * (total - rows[i].end) + "px";
            bar.style.backgroundColor = rows[i].color;
            bar.textContent = "\u200B"; // Important for 0-time items to have 0 width.
            row.appendChild(bar);

            var title = document.createElement("span");
            title.className = "network-timing-bar-title";
            if (total - rows[i].end < rows[i].start)
                title.style.right = (scale * (total - rows[i].end) + 3) + "px";
            else
                title.style.left = (scale * rows[i].start + 3) + "px";
            title.textContent = Number.millisToString(rows[i].end - rows[i].start);
            row.appendChild(title);

            tr.appendChild(td);
        }

        var popover = new WebInspector.Popover(tableElement);
        popover.show(anchor);
        return popover;
    },

    hide: function()
    {
        WebInspector.Panel.prototype.hide.call(this);
        this._popoverHelper.hidePopup();
    }
}

WebInspector.NetworkPanel.prototype.__proto__ = WebInspector.Panel.prototype;

WebInspector.getResourceContent = function(identifier, callback)
{
    InspectorBackend.getResourceContent(identifier, false, callback);
}

WebInspector.NetworkBaseCalculator = function()
{
}

WebInspector.NetworkBaseCalculator.prototype = {
    computeSummaryValues: function(items)
    {
        var total = 0;
        var categoryValues = {};

        var itemsLength = items.length;
        for (var i = 0; i < itemsLength; ++i) {
            var item = items[i];
            var value = this._value(item);
            if (typeof value === "undefined")
                continue;
            if (!(item.category.name in categoryValues))
                categoryValues[item.category.name] = 0;
            categoryValues[item.category.name] += value;
            total += value;
        }

        return {categoryValues: categoryValues, total: total};
    },

    computeBarGraphPercentages: function(item)
    {
        return {start: 0, middle: 0, end: (this._value(item) / this.boundarySpan) * 100};
    },

    computeBarGraphLabels: function(item)
    {
        const label = this.formatValue(this._value(item));
        return {left: label, right: label, tooltip: label};
    },

    get boundarySpan()
    {
        return this.maximumBoundary - this.minimumBoundary;
    },

    updateBoundaries: function(item)
    {
        this.minimumBoundary = 0;

        var value = this._value(item);
        if (typeof this.maximumBoundary === "undefined" || value > this.maximumBoundary) {
            this.maximumBoundary = value;
            return true;
        }
        return false;
    },

    reset: function()
    {
        delete this.minimumBoundary;
        delete this.maximumBoundary;
    },

    _value: function(item)
    {
        return 0;
    },

    formatValue: function(value)
    {
        return value.toString();
    }
}

WebInspector.NetworkTimeCalculator = function(startAtZero)
{
    WebInspector.NetworkBaseCalculator.call(this);
    this.startAtZero = startAtZero;
}

WebInspector.NetworkTimeCalculator.prototype = {
    computeSummaryValues: function(resources)
    {
        var resourcesByCategory = {};
        var resourcesLength = resources.length;
        for (var i = 0; i < resourcesLength; ++i) {
            var resource = resources[i];
            if (!(resource.category.name in resourcesByCategory))
                resourcesByCategory[resource.category.name] = [];
            resourcesByCategory[resource.category.name].push(resource);
        }

        var earliestStart;
        var latestEnd;
        var categoryValues = {};
        for (var category in resourcesByCategory) {
            resourcesByCategory[category].sort(WebInspector.Resource.CompareByTime);
            categoryValues[category] = 0;

            var segment = {start: -1, end: -1};

            var categoryResources = resourcesByCategory[category];
            var resourcesLength = categoryResources.length;
            for (var i = 0; i < resourcesLength; ++i) {
                var resource = categoryResources[i];
                if (resource.startTime === -1 || resource.endTime === -1)
                    continue;

                if (typeof earliestStart === "undefined")
                    earliestStart = resource.startTime;
                else
                    earliestStart = Math.min(earliestStart, resource.startTime);

                if (typeof latestEnd === "undefined")
                    latestEnd = resource.endTime;
                else
                    latestEnd = Math.max(latestEnd, resource.endTime);

                if (resource.startTime <= segment.end) {
                    segment.end = Math.max(segment.end, resource.endTime);
                    continue;
                }

                categoryValues[category] += segment.end - segment.start;

                segment.start = resource.startTime;
                segment.end = resource.endTime;
            }

            // Add the last segment
            categoryValues[category] += segment.end - segment.start;
        }

        return {categoryValues: categoryValues, total: latestEnd - earliestStart};
    },

    computeBarGraphPercentages: function(resource)
    {
        if (resource.startTime !== -1)
            var start = ((resource.startTime - this.minimumBoundary) / this.boundarySpan) * 100;
        else
            var start = 0;

        if (resource.responseReceivedTime !== -1)
            var middle = ((resource.responseReceivedTime - this.minimumBoundary) / this.boundarySpan) * 100;
        else
            var middle = (this.startAtZero ? start : 100);

        if (resource.endTime !== -1)
            var end = ((resource.endTime - this.minimumBoundary) / this.boundarySpan) * 100;
        else
            var end = (this.startAtZero ? middle : 100);

        if (this.startAtZero) {
            end -= start;
            middle -= start;
            start = 0;
        }

        return {start: start, middle: middle, end: end};
    },
    
    computePercentageFromEventTime: function(eventTime)
    {
        // This function computes a percentage in terms of the total loading time
        // of a specific event. If startAtZero is set, then this is useless, and we
        // want to return 0.
        if (eventTime !== -1 && !this.startAtZero)
            return ((eventTime - this.minimumBoundary) / this.boundarySpan) * 100;

        return 0;
    },

    computeBarGraphLabels: function(resource)
    {
        var rightLabel = "";
        if (resource.responseReceivedTime !== -1 && resource.endTime !== -1)
            rightLabel = this.formatValue(resource.endTime - resource.responseReceivedTime);

        var hasLatency = resource.latency > 0;
        if (hasLatency)
            var leftLabel = this.formatValue(resource.latency);
        else
            var leftLabel = rightLabel;

        if (resource.timing)
            return {left: leftLabel, right: rightLabel};

        if (hasLatency && rightLabel) {
            var total = this.formatValue(resource.duration);
            var tooltip = WebInspector.UIString("%s latency, %s download (%s total)", leftLabel, rightLabel, total);
        } else if (hasLatency)
            var tooltip = WebInspector.UIString("%s latency", leftLabel);
        else if (rightLabel)
            var tooltip = WebInspector.UIString("%s download", rightLabel);

        if (resource.cached)
            tooltip = WebInspector.UIString("%s (from cache)", tooltip);
        return {left: leftLabel, right: rightLabel, tooltip: tooltip};
    },

    updateBoundaries: function(resource)
    {
        var didChange = false;

        var lowerBound;
        if (this.startAtZero)
            lowerBound = 0;
        else
            lowerBound = this._lowerBound(resource);

        if (lowerBound !== -1 && (typeof this.minimumBoundary === "undefined" || lowerBound < this.minimumBoundary)) {
            this.minimumBoundary = lowerBound;
            didChange = true;
        }

        var upperBound = this._upperBound(resource);
        if (upperBound !== -1 && (typeof this.maximumBoundary === "undefined" || upperBound > this.maximumBoundary)) {
            this.maximumBoundary = upperBound;
            didChange = true;
        }

        return didChange;
    },

    formatValue: function(value)
    {
        return Number.secondsToString(value, WebInspector.UIString);
    },

    _lowerBound: function(resource)
    {
        return 0;
    },

    _upperBound: function(resource)
    {
        return 0;
    }
}

WebInspector.NetworkTimeCalculator.prototype.__proto__ = WebInspector.NetworkBaseCalculator.prototype;

WebInspector.NetworkTransferTimeCalculator = function()
{
    WebInspector.NetworkTimeCalculator.call(this, false);
}

WebInspector.NetworkTransferTimeCalculator.prototype = {
    formatValue: function(value)
    {
        return Number.secondsToString(value, WebInspector.UIString);
    },

    _lowerBound: function(resource)
    {
        return resource.startTime;
    },

    _upperBound: function(resource)
    {
        return resource.endTime;
    }
}

WebInspector.NetworkTransferTimeCalculator.prototype.__proto__ = WebInspector.NetworkTimeCalculator.prototype;

WebInspector.NetworkTransferDurationCalculator = function()
{
    WebInspector.NetworkTimeCalculator.call(this, true);
}

WebInspector.NetworkTransferDurationCalculator.prototype = {
    formatValue: function(value)
    {
        return Number.secondsToString(value, WebInspector.UIString);
    },

    _upperBound: function(resource)
    {
        return resource.duration;
    }
}

WebInspector.NetworkTransferDurationCalculator.prototype.__proto__ = WebInspector.NetworkTimeCalculator.prototype;

WebInspector.NetworkDataGridNode = function(resource)
{
    WebInspector.DataGridNode.call(this, {});
    this._resource = resource;
}

WebInspector.NetworkDataGridNode.prototype = {
    createCells: function()
    {
        this._nameCell = this._createDivInTD("name");
        this._methodCell = this._createDivInTD("method");
        this._statusCell = this._createDivInTD("status");
        this._typeCell = this._createDivInTD("type");
        this._sizeCell = this._createDivInTD("size");
        this._timeCell = this._createDivInTD("time");
        this._createTimelineCell();
    },

    _createDivInTD: function(columnIdentifier) {
        var td = document.createElement("td");
        td.className = columnIdentifier + "-column";
        var div = document.createElement("div");
        td.appendChild(div);
        this._element.appendChild(td);
        return div;
    },


    _createTimelineCell: function()
    {
        this._graphElement = document.createElement("div");
        this._graphElement.className = "network-graph-side";
        this._graphElement.addEventListener("mouseover", this._refreshLabelPositions.bind(this), false);

        this._barAreaElement = document.createElement("div");
        //    this._barAreaElement.className = "network-graph-bar-area hidden";
        this._barAreaElement.className = "network-graph-bar-area";
        this._barAreaElement.resource = this._resource;
        this._graphElement.appendChild(this._barAreaElement);

        this._barLeftElement = document.createElement("div");
        this._barLeftElement.className = "network-graph-bar waiting";
        this._barAreaElement.appendChild(this._barLeftElement);

        this._barRightElement = document.createElement("div");
        this._barRightElement.className = "network-graph-bar";
        this._barAreaElement.appendChild(this._barRightElement);

        this._labelLeftElement = document.createElement("div");
        this._labelLeftElement.className = "network-graph-label waiting";
        this._barAreaElement.appendChild(this._labelLeftElement);

        this._labelRightElement = document.createElement("div");
        this._labelRightElement.className = "network-graph-label";
        this._barAreaElement.appendChild(this._labelRightElement);

        this._timelineCell = document.createElement("td");
        this._element.appendChild(this._timelineCell);
        this._timelineCell.appendChild(this._graphElement);
    },

    refreshResource: function()
    {
        this._refreshNameCell();

        this._methodCell.textContent = this._resource.requestMethod;

        this._refreshStatusCell();

        if (this._resource.mimeType) {
            this._typeCell.removeStyleClass("network-dim-cell");
            this._typeCell.textContent = this._resource.mimeType;
        } else {
            this._typeCell.addStyleClass("network-dim-cell");
            this._typeCell.textContent = WebInspector.UIString("Pending");
        }

        this._refreshSizeCell();
        this._refreshTimeCell();

        if (this._resource.cached)
            this._graphElement.addStyleClass("resource-cached");

        if (!this._element.hasStyleClass("network-category-" + this._resource.category.name)) {
            this._element.removeMatchingStyleClasses("network-category-\\w+");
            this._element.addStyleClass("network-category-" + this._resource.category.name);
        }
    },

    _refreshNameCell: function()
    {
        this._nameCell.removeChildren();

        if (this._resource.category === WebInspector.resourceCategories.images) {
            var previewImage = document.createElement("img");
            previewImage.className = "image-network-icon-preview";
            previewImage.src = this._resource.url;

            var iconElement = document.createElement("div");
            iconElement.className = "icon";
            iconElement.appendChild(previewImage);
        } else {
            var iconElement = document.createElement("img");
            iconElement.className = "icon";
        }
        this._nameCell.appendChild(iconElement);
        this._nameCell.appendChild(document.createTextNode(this._fileName()));


        var subtitle = this._resource.displayDomain;

        if (this._resource.path && this._resource.lastPathComponent) {
            var lastPathComponentIndex = this._resource.path.lastIndexOf("/" + this._resource.lastPathComponent);
            if (lastPathComponentIndex != -1)
                subtitle += this._resource.path.substring(0, lastPathComponentIndex);
        }

        this._appendSubtitle(this._nameCell, subtitle);
        this._nameCell.title = this._resource.url;
    },

    _fileName: function()
    {
        var fileName = this._resource.displayName;
        if (this._resource.queryString)
            fileName += "?" + this._resource.queryString;
        return fileName;
    },

    _refreshStatusCell: function()
    {
        this._statusCell.removeChildren();

        if (this._resource.statusCode) {
            this._statusCell.appendChild(document.createTextNode(this._resource.statusCode));
            this._statusCell.removeStyleClass("network-dim-cell");
            this._appendSubtitle(this._statusCell, this._resource.statusText);
            this._statusCell.title = this._resource.statusCode + " " + this._resource.statusText;
        } else {
            this._statusCell.addStyleClass("network-dim-cell");
            this._statusCell.textContent = WebInspector.UIString("Pending");
        }
    },

    _refreshSizeCell: function()
    {
        var resourceSize = typeof this._resource.resourceSize === "number" ? Number.bytesToString(this._resource.resourceSize) : "?";
        var transferSize = typeof this._resource.transferSize === "number" ? Number.bytesToString(this._resource.transferSize) : "?";
        var fromCache = this._resource.cached;
        this._sizeCell.textContent = !fromCache ? resourceSize : WebInspector.UIString("(from cache)");
        if (fromCache)
            this._sizeCell.addStyleClass("network-dim-cell");
        else
            this._sizeCell.removeStyleClass("network-dim-cell");
        if (!fromCache)
            this._appendSubtitle(this._sizeCell, transferSize);
    },

    _refreshTimeCell: function()
    {
        if (this._resource.duration > 0) {
            this._timeCell.removeStyleClass("network-dim-cell");
            this._timeCell.textContent = Number.secondsToString(this._resource.duration);
            this._appendSubtitle(this._timeCell, Number.secondsToString(this._resource.latency));
        } else {
            this._timeCell.addStyleClass("network-dim-cell");
            this._timeCell.textContent = WebInspector.UIString("Pending");
        }
    },

    _appendSubtitle: function(cellElement, subtitleText)
    {
        var subtitleElement = document.createElement("div");
        subtitleElement.className = "network-cell-subtitle";
        subtitleElement.textContent = subtitleText;
        cellElement.appendChild(subtitleElement);
    },

    refreshGraph: function(calculator)
    {
        var percentages = calculator.computeBarGraphPercentages(this._resource);
        var labels = calculator.computeBarGraphLabels(this._resource);

        this._percentages = percentages;

        this._barAreaElement.removeStyleClass("hidden");

        if (!this._graphElement.hasStyleClass("network-category-" + this._resource.category.name)) {
            this._graphElement.removeMatchingStyleClasses("network-category-\\w+");
            this._graphElement.addStyleClass("network-category-" + this._resource.category.name);
        }

        this._barLeftElement.style.setProperty("left", percentages.start + "%");
        this._barRightElement.style.setProperty("right", (100 - percentages.end) + "%");

        this._barLeftElement.style.setProperty("right", (100 - percentages.end) + "%");
        this._barRightElement.style.setProperty("left", percentages.middle + "%");

        this._labelLeftElement.textContent = labels.left;
        this._labelRightElement.textContent = labels.right;

        var tooltip = (labels.tooltip || "");
        this._barLeftElement.title = tooltip;
        this._labelLeftElement.title = tooltip;
        this._labelRightElement.title = tooltip;
        this._barRightElement.title = tooltip;
    },

    _refreshLabelPositions: function()
    {
        this._labelLeftElement.style.removeProperty("left");
        this._labelLeftElement.style.removeProperty("right");
        this._labelLeftElement.removeStyleClass("before");
        this._labelLeftElement.removeStyleClass("hidden");

        this._labelRightElement.style.removeProperty("left");
        this._labelRightElement.style.removeProperty("right");
        this._labelRightElement.removeStyleClass("after");
        this._labelRightElement.removeStyleClass("hidden");

        const labelPadding = 10;
        const barRightElementOffsetWidth = this._barRightElement.offsetWidth;
        const barLeftElementOffsetWidth = this._barLeftElement.offsetWidth;

        if (this._barLeftElement) {
            var leftBarWidth = barLeftElementOffsetWidth - labelPadding;
            var rightBarWidth = (barRightElementOffsetWidth - barLeftElementOffsetWidth) - labelPadding;
        } else {
            var leftBarWidth = (barLeftElementOffsetWidth - barRightElementOffsetWidth) - labelPadding;
            var rightBarWidth = barRightElementOffsetWidth - labelPadding;
        }

        const labelLeftElementOffsetWidth = this._labelLeftElement.offsetWidth;
        const labelRightElementOffsetWidth = this._labelRightElement.offsetWidth;

        const labelBefore = (labelLeftElementOffsetWidth > leftBarWidth);
        const labelAfter = (labelRightElementOffsetWidth > rightBarWidth);
        const graphElementOffsetWidth = this._graphElement.offsetWidth;

        if (labelBefore && (graphElementOffsetWidth * (this._percentages.start / 100)) < (labelLeftElementOffsetWidth + 10))
            var leftHidden = true;

        if (labelAfter && (graphElementOffsetWidth * ((100 - this._percentages.end) / 100)) < (labelRightElementOffsetWidth + 10))
            var rightHidden = true;

        if (barLeftElementOffsetWidth == barRightElementOffsetWidth) {
            // The left/right label data are the same, so a before/after label can be replaced by an on-bar label.
            if (labelBefore && !labelAfter)
                leftHidden = true;
            else if (labelAfter && !labelBefore)
                rightHidden = true;
        }

        if (labelBefore) {
            if (leftHidden)
                this._labelLeftElement.addStyleClass("hidden");
            this._labelLeftElement.style.setProperty("right", (100 - this._percentages.start) + "%");
            this._labelLeftElement.addStyleClass("before");
        } else {
            this._labelLeftElement.style.setProperty("left", this._percentages.start + "%");
            this._labelLeftElement.style.setProperty("right", (100 - this._percentages.middle) + "%");
        }

        if (labelAfter) {
            if (rightHidden)
                this._labelRightElement.addStyleClass("hidden");
            this._labelRightElement.style.setProperty("left", this._percentages.end + "%");
            this._labelRightElement.addStyleClass("after");
        } else {
            this._labelRightElement.style.setProperty("left", this._percentages.middle + "%");
            this._labelRightElement.style.setProperty("right", (100 - this._percentages.end) + "%");
        }
    }
}

WebInspector.NetworkDataGridNode.NameComparator = function(a, b)
{
    var aFileName = a._resource.displayName + (a._resource.queryString ? a._resource.queryString : "");
    var bFileName = b._resource.displayName + (b._resource.queryString ? b._resource.queryString : "");
    if (aFileName > bFileName)
        return 1;
    if (bFileName > aFileName)
        return -1;
    return 0;
}

WebInspector.NetworkDataGridNode.SizeComparator = function(a, b)
{
    if (b._resource.cached && !a._resource.cached)
        return 1;
    if (a._resource.cached && !b._resource.cached)
        return -1;

    if (a._resource.resourceSize === b._resource.resourceSize)
        return 0;

    return a._resource.resourceSize - b._resource.resourceSize;
}

WebInspector.NetworkDataGridNode.ResourcePropertyComparator = function(propertyName, revert, a, b)
{
    var aValue = a._resource[propertyName];
    var bValue = b._resource[propertyName];
    if (aValue > bValue)
        return revert ? -1 : 1;
    if (bValue > aValue)
        return revert ? 1 : -1;
    return 0;
}

WebInspector.NetworkDataGridNode.prototype.__proto__ = WebInspector.DataGridNode.prototype;

WebInspector.NetworkTotalGridNode = function(element)
{
    this._summaryBarElement = element;
    WebInspector.DataGridNode.call(this, {summaryRow: true});
}

WebInspector.NetworkTotalGridNode.prototype = {
    createCells: function()
    {
        var td = document.createElement("td");
        td.setAttribute("colspan", 7);
        td.className = "network-summary";
        td.appendChild(this._summaryBarElement);
        this._element.appendChild(td);
    }
}

WebInspector.NetworkTotalGridNode.prototype.__proto__ = WebInspector.DataGridNode.prototype;
