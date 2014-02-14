/**
 * Determines the frame rate
 */
var FRAME_DELAY = 20.0; // Milliseconds

/**
 * How long each trial should take
 */
var TRIAL_TIME = 6000.0; // Milliseconds

/**
 * How long the countdown lasts
 */
var COUNTDOWN_TIME = 20000; // Milliseconds

/**
 * How long the pause is between a trial ending and the countdown beginning
 */
var PAUSE_BETWEEN_TRIALS = 2000; // Milliseconds

/**
 * Trial types
 */
var AIRPLANE = 'AIRPLANE';
var MISSILE = 'MISSILE';
var TRIAL_TYPE_ENUM = [AIRPLANE, MISSILE];

/**
 * TMS port
 */
var TMS_PORT = 25000;
