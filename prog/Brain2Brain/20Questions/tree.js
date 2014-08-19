var Questions = [];
var Answers = {};

/**
 * Fetches the file from the server and parses it into a question/answer tree
 */
InitializeQuestionMatrix = function(filename) {
    // Fetch the data
    $.get(filename)
    .done(function(data) {
        var lines = data.split('\n');

        // Build the tree
        QuestionTree = BuildQuestionMatrix(lines);

        // Hide the scope of the next operation
        // Since the client determines the answer to the game that is being played,
        //   we don't want to accidentally save this information on the client
        (function() {
            // Update the UI
            var answer = UpdateSelectors();

            // Tell the BCI-side what the client is playing for
            LogInfo('Answer is: ' + answer);
            $.ajax({
                type: 'PUT',
                url: '/text/answer',
                async: true,
                data: answer,
                error: function(jqXHR) {
                    if (jqXHR.status == 418) {
                        alert('Application not ready to begin');
                        location.reload();
                    } else {
                        alert("POST /text/answer -> " + JSON.stringify(jqXHR));
                    }
                }
            });
        })();

    }).fail(function() {
        alert("Could not fetch " + filename);
    });
};

AnswerQuestion = function(questionAsked, isYes) {
    // Gray out answers that have been ruled out and move them to the bottom
    var ruledOut = isYes ? 'no' : 'yes';
    for (key in Answers) {
        if (Answers[key][questionAsked] === ruledOut) {
            $('#' + key.replace(' ', '-')).addClass('ruled-out');
        }
    }
}

/******************************************************************************
 *                          Private helper functions                          *
 ******************************************************************************/
// Note: there is a way to make these functions actually private (anonymous functions),
//       but the added complexity would be unnecessary

/**
 * Fills in the Questions/Answers selectables with the current tree
 * Returns a random element from the array of possible answers
 */
UpdateSelectors = function() {
    // Clear the existing items
    $('#QuestionsList, #AnswersList').empty();

    // Fill in the questions
    var qList = $('#QuestionsList');
    for (var i in Questions) {
        if (i >= MAX_SELECTABLES) {
            break;
        }
        qList.append('<li class="ui-widget-content">' + Questions[i] + '</li>');
    }

    // Fill in the answers
    var aList = $('#AnswersList');
    var counter = 0;
    for (var key in Answers) {
        if (counter >= MAX_SELECTABLES) {
            break;
        }
        aList.append('<li id="' + key.replace(' ', '-') + '" class="ui-widget-content">' + key + '</li>');
        counter++;
    }

    // Make the lists pretty and interactive
    $('#QuestionsList, #AnswersList').selectable({
        stop: function(event, ui) {
            // Prevent multi-selection within a single selectable
            $(event.target).children('.ui-selected').not(':first').removeClass('ui-selected');

            // Prevent multi-selection between the two selectables
            $('#QuestionsList, #AnswersList').not(event.target).children('.ui-selected').removeClass('ui-selected');
        }
    });

    return Object.keys(Answers)[Math.floor(Math.random() * answers.length)];
};

/**
 * Builds up the question matrix by reading a CSV
 * The header row contains the type of matrix (irrelevant info) followed by the questions
 * Each row contains an answer with a Yes/No for each question
 */
BuildQuestionMatrix = function(elements) {
    // Parse the header (questions)
    var header = elements[0];
    header = header.split(',');
    Questions = header.slice(1).map(function (value) { return value.trim(); });
    
    // Parse the rows (answers)
    var rows = elements.slice(1);
    for (var idx in rows) {
        var row = {};
        
        // Map each question to a Yes/No answer
        var info = rows[idx].split(',');
        for (var i = 1; i < info.length; i++) {
            row[Questions[i - 1]] = info[i].toLowerCase().trim();
        }
        
        // Save the row
        Answers[info[0].trim()] = row;
    };
};
