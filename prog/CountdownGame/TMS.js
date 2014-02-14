/**
 * Polls the B2B server for a hit
 * When found, triggers a TMS pulse
 */
CheckServerForHit = function() {
    $.ajax(
        type: 'GET', 
        url: '/trial/hit', 
        async: false
    ).done(
        function(data) {
            // If there is a hit, activate the TMS
            var temp = (data).match(/HIT/);
            if (temp && temp.length > 1) {
                $.ajax({
                        type: 'POST', 
                        url: 'http://localhost:' + TMS_PORT + '/TMS/fire', 
                        async: false
                    });
            }
        }
    ).always(
        // Continue polling for hits
        function() {
            setTimeout(CheckServerForHit, FRAME_DELAY);
        }
    );
}
