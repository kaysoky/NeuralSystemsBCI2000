var QuestionTree = {};

/**
 * Fetches the file from the server and parses it into a question/answer tree
 */
InitializeQuestionTree = function(filename) {
    // Fetch the data
    $.get(filename)
    .done(function(data) {
        var lines = data.split('\n');
        
        // Build the tree
        QuestionTree = BuildQuestionTree(lines);
        
        // Hide the scope of the next operation
        // Since the client determines the answer to the game that is being played,
        //   we don't want to accidentally save this information on the client
        (function() {
            // Update the UI
            var answer = UpdateSelectors();
            
            // Tell the BCI-side what the client is playing for
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

/**
 * Fills in the Questions/Answers selectables with the current tree
 * Returns a random element from the array of possible answers
 */
UpdateSelectors = function() {
    // Clear the existing items
    $('#QuestionsList, #AnswersList').empty();
    
    // Fill in the questions
    var questions = GetQuestions(QuestionTree);
    var qList = $('#QuestionsList');
    for (var i in questions) {
        qList.append('<li class="ui-widget-content">' + questions[i] + '</li>')
    }

    // Fill in the answers
    var answers = GetAnswers(QuestionTree);
    var aList = $('#AnswersList');
    for (var i in answers) {
        aList.append('<li class="ui-widget-content">' + answers[i] + '</li>')
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
    
    return answers[Math.floor(Math.random() * answers.length)];
};

/**
 * Builds up the question tree
 *   A YES traverses left
 *   A NO traverses right
 *
 * Example file:
 *     Q: Is it an animal
 *     Q: Can it fly?
 *     A: Bird
 *     A: Cat
 *     A: Laptop
 *
 *              Animal?
 *            /         \
 *       Can it fly?     Laptop
 *      /           \
 *  Bird            Cat
 */
BuildQuestionTree = function(elements) {
    // Finished parsing
    if (elements.length <= 0) {
        return "";
    }

    // Get the line to parse
    var line = elements.shift();
    var parts = line.split(':');

    if (parts[0] == 'A') {
        return parts[1];
    } else if (parts[0] == 'Q') {
        var node = {};
        node.question = parts[1];
        node.left = BuildQuestionTree(elements);
        node.right = BuildQuestionTree(elements);
        return node;
    } else {
        LogInfo('Could not parse line ' + index + ': ' + line);
    }
};

/**
 * Returns a list of all the questions contained in the tree
 * Traversal order is: middle -> left -> right
 */
GetQuestions = function(tree) {
    var questions = [];

    // Is this a question node?
    if (tree.question) {
        return [tree.question]
            .concat(GetQuestions(tree.left))
            .concat(GetQuestions(tree.right));
    }

    return [];
};

/**
 * Returns a list of all the answers contained in the tree
 */
GetAnswers = function(tree) {
    var answers = [];

    // Is this a question node?
    if (tree.question) {
        return GetAnswers(tree.left)
            .concat(GetAnswers(tree.right));
    }

    return [tree];
};
