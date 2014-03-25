/**
 * Asynchronously POST's some text for the server to log
 */
LogInfo = function(text) {
    $.ajax({
        type: 'POST',
        url: '/log',
        data: text,
        error: function(jqXHR) {
            alert("POST /log -> " + jqXHR.status + " " + jqXHR.statusText);
        }
    });
}
