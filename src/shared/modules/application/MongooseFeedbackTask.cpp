#include "PCHIncludes.h"
#pragma hdrstop

#include "MongooseFeedbackTask.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <algorithm>
#include <cstdlib>

/*
 * The currently running task
 */
static MongooseFeedbackTask *currentTask;

MongooseFeedbackTask::MongooseFeedbackTask() : nextTrialType(NULL) {
    // Note: See FeedbackTask.cpp for more parameters and states
    BEGIN_PARAMETER_DEFINITIONS
    "Application:Mongoose string FileDirectory= % "
        "/CountdownGame/ % % //"
    "Application:Mongoose string ListeningPort= % "
        "20320 % % // Port for the server to listen on",
    END_PARAMETER_DEFINITIONS

    server_lock = new OSMutex();
    server = NULL;

    // Give the static C-functions a reference to this program
    currentTask = this;

    // Start the server on a separate thread
    mg_start_thread(MongooseServerThread, NULL);
}

void MongooseFeedbackTask::CheckServerParameters(const SignalProperties& Input) const {
    if (string(Parameter("FileDirectory")).empty()) {
        bcierr << "Mongoose FileDirectory must be specified" << endl;
    }
    if (string(Parameter("ListeningPort")).empty()) {
        bcierr << "Mongoose server ListeningPort must be specified" << endl;
    }
}

void MongooseFeedbackTask::InitializeServer(const SignalProperties& Input) {
    // Initialize the server
    server_lock->Acquire();
    if (server != NULL) {
        mg_destroy_server(&server);
    }
    server = mg_create_server(NULL);

    // Determine and set where the files to be served are
    char fileDirectory[MAX_PATH];
    fileDirectory = realpath(std::string(Parameter("FileDirectory")).c_str(), fileDirectory);
    mg_set_option(server, "document_root", fileDirectory);

    // Set the request handler
    mg_set_request_handler(server, MongooseServerHandler);

    // Determine and set the listening port
    std::string listeningPort = std::string(Parameter("ListeningPort"));
    mg_set_option(server, "listening_port", listeningPort.c_str());
    bciout << "Server listening on port " << mg_get_option(server, "listening_port");

    // Done with server initialization
    server_lock->Release();
}

void *MongooseServerThread(void *arg) {
    while (true) {
        if (currentTask->server == NULL) {
            continue;
        }

        currentTask->server_lock->Acquire();
        mg_poll_server(currentTask->server, 10);
        currentTask->server_lock->Release();
    }

    return NULL;
}

int MongooseServerHandler(struct mg_connection *conn) {
    std::string method(conn->request_method);
    bciout << method << " " << conn->uri << endl;
    
    if (method.compare("GET") == 0) {
        // For generic requests, 
        //   don't route anything to the child handler unless it's ready
        if (!currentTask->isRunning) {
            return MG_REQUEST_NOT_PROCESSED;
        }
    } else {
        // Fail all non-generic requests with the special error code
        if (!currentTask->isRunning) {
            mg_send_status(conn, 418);
            mg_send_data(conn, "", 0);
            return MG_REQUEST_PROCESSED;
        }
    }
    
    // Route all other requests to the child
    return currentTask->HandleMongooseRequest(conn);
}
