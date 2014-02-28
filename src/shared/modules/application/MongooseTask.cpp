#include "PCHIncludes.h"
#pragma hdrstop

#include "MongooseTask.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sstream>
#include <algorithm>
#include <cstdlib>

/*
 * The currently running task
 */
static MongooseTask *currentTask;

MongooseTask::MongooseTask() : nextTrialType(NULL) {

    // Note: See FeedbackTask.cpp for more parameters and states
    
    BEGIN_PARAMETER_DEFINITIONS
    "Application:Mongoose string FileDirectory= % "
        "/CountdownGame/ % % //"
    "Application:Mongoose string ListeningPort= % "
        "20320 % % // Port for the server to listen on",
    END_PARAMETER_DEFINITIONS

    BEGIN_STATE_DEFINITIONS
        "TrialType              2 0 0 0", // 0 for Airplane, 1 for Missile
        "CountdownHitReported   1 0 0 0", // Spacebar was pressed in the Countdown game
        "CountdownMissileScore  8 0 0 0", // Number of missiles shot
        "CountdownAirplaneScore 8 0 0 0", // Number of airplanes shot
    END_STATE_DEFINITIONS

    server_lock = new OSMutex();
    server = NULL;
    
    // Give the static C-functions a reference to this program
    currentTask = this;
    
    // Start the server on a separate thread
    mg_start_thread(MongooseServerThread, NULL);
}

void MongooseTask::CheckServerParameters(const SignalProperties& Input) const {
    if (string(Parameter("FileDirectory")).empty()) {
        bcierr << "Mongoose FileDirectory must be specified" << endl;
    }
    if (string(Parameter("ListeningPort")).empty()) {
        bcierr << "Mongoose server ListeningPort must be specified" << endl;
    }
}

void MongooseTask::InitializeServer(const SignalProperties& Input) {
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

void MongooseTask::PrepareForRun() {
    // Reset state of the game
    server_lock->Acquire();
    countdownMissileScore = 0;
    countdownAirplaneScore = 0;
    isRunning = true;
    lastClientPost = CONTINUE;
    targetHit = false;
    runEnded = false;

    // Determine the trial types
    if (nextTrialType != NULL) {
        delete nextTrialType;
    }
    nextTrialType = new std::queue<TrialType>();
    // Note: This parameter is defined in FeedbackTask.cpp
    if (!std::string(Parameter("NumberOfTrials")).empty()) {
        // We know the number of trials,
        // so we can enforce an equal number of each trial type
        int numTrials = Parameter("NumberOfTrials");
        int typesPerTrial = numTrials / LAST;
        vector<TrialType> trialVector(numTrials, AIRPLANE);
        for (int i = AIRPLANE + 1; i < LAST; i++) {
            for (int j = 0; j < typesPerTrial; j++) {
                trialVector[i * typesPerTrial + j] = static_cast<TrialType>(i);
            }
        }

        // Shuffle and store the ordering
        std::random_shuffle(trialVector.begin(), trialVector.end());
        for (unsigned int i = 0; i < trialVector.size(); i++) {
            nextTrialType->push(trialVector[i]);
        }
    }
    server_lock->Release();
}

bool MongooseTask::IsClientReady() {
    return lastClientPost == START_TRIAL;
}

void MongooseTask::PrepareForTrial() {
    server_lock->Acquire();
    targetHit = false;
    countdownSpacebarPressed = false;
    server_lock->Release();
}

void IndicateTargetHit() {
    server_lock->Acquire();
    targetHit = true;
    server_lock->Release();
}

bool MongooseTask::IsClientDone() {
    // Save whatever state is available to BCI2000
    server_lock->Acquire();
    State("TrialType") = currentTrialType;
    State("CountdownHitReported") = countdownSpacebarPressed;
    State("CountdownMissileScore") = countdownMissileScore;
    State("CountdownAirplaneScore") = countdownAirplaneScore;
    server_lock->Release();
    
    return lastClientPost == STOP_TRIAL;
}

void MongooseTask::FinishRun() {
    server_lock->Acquire();
    runEnded = true;
    isRunning = false;
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

/*
 * Splits a string by a delimiter
 */
static vector<string> split(const std::string input, char delimiter) {
    std::stringstream splitter(input);
    std::string item;
    std::vector<string> items;
    while (getline(splitter, item, delimiter)) {
        if (!item.empty()) {
            items.push_back(item);
        }
    }
    return items;
}

int MongooseServerHandler(struct mg_connection *conn) {
    std::string method(conn->request_method);
    std::string uri(conn->uri);
    bciout << method << " " << uri << endl;

    if (method.compare("POST") == 0) {
        // These methods set an internal variable for the other threads to monitor
        // Whenever the value is read at the appropriate stage of the trial,
        //   'lastClientPost' is reset to CONTINUE
        if (uri.compare("/trial/start") == 0) {
            // Check to see if the application is ready
            // Otherwise, reject the REST call
            if (!currentTask->isRunning) {
                mg_send_status(conn, 418);
                mg_send_data(conn, "", 0);
                return MG_REQUEST_PROCESSED;
            }
            
            // Fetch the next trial type
            if (currentTask->nextTrialType->empty()) {
                currentTask->currentTrialType = static_cast<DynamicFeedbackTask::TrialType>(rand() % DynamicFeedbackTask::LAST);
            } else {
                currentTask->currentTrialType = currentTask->nextTrialType->front();
                currentTask->nextTrialType->pop();
            }
            
            mg_send_status(conn, 200);
            mg_send_header(conn, "Content-Type", "text/plain");
            mg_printf_data(conn, "%d", currentTask->currentTrialType);
            currentTask->lastClientPost = DynamicFeedbackTask::START_TRIAL;

            return MG_REQUEST_PROCESSED;

        } else if (uri.compare("/trial/stop") == 0) {
            mg_send_status(conn, 204);
            mg_send_data(conn, "", 0);
            currentTask->lastClientPost = DynamicFeedbackTask::STOP_TRIAL;
            
            // Process the query string
            std::vector<string> queries = split(string(conn->query_string), '&');
            for (unsigned int i = 0; i < queries.size(); i++) {
                std::vector<string> parts = split(queries[i], '=');
                if (parts.size() != 2) {
                    continue;
                }
                
                // We expect integers
                if (parts[0].compare("spacebar") == 0) {
                    currentTask->countdownSpacebarPressed = stoi(parts[1]);
                } else if (parts[0].compare("missile") == 0) {
                    currentTask->countdownMissileScore = stoi(parts[1]);
                } else if (parts[0].compare("airplane") == 0) {
                    currentTask->countdownAirplaneScore = stoi(parts[1]);
                }
            }

            return MG_REQUEST_PROCESSED;
        }

    } else if (method.compare("GET") == 0) {
        // This allows the Countdown game to poll for a status
        //   HIT means a TMS pulse should be triggerd
        //   REFRESH means the run has ended and the game should refresh
        if (uri.compare("/trial/status") == 0) {
            if (currentTask->targetHit) {
                mg_send_status(conn, 200);
                mg_send_header(conn, "Content-Type", "text/plain");
                mg_printf_data(conn, "HIT");
                currentTask->targetHit = false;
            } else if (currentTask->runEnded) {
                mg_send_status(conn, 200);
                mg_send_header(conn, "Content-Type", "text/plain");
                mg_printf_data(conn, "REFRESH");
                currentTask->runEnded = false;
            } else {
                mg_send_status(conn, 204);
                mg_send_data(conn, "", 0);
            }

            return MG_REQUEST_PROCESSED;
        }
    }

    return MG_REQUEST_NOT_PROCESSED;
}
