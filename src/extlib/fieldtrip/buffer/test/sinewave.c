/*
 * This piece of code demonstrates how an acquisition device could be
 * writing header information (e.g. number of channels and sampling
 * frequency) and streaming EEG data to the buffer.
 *
 * Copyright (C) 2008, Robert Oostenveld
 * F.C. Donders Centre for Cognitive Neuroimaging, Radboud University Nijmegen,
 * Kapittelweg 29, 6525 EN Nijmegen, The Netherlands
 *
 * $Log: sinewave.c,v $
 * Revision 1.4  2008/07/09 10:40:08  roboos
 * some minor cleanup
 *
 * Revision 1.3  2008/07/09 10:10:34  roboos
 * some cleanup
 *
 * Revision 1.2  2008/07/08 20:24:23  roboos
 * greatly simplified example code, removed runlevel, removed configurable options (properties), moved event-writing to another demo
 *
 *
 */

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "message.h"
#include "buffer.h"
#include "socket_includes.h"
#include "unix_includes.h"

#define FREQ       1
#define PI         3.1415926

void *sinewave_thread(void *arg) {
	int i, j, sample = 0, status = 0;
	char str[256];
	long tdif;
	host_t *host = (host_t *)arg;

	/* these represent the acquisition system properties */
	int nchans         = 16;
	int fsample        = 250;
	int blocksize      = 125;
	int stopthread	   = 0;

	/* these are used in the communication and represent statefull information */
	int server             = -1;
	message_t    *request  = NULL;
	message_t    *response = NULL;
	header_t     *header   = NULL;
	data_t       *data     = NULL;
	event_t      *event    = NULL;

	/* this is to prevent closing the thread at an unwanted moment and memory from leaking */
	int oldcancelstate, oldcanceltype;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldcancelstate);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldcanceltype);
	pthread_cleanup_push(cleanup_message, request);
	pthread_cleanup_push(cleanup_message, response);
	pthread_cleanup_push(cleanup_header,  header);
	pthread_cleanup_push(cleanup_data,    data);
	pthread_cleanup_push(cleanup_event,   event);
	pthread_cleanup_push(cleanup_socket,  &server);

	/* this determines the hostname and port on which the client will write */
	if (!arg)
		exit(1);

	fprintf(stderr, "sinewave: host.name =  %s\n", host->name);
	fprintf(stderr, "sinewave: host.port =  %d\n", host->port);

	/* allocate the elements that will be used in the communication */
	request      = malloc(sizeof(message_t));
	request->def = malloc(sizeof(messagedef_t));
	request->buf = NULL;
	request->def->version = VERSION;
	request->def->bufsize = 0;

	header      = malloc(sizeof(header_t));
	header->def = malloc(sizeof(headerdef_t));
	header->buf = NULL;

	data      = malloc(sizeof(data_t));
	data->def = malloc(sizeof(datadef_t));
	data->buf = NULL;

	event      = malloc(sizeof(event_t));
	event->def = malloc(sizeof(eventdef_t));
	event->buf = NULL;

	/* define the header */
	header->def->nchans    = nchans;
	header->def->nsamples  = 0;
	header->def->nevents   = 0;
	header->def->fsample   = fsample;
	header->def->data_type = DATATYPE_FLOAT32;
	header->def->bufsize   = 0;
	FREE(header->buf);

	/* define the data definition that stays constant and allocate space for the variable part */
	data->def->nchans    = nchans;
	data->def->nsamples  = blocksize;
	data->def->data_type = DATATYPE_FLOAT32;
	data->def->bufsize   = WORDSIZE_FLOAT32*nchans*blocksize;
	FREE(data->buf);
	data->buf            = malloc(WORDSIZE_FLOAT32*nchans*blocksize);

	/* define the event definition that stays constant and allocate space for the variable part */
	event->def->type_type   = DATATYPE_CHAR; 
	event->def->type_numel  = strlen("time");
	event->def->value_type  = DATATYPE_CHAR;
	event->def->value_numel = strlen(str);
	event->def->offset      = 0;
	event->def->duration    = 0;
	event->def->bufsize     = 256;
	FREE(event->buf);
	event->buf              = malloc(256);

    /* the stopthread property is used to signal that the thread should exit */
	server = open_remotehost(host->name, host->port);
	status = set_property(server, "sinewaveStopThread", &stopthread);
	if (server>=0)
		closesocket(server);

	/* initialization phase, send the header */
	request->def->command = PUT_HDR;
	request->def->bufsize = append(&request->buf, request->def->bufsize, header->def, sizeof(headerdef_t));
	request->def->bufsize = append(&request->buf, request->def->bufsize, header->buf, header->def->bufsize);

	server = open_remotehost(host->name, host->port);
	clientrequest(server, request, &response);
	if (server>=0)
		closesocket(server);

	/* FIXME do someting with the response, i.e. check that it is OK */
	request->def->bufsize = 0;
	FREE(request->buf);
	if (response) {
		FREE(response->def);
		FREE(response->buf);
		FREE(response);
	}

	while (1) {

        /* the stopthread property is used to signal that the thread should exit */
		server = open_remotehost(host->name, host->port);
		status = get_property(server, "sinewaveStopThread", &stopthread);
		if (server>=0)
			closesocket(server);
		if(stopthread == 1)
			goto cleanup;

		for (j=0; j<blocksize; j++) {
			if (nchans>0)
				((FLOAT32_T *)(data->buf))[j*nchans+0] = sample;
			if (nchans>1)
				((FLOAT32_T *)(data->buf))[j*nchans+1] = sin(2*PI*FREQ*sample/fsample);
			if (nchans>2)
				((FLOAT32_T *)(data->buf))[j*nchans+2] = 2.0*((FLOAT32_T)rand())/RAND_MAX - 1.0;
			if (nchans>3)
				for (i=3; i<nchans; i++)
					((FLOAT32_T *)(data->buf))[j*nchans+i] = sin(2*PI*FREQ*sample/fsample) + 2.0*((FLOAT32_T)rand())/RAND_MAX - 1.0;
			sample++;
		}

		request->def->command = PUT_DAT;
		request->def->bufsize = append(&request->buf, request->def->bufsize, data->def, sizeof(datadef_t));
		request->def->bufsize = append(&request->buf, request->def->bufsize, data->buf, data->def->bufsize);

		server = open_remotehost(host->name, host->port);
		clientrequest(server, request, &response);
		if (server>=0)
			closesocket(server);

		/* FIXME do someting with the response, i.e. check that it is OK */
		request->def->bufsize = 0;
		FREE(request->buf);
		if (response) {
			FREE(response->def);
			FREE(response->buf);
			FREE(response);
		}

		/* approximate delay in microseconds */
		tdif = (long)(blocksize * 1000000 / fsample);
		usleep(tdif);

	} /* while(1) */

cleanup:
	/* from now on it is save to cancel the thread */
	pthread_setcancelstate(oldcancelstate, NULL);
	pthread_setcanceltype(oldcanceltype, NULL);

	pthread_cleanup_pop(1);  /* server */
	pthread_cleanup_pop(1);  /* event */
	pthread_cleanup_pop(1);  /* data */
	pthread_cleanup_pop(1);  /* header */
	pthread_cleanup_pop(1);  /* response */
	pthread_cleanup_pop(1);  /* request */

	pthread_exit(NULL);
	return NULL;
}

