#ifndef MONGOOSETASK_H
#define MONGOOSETASK_H

#include <queue>
#include <vector>

#include "FeedbackTask.h"
#include "OSMutex.h"
#include "mongoose.h"

class MongooseFeedbackTask : public FeedbackTask {
public:
    MongooseFeedbackTask();
    virtual ~MongooseFeedbackTask();

    /*
     * Loops infinitely and polls for incoming connections
     * Note: The underlying server can be reinitialized and swapped out at any time
     */
    friend void *MongooseServerThread(void *arg);

    /*
     * Handles the pre-defined set of REST methods
     * Other methods are passed along to Mongoose for default handling
     */
    friend int MongooseServerHandler(struct mg_connection *conn);
    
    /*
     * Derived classes should extend this function to process specific requests
     */
    virtual int HandleMongooseRequest(struct mg_connection *conn) {return MG_REQUEST_NOT_PROCESSED;}

protected:
    /*
     * Checks MongooseFeedbackTask specific parameter settings
     * This should be called within Preflight
     */
    void CheckServerParameters(const SignalProperties& Input) const;

    /*
     * Creates the Mongoose server
     * This should be called within Initialization
     */
    void InitializeServer(const SignalProperties& Input);

    /*
     * Derived classes should set this value to True
     * IFF the class is ready to process requests
     */
    bool isRunning;

private:
    /*
     * Provides synchronization for the Mongoose server
     * This allows a new server instance to be swapped in at any time
     */
    OSMutex *server_lock;

    /*
     * Mongoose server
     * Serves up the files within the directory it is given
     *   and handles the various communication REST functions
     * A new server instance is created every time the configuration is set
     */
	struct mg_server *server;
};

#endif // MONGOOSETASK_H

