/**
 * Tells the BCI to start a trial
 * If the application is not ready, onFailure is called
 */
POST_TrialStart = function(text, callback, onFailure) {
    $.ajax({
        type: 'POST',
        url: '/trial/start',
        async: false,
        data: text, 
        success: callback, 
        error: function(jqXHR) {
            // Application not ready
            if (jqXHR.status == 418) {
                onFailure();
            } else {
                alert("POST /trial/start -> " + JSON.stringify(jqXHR));
            }
        }
    });
}

/**
 * Tells the BCI to stop a trial
 * If the application is not ready, onFailure is called
 */
POST_TrialStop = function(callback, onFailure) {
    $.ajax({
        type: 'POST',
        url: '/trial/stop',
        async: false,
        success: callback, 
        error: function(jqXHR) {
            // Did the BCI-side stop first?
            if (jqXHR.status == 418) {
                onFailure();
            } else {
                alert("POST /trial/stop -> " + JSON.stringify(jqXHR));
            }
        }
    });
}
