var Questions = [];
var Answers = {};

// This is the "answer" the client is trying to guess
// Should not be exposed to the UI
var TheAnswer = undefined;

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
            TheAnswer = UpdateSelectors();

            // Tell the BCI-side what the client is playing for
            LogInfo('The file being used is: ' + filename);
            LogInfo('Answer is ' + TheAnswer);
            PUT_TextAnswer(TheAnswer, function () {
                alert('Application not ready to begin');
                location.reload();
            });
        })();

    }).fail(function() {
        alert("Could not fetch " + filename);
    });
};

AnswerQuestion = function(questionAsked, isYes) {
    // Gray out the question that was asked
    $('#' + GetSanitizedText(questionAsked)).addClass('ruled-out');

    // Gray out answers that have been ruled out and move them to the bottom
    var ruledOut = isYes ? 'no' : 'yes';
    for (key in Answers) {
        if (Answers[key][questionAsked] === ruledOut) {
            $('#' + GetSanitizedText(key)).addClass('ruled-out');
        }
    }

    // Check to see if the game is over (only one answer left)
    var remainingAnswers = $('#AnswersList > li').not('.ruled-out');
    if (remainingAnswers.length === 1) {
        var lastAnswer = remainingAnswers.text();
        var alertText = 'guessed "' + lastAnswer + '" ' 
            + (lastAnswer === TheAnswer ? '' : 'in')
            + 'correctly!';
            
        // Show the EEG-side the result text
        PUT_TextAnswer('Your partner ' + alertText, function () {
            alert('Application not ready to begin');
            location.reload();
        });
        location.reload();
    } else if (remainingAnswers.length <= 0) {
        alert('No possibilities remain.  Game over.\nClose this alert to refresh the page');
        location.reload();
    }
}

/******************************************************************************
 *                          Private helper functions                          *
 ******************************************************************************/
// Note: there is a way to make these functions actually private (anonymous functions),
//       but the added complexity would be unnecessary

/**
 * Returns a string that can be safely used as part of a jQuery ID selector
 */
GetSanitizedText = function(text) {
    return text.replace(/\W/g, '-');
};

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
        qList.append('<li id="' + GetSanitizedText(Questions[i]) + '" class="ui-widget-content">' 
            + Questions[i] + '</li>');
    }

    // Fill in the answers
    var aList = $('#AnswersList');
    for (var key in Answers) {
        aList.append('<li id="' + GetSanitizedText(key) + '" class="ui-widget-content">' 
            + key + '</li>');
    }

    // Make the lists pretty and interactive
    $('#QuestionsList, #AnswersList').selectable({
        stop: function(event, ui) {
            // Prevent multi-selection within a single selectable
            $(event.target).children('.ui-selected').not(':first').removeClass('ui-selected');

            // Prevent multi-selection between the two selectables
            $('#QuestionsList, #AnswersList').not(event.target).children('.ui-selected').removeClass('ui-selected');
            
            $('#SubmitButton > .ui-button-text')
                .html($('#QuestionsList').children('.ui-selected').length > 0 ? 
                    'Submit' : 
                    '&uarr; Select a question &uarr;');
        }
    });

    var possibleAnswers = Object.keys(Answers);
    return possibleAnswers[Math.floor(Math.random() * possibleAnswers.length)];
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
