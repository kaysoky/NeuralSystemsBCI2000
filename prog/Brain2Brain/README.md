The Brain2Brain application starts a server that handles the following REST methods:
* POST /trial/start
    The client should call this to indicate that the countdown has ended
        This allows the application to transition into the appropriate "doFeedback" loop
    Returns:
        The trial type that should be displayed
            0 = airplane
            1 = missile
    Note: A 418 error will be thrown if the B2B application is not ready for a trial
        In this case, the client should restart the countdown
    
* POST /trial/stop
    The client should call this to indicate that the trial has ended
    Querystring:
        spacebar = boolean, indicates that the spacebar was pressed during the trial
        missile = integer, number of times a missile has been shot down during the run
        airplane = integer, number of times a plane has been shot down during the run
    
* GET /trial/status
    Returns:
        "HIT" = a target has been hit in the B2B application
            The client should then trigger a TMS pulse
        "REFRESH" = the run has ended
            The client should refresh the page