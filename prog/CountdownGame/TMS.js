/**
 * Polls the B2B server for a hit
 * When found, triggers a TMS pulse
 */
CheckServerForHit = function() {
    $.get('/trial/hit')
    .done(
        function(data) {
            // If there is a hit, activate the TMS
            var temp = (data).match(/HIT/);
            if (temp && temp.length > 1) {
                $.post('http://localhost:25000/TMS/fire');
            }
        }
    ).always(
        // Continue polling for hits
        function() {
            setTimeout(CheckServerForHit, FRAME_DELAY);
        }
    );
}
