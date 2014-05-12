/**
 * Asynchronously POST's some text for the server to log
 */
LogInfo = function(text) {
    // Indent multi-line logs
    text = text.split('\n').join('\n-> ');

    // Append a client-side timestamp
    var timestamp = new Date().toISOString() + " [client] ";
    text = timestamp + text;

    $.ajax({
        type: 'POST',
        url: '/log',
        data: text,
        error: function(jqXHR) {
            alert("POST /log -> " + jqXHR.status + " " + jqXHR.statusText);
        }
    });
}
