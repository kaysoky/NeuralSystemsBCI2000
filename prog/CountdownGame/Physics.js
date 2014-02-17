var missileAnimation = null;

ResetProjectiles = function() {
    // Stop any animations
    $('#Aeroplane, #Missile').stop();
    $(missileAnimation).stop();
    
    // Display the correct picture
    if ($('#Aeroplane > .projectile').hasClass('hidden')) {
        $('#Aeroplane > img').toggleClass('hidden');
    }
    if ($('#Missile > .projectile').hasClass('hidden')) {
        $('#Missile > img').toggleClass('hidden');
    }
    
    // Reset positions
    $('#Aeroplane').css({
        'left': '100%',
        'top': '15%'
    });
    $('#Missile').css({
        'left': '85%',
        'top': '100%',
        'transform': 'rotate(0rad)'
    }).attr('radians', '0.0');
};

/**
 * Moves the airplane and missile
 * Airplane -> Moves from top right to top left
 * Missile -> Moves from bottom right to top left
 */
MoveProjectiles = function() {
    $('#Aeroplane').animate({
        'left': '-10%'
    }, TRIAL_TIME);
    
    var missile = $('#Missile');
    missileAnimation = $({rad: 0}).animate({rad: 1.72}, {
        duration: TRIAL_TIME,
        step: function(now) {
            missile.css({
                'left': (Math.cos(now) * 85) + "%",
                'top': (100 - Math.sin(now) * 85) + "%",
                'transform': 'rotate(-' + now + 'rad)'
            });
        }
    });
}
