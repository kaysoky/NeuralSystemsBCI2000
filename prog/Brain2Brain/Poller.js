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
                    LogInfo("Hit detected, triggering TMS");
                    $.ajax({
                            type: 'POST',
                            url: 'http://localhost:' + TMS_PORT + '/TMS/fire',
                            async: false, 
                            success: function() {}, 
                            error: function(jqXHR) {
                                alert("POST /TMS/fire -> " + JSON.stringify(jqXHR));
                            }
                        });
                }

                if (data.search("REFRESH") >= 0) {
                    location.reload();
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
