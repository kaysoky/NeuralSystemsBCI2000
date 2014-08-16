var QuestionTree = {};
var LastSelectedNode = null;

var NodeWidth = 150;
var NodeHeight = 75;

/**
 * Fetches the file from the server and parses it into a question/answer tree
 */
InitializeQuestionTree = function(filename) {
    // Fetch the data
    $.get(filename)
    .done(function(data) {
        var lines = data.split('\n');

        // Build the tree in memory
        QuestionTree = BuildQuestionTree(lines);
        
        // Prepare the SVG for tree-ification
        var TreeWidth = GetAnswers(QuestionTree).length * NodeWidth;
        var TreeHeight = GetDepth(QuestionTree) * NodeHeight;
        var vis = d3.select('#QuestionTree').append('svg')
            .attr('width', TreeWidth + NodeWidth).attr('height', TreeHeight + NodeHeight)
            .attr('transform', 'translate(' + (NodeWidth / 2) + ',' + (NodeHeight / 2) + ')');

        // Initialize the D3 tree layout
        var tree = d3.layout.tree()
            .size([TreeWidth, TreeHeight])
            .children(function (d) {
                return d.question ? [d.left, d.right] : null;
            });

        // Convert the data into nodes and links
        var nodes = tree.nodes(QuestionTree);
        var links = tree.links(nodes);
   
        // Initialize the links
        var diagonal = d3.svg.diagonal();
        var link = vis.selectAll('path')
            .data(links)
            .enter().append('path')
            .attr('class', 'link')
            .attr('d', diagonal)
            .attr('id', function (d) {
                return 'Link-' + d.source.id + '-' + d.target.id;
            });

        // Initialize the nodes
        var node = vis.selectAll('g.node')
            .data(nodes)
            .enter().append('g')
            .attr('id', function (d) { return 'Node-' + d.id; })
            .attr('transform', function(d) { return 'translate(' + d.x + ',' + d.y + ')'; })
            .on('mouseover', function () {
                d3.select(this).select('foreignObject > div').classed({ 'highlighted': true });
            })
            .on('mouseout', function () {
                d3.select(this).select('foreignObject > div').classed({ 'highlighted': false });
            })
            .on('click', function (d) {
                var element = d3.select(this);
                
                // Submit the question when clicking the node for the first time
                if (!element.classed('explored')) {
                    LastSelectedNode = d;
                    SubmitQuestion(d.question || d.answer, d.question);
                    element.classed({ 'explored': true });
                    d3.select('#Link-' + d.parent.id + '-' + d.id).classed({ 'explored': true });
                }
            });

        // Add the dot at every node
        node.append('circle').attr('r', NodeWidth / 10);

        // place the name atribute left or right depending if children
        node.append('foreignObject')
            .attr('transform', 'translate(-' + (NodeWidth / 2) + ', -' + (NodeWidth / 15) + ')')
            .attr('width', NodeWidth)
            .attr('height', NodeHeight)
            .append("xhtml:div")
            .style({ 'width': NodeWidth + 'px',
                'height': NodeHeight + 'px' })
            .text(function(d) { return d.question || d.answer; });

        $('svg').draggable().css({
            'left': ($('#QuestionTree').width() / 2 - (TreeWidth + NodeWidth) / 2) + 'px'
        });

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

AnswerQuestion = function(isYes) {
    if (LastSelectedNode && LastSelectedNode.children && LastSelectedNode.children.length === 2) {
        var LinkPrefix = '#Link-' + LastSelectedNode.id + '-';
        d3.select(LinkPrefix + LastSelectedNode.children[0].id).classed({ 'selected': isYes });
        d3.select(LinkPrefix + LastSelectedNode.children[1].id).classed({ 'selected': !isYes });
    }
}

/******************************************************************************
 *                          Private helper functions                          *
 ******************************************************************************/
// Note: there is a way to make these functions actually private (anonymous functions),
//       but the added complexity would be unnecessary

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
var NodeIDCounter = 0;
BuildQuestionTree = function(elements) {
    // Finished parsing
    if (elements.length <= 0) {
        return "";
    }

    // Get the line to parse
    var line = elements.shift();
    var parts = line.split(':');

    var node = {};
    node.id = NodeIDCounter++;
    if (parts[0] == 'A') {
        node.answer = parts[1].trim();
        return node;
    } else if (parts[0] == 'Q') {
        node.question = parts[1].trim();
        node.left = BuildQuestionTree(elements);
        node.right = BuildQuestionTree(elements);
        return node;
    } else {
        LogInfo('Could not parse line ' + index + ': ' + line);
    }
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

GetDepth = function (tree) {
    if (tree.question) {
        return Math.max(GetDepth(tree.left), GetDepth(tree.right)) + 1;
    }

    return 1;
};
