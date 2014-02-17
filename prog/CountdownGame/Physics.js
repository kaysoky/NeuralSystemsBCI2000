var missileAnimation = null;

ResetProjectiles = function() {
    // Stop any animations
    $('#Aeroplane, #Missile, #CannonOrigin').stop();
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
    });
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
    
    // Determine which projectile the cannon should follow
    var projectile = null;
    if ($('#Aeroplane').css('display') != 'none') {
        projectile = $('#Aeroplane');
    } else if ($('#Missile').css('display') != 'none') {
        projectile = $('#Missile');
    } else {
        alert("No projectile visible");
    }
    
    // Have the cannon follow the projectile
    var cannon = $('#CannonOrigin');
    var cPos = cannon.position();
    cannon.animate({'rad': '0'}, {
        duration: TRIAL_TIME,
        step: function() {
            var pPos = projectile.position();
            cannon.css({
                'transform': 'rotate(' + (3.14 - Math.atan2(pPos.left - cPos.left, pPos.top - cPos.top)) + 'rad)'
            })
        }
    });
}
