/**
 * How long each trial should take
 * This value is overwritten each time a trial is started
 */
var TRIAL_TIME = 0; // Milliseconds

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
        success: function (data) {
            // The data is 2 numbers:
            //   <a trial number>
            //   <a trial time> // <- This one is removed and put into TRIAL_TIME
            //   <anything else>
            data = data.split('\n');
            TRIAL_TIME = parseInt(data.splice(1, 1)[0]);
            data = data.join('\n');
            
            callback(data);
        }, 
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

/**
 * Tells the BCI to display some text in the "answer" box
 * If the application is not ready, onFailure is called
 */
PUT_TextAnswer = function(text, onFailure) {
    $.ajax({
        type: 'PUT',
        url: '/text/answer',
        async: true,
        data: text,
        error: function(jqXHR) {
            if (jqXHR.status == 418) {
                onFailure();
            } else {
                alert("PUT /text/answer -> " + JSON.stringify(jqXHR));
            }
        }
    });
}
