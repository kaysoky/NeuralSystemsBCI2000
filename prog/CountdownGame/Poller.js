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
                if (data.search("HIT") >= 0) {
                    alert("Hit");
                    $.ajax({
                            type: 'POST',
                            url: 'http://localhost:' + TMS_PORT + '/TMS/fire',
                            async: false
                        });
                }

                if (data.search("REFRESH") >= 0) {
                    location.reload();
                }
            }
            
            // Continue polling for hits
            setTimeout(PollServer, POLL_INTERVAL);
        }
    });
}
