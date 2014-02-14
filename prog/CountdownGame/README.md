The Brain2Brain application starts a server that handles the following REST methods:
* POST /trial/start
    - Indicates that the countdown has ended
    - This makes the application start reading brain data
    
* POST /trial/stop
    - Indicates that the trial has ended
    
* GET /trial/hit
    - Reports if a target has been hit during a trial