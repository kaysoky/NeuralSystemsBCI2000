ResetProjectiles = function() {
    $('#Aeroplane').css({
        "left": "100%",
        "top": "15%"
    });
    $('#Missile').css({
        "left": "85%",
        "top": "100%",
        "transform": "rotate(0rad)"
    }).attr('radians', '0.0');
};

/**
 * Moves the airplane and missile
 * Airplane -> Moves from top right to top left
 * Missile -> Moves from bottom right to top left
 */
MoveProjectiles = function() {
    var width = $('#Gamescreen').width() * 1.0;
    var height = $('#Gamescreen').height() * 1.0;

    // Move the aeroplane left
    // It should move off the screen over the trial time
    var aeroplane = $('#Aeroplane');
    var aeroplaneWidth = aeroplane.width();
    var aeroplanePos = aeroplane.position().left / width
                      - FRAME_DELAY / TRIAL_TIME
                          * (1.0 + aeroplaneWidth / width);
    aeroplanePos *= 100; // Percent
    aeroplane.css({
        "left": aeroplanePos + "%"
    });

    // Move the missile towards the city
    var missile = $('#Missile');
    var missileRad = parseFloat(missile.attr('radians'))
                     + FRAME_DELAY / TRIAL_TIME
                         * 1.5707963267948966192313216916398;
    missile.attr('radians', '' + missileRad);
    missile.css({
        "left": (Math.cos(missileRad) * 85) + "%",
        "top": (100 - Math.sin(missileRad) * 85) + "%",
        "transform": "rotate(-" + missileRad + "rad)"
    });
}
