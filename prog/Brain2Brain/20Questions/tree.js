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
        
        // Update the UI
        UpdateSelectors();
        
    }).fail(function() {
        alert("Could not fetch " + filename);
    });
};

/**
 * Fills in the Questions/Answers selectables with the current tree
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
        stop: function(event, ui){
            $(event.target).children('.ui-selected').not(':first').removeClass('ui-selected');
        }
    });
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
