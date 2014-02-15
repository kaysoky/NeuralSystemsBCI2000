/**
 * Polls the B2B server for a hit
 * When found, triggers a TMS pulse
 */
CheckServerForHit = function() {
    $.ajax({
        type: 'GET', 
        url: '/trial/hit', 
        async: false, 
        success: function(data) {
            // If there is a hit, activate the TMS
            if (data && data.match) {
                var temp = data.match(/HIT/);
                if (temp && temp.length > 1) {
                    alert("Hit");
                    $.ajax({
                            type: 'POST', 
                            url: 'http://localhost:' + TMS_PORT + '/TMS/fire', 
                            async: false
                        });
                }
            }
            // Continue polling for hits
            setTimeout(CheckServerForHit, HIT_POLL_INTERVAL);
        }
    });
}
