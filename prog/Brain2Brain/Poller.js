/**
 * Polls the B2B server for a hit
 * When found, triggers a TMS pulse
 */
PollServer = function() {
    $.ajax({
        type: 'GET',
        url: '/trial/status',
        async: true,
        success: function(data) {
            // If there is a hit, activate the TMS
            if (data && data.search) {
                if (data.search("YES") >= 0) {
                    LogInfo("Positive signal detected, triggering TMS (high)");
                    $.ajax({
                            type: 'POST',
                            url: 'http://localhost:' + TMS_PORT + '/TMS/fire/high',
                            async: false,
                            success: function(data) {
                                LogInfo("TMS intensity = " + data);
                            },
                            error: function(jqXHR) {
                                alert("POST /TMS/fire/high -> " + JSON.stringify(jqXHR));
                            }
                        });

                } else if (data.search('NO') >= 0) {
                    LogInfo("Negative signal detected, triggering TMS (low)");
                    $.ajax({
                            type: 'POST',
                            url: 'http://localhost:' + TMS_PORT + '/TMS/fire/low',
                            async: false,
                            success: function(data) {
                                LogInfo("TMS intensity = " + data);
                            },
                            error: function(jqXHR) {
                                alert("POST /TMS/fire/low -> " + JSON.stringify(jqXHR));
                            }
                        });
                
                } else if (data.search("REFRESH") >= 0) {
                    setTimeout(function() {
                        alert("Run ended, close this alert to refresh the page");
                        location.reload();
                    }, REFRESH_WAIT);
                }
            }

            // Continue polling for hits
            setTimeout(PollServer, POLL_INTERVAL);
        },
        error: function(jqXHR) {
            alert("GET /trial/status -> " + JSON.stringify(jqXHR));
        }
    });
}
