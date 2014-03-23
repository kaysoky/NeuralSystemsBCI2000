#include "PCHIncludes.h"
#pragma hdrstop

#include "MongooseFeedbackTask.h"

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

/*
 * The currently running task
 */
static MongooseFeedbackTask *currentTask;

MongooseFeedbackTask::MongooseFeedbackTask()
    : isRunning(false) {
    // Note: See FeedbackTask.cpp for more parameters and states
    BEGIN_PARAMETER_DEFINITIONS
    "Application:Mongoose string FileDirectory= % "
        "/CountdownGame/ % % // Directory where the game files are located", 
    "Application:Mongoose string ListeningPort= % "
        "20320 % % // Port for the server to listen on",
    END_PARAMETER_DEFINITIONS

    server_lock = new OSMutex();
    server = NULL;

    // Give the C-functions a reference to this class
    currentTask = this;

    // Start the server on a separate thread
    mg_start_thread(MongooseServerThread, NULL);
}

MongooseFeedbackTask::~MongooseFeedbackTask() {
	// TODO: Cleanup is not thread-safe yet
    // delete server_lock;
    // mg_destroy_server(&server);
}

void MongooseFeedbackTask::CheckServerParameters(const SignalProperties& Input) const {
    if (std::string(Parameter("FileDirectory")).empty()) {
        bcierr << "Mongoose FileDirectory must be specified" << std::endl;
    }
    if (std::string(Parameter("ListeningPort")).empty()) {
        bcierr << "Mongoose server ListeningPort must be specified" << std::endl;
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
    int retVal = GetFullPathName(std::string(Parameter("FileDirectory")).c_str(), MAX_PATH, fileDirectory, NULL);
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
	std::string uri(conn->uri);
    bciout << method << " " << conn->uri << std::endl;
    
    if (method.compare("GET") == 0) {
        // For generic requests, 
        //   don't route anything to the child handler unless it's ready
        if (!currentTask->isRunning) {
			// Catch this polling command (the only non-static GET request)
			if (uri.compare("/trial/status") == 0) {
				mg_send_status(conn, 204);
				mg_send_data(conn, "", 0);
				return MG_REQUEST_PROCESSED;
			}

            return MG_REQUEST_NOT_PROCESSED;
        }
    } else {
        // Allow the client to log arbitrary info
        if (method.compare("POST") == 0 && uri.compare("/log") == 0) {
            currentTask->AppLog << conn->content << std::endl;
            mg_send_status(conn, 204);
            mg_send_data(conn, "", 0);
            return MG_REQUEST_PROCESSED;
        }
        
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
