// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(async function() {
  TestRunner.addResult(
      `Tests that Tracing agent returns a session id upon a start that is matching one issued in trace events.\n`);
  await TestRunner.loadModule('performance_test_runner');
  await TestRunner.showPanel('timeline');
  await TestRunner.loadHTML(`
      <p style="transform: translateZ(10px)"> <!-- Force compositing so we have SetLayerTreeHostId event as well -->
      </p>
      <script>
        function waitForRaf() {
          return new Promise(f => requestAnimationFrame(f));
        }
      </script>
    `);

  await PerformanceTestRunner.invokeAsyncWithTimeline('waitForRaf');

  PerformanceTestRunner.tracingModel().sortedProcesses().forEach(function(process) {
    process.sortedThreads().forEach(function(thread) {
      thread.events().forEach(processEvent);
    });
  });
  TestRunner.completeTest();

  var frameId = '';

  function processEvent(event) {
    if (!event.hasCategory(SDK.TracingModel.DevToolsMetadataEventCategory))
      return;

    if (event.name === TimelineModel.TimelineModel.RecordType.TracingStartedInPage) {
      TestRunner.assertEquals(PerformanceTestRunner.timelineModel()._sessionId, event.args['sessionId'] || event.args['data']['sessionId']);
      TestRunner.addResult('Got DevTools metadata event: ' + event.name);
      frameId = event.args['data']['frames'][0]['frame'];
    } else if (event.name === TimelineModel.TimelineModel.RecordType.SetLayerTreeId) {
      if (frameId === event.args['data']['frame'])
        TestRunner.addResult('Got DevTools metadata event: ' + event.name);
    }
  }
})();
