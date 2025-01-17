// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.contextual_suggestions;

import android.content.Context;
import android.support.annotation.Nullable;
import android.text.TextUtils;

import org.chromium.base.ContextUtils;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.fullscreen.ChromeFullscreenManager;
import org.chromium.chrome.browser.fullscreen.ChromeFullscreenManager.FullscreenListener;
import org.chromium.chrome.browser.ntp.snippets.SnippetArticle;
import org.chromium.chrome.browser.ntp.snippets.SnippetsBridge;
import org.chromium.chrome.browser.ntp.snippets.SuggestionsSource;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.tabmodel.TabModelSelector;
import org.chromium.chrome.browser.util.MathUtils;
import org.chromium.ui.widget.Toast;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * A mediator for the contextual suggestions UI component responsible for interacting with
 * the contextual suggestions C++ components (via a bridge), updating the model, and communicating
 * with the component coordinator(s).
 */
class ContextualSuggestionsMediator implements FetchHelper.Delegate {
    private final Context mContext;
    private final ContextualSuggestionsCoordinator mCoordinator;
    private final ContextualSuggestionsModel mModel;
    private final SnippetsBridge mBridge;
    private final FetchHelper mFetchHelper;
    private final ChromeFullscreenManager mFullscreenManager;

    private boolean mDidSuggestionsShowForTab;

    @Nullable
    private String mCurrentRequestUrl;

    /**
     * Construct a new {@link ContextualSuggestionsMediator}.
     * @param context The {@link Context} used to retrieve resources.
     * @param profile The regular {@link Profile}.
     * @param tabModelSelector The {@link TabModelSelector} for the containing activity.
     * @param fullscreenManager The {@link ChromeFullscreenManager} to listen for browser controls
     *                          events.
     * @param coordinator The {@link ContextualSuggestionsCoordinator} for the component.
     * @param model The {@link ContextualSuggestionsModel} for the component.
     */
    ContextualSuggestionsMediator(Context context, Profile profile,
            TabModelSelector tabModelSelector, ChromeFullscreenManager fullscreenManager,
            ContextualSuggestionsCoordinator coordinator, ContextualSuggestionsModel model) {
        mContext = context;
        mCoordinator = coordinator;
        mModel = model;
        mFullscreenManager = fullscreenManager;

        mBridge = new SnippetsBridge(Profile.getLastUsedProfile());
        mFetchHelper = new FetchHelper(this, tabModelSelector);

        fullscreenManager.addListener(new FullscreenListener() {
            @Override
            public void onContentOffsetChanged(float offset) {}

            @Override
            public void onControlsOffsetChanged(
                    float topOffset, float bottomOffset, boolean needsAnimate) {
                // When the controls scroll completely off-screen, the suggestions are "shown" but
                // remain hidden since their offset from the bottom of the screen is determined by
                // the top controls.
                if (!mDidSuggestionsShowForTab && mModel.hasSuggestions()
                        && areBrowserControlsHidden()) {
                    mDidSuggestionsShowForTab = true;
                    mCoordinator.showSuggestions();
                }
            }

            @Override
            public void onToggleOverlayVideoMode(boolean enabled) {}

            @Override
            public void onBottomControlsHeightChanged(int bottomControlsHeight) {}
        });
    }

    /**
     * @return The {@link SuggestionsSource} used to fetch suggestions.
     *
     * TODO(twellington): This method is needed to construct {@link SuggestionsUiDelegateImpl} in
     * the coordinator. Try to remove this dependency.
     */
    SuggestionsSource getSuggestionsSource() {
        return mBridge;
    }

    /** Destroys the mediator. */
    void destroy() {
        mFetchHelper.destroy();
    }

    /**
     * @return Whether the browser controls are currently completely hidden.
     */
    private boolean areBrowserControlsHidden() {
        return MathUtils.areFloatsEqual(-mFullscreenManager.getTopControlOffset(),
                mFullscreenManager.getTopControlsHeight());
    }

    @Override
    public void requestSuggestions(String url) {
        mCurrentRequestUrl = url;
        mBridge.fetchContextualSuggestions(url, (suggestions) -> {
            // Avoiding double fetches causing suggestions for incorrect context.
            if (!TextUtils.equals(url, mCurrentRequestUrl)) return;

            Toast.makeText(ContextUtils.getApplicationContext(),
                         suggestions.size() + " suggestions fetched", Toast.LENGTH_SHORT)
                    .show();

            if (suggestions.size() > 0) {
                preloadSuggestions(generateClusterList(suggestions), suggestions.get(0).mTitle);
                // If the controls are already off-screen, show the suggestions immediately so they
                // are available on reverse scroll.
                if (areBrowserControlsHidden()) mCoordinator.showSuggestions();
            }
        });
    }

    @Override
    public void clearState() {
        clearSuggestions();
    }

    private void clearSuggestions() {
        // TODO(twellington): Does this signal need to go back to FetchHelper?
        mDidSuggestionsShowForTab = false;
        mModel.setClusterList(new ClusterList(Collections.emptyList()));
        mModel.setCloseButtonOnClickListener(null);
        mModel.setTitle(null);
        mCoordinator.removeSuggestions();
        mCurrentRequestUrl = "";
    }

    private void preloadSuggestions(ClusterList clusters, String title) {
        mModel.setClusterList(clusters);
        mModel.setCloseButtonOnClickListener(view -> { clearSuggestions(); });
        mModel.setTitle(mContext.getString(R.string.contextual_suggestions_toolbar_title, title));
        mCoordinator.preloadSuggestionsInSheet();
    }

    // TODO(twellington): Remove after clusters are returned from the backend.
    private ClusterList generateClusterList(List<SnippetArticle> suggestions) {
        List<ContextualSuggestionsCluster> clusters = new ArrayList<>();
        int clusterSize = suggestions.size() >= 6 ? 3 : 2;
        int numClusters = suggestions.size() < 4 ? 1 : suggestions.size() / clusterSize;
        int currentSuggestion = 0;

        // Construct a list of clusters.
        for (int i = 0; i < numClusters; i++) {
            ContextualSuggestionsCluster cluster =
                    new ContextualSuggestionsCluster(suggestions.get(currentSuggestion).mTitle);
            if (i == 0) cluster.setShouldShowTitle(false);

            for (int j = 0; j < clusterSize; j++) {
                cluster.getSuggestions().add(suggestions.get(currentSuggestion));
                currentSuggestion++;
            }

            clusters.add(cluster);
        }

        // Add the remaining suggestions to the last cluster.
        while (currentSuggestion < suggestions.size()) {
            clusters.get(clusters.size() - 1)
                    .getSuggestions()
                    .add(suggestions.get(currentSuggestion));
            currentSuggestion++;
        }

        for (ContextualSuggestionsCluster cluster : clusters) {
            cluster.buildChildren();
        }

        return new ClusterList(clusters);
    }
}
