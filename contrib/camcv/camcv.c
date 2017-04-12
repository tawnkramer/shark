
/*MODIFIED RASPIVID TO CAMCV.C
This is just a very stripped down version of raspivid (copy right info below), combined with some work from Pierre Raufast, specifically http://raufast.org/download/camcv_vid0.c. 
All it currently does is:
- startup the camera with a preview
- capture the actual memory for each frame of the video
- dump it to disk after 1s
- print out its size every 1s
- shut down after 30s
It's the first step to a reworking to come next that'll wrap up the camera in a slightly more useable api
*/

/*
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, James Hughes
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * \file RaspiVid.c
 * Command line program to capture a camera video stream and encode it to file.
 * Also optionally display a preview/viewfinder of current camera input.
 *
 * \date 28th Feb 2013
 * \Author: James Hughes
 *
 * Description
 *
 * 3 components are created; camera, preview and video encoder.
 * Camera component has three ports, preview, video and stills.
 * This program connects preview and stills to the preview and video
 * encoder. Using mmal we don't need to worry about buffers between these
 * components, but we do need to handle buffers from the encoder, which
 * are simply written straight to the file in the requisite buffer callback.
 *
 * We use the RaspiCamControl code to handle the specific camera settings.
 * We use the RaspiPreview code to handle the (generic) preview window
 */

// We use some GNU extensions (basename)
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sysexits.h>

#define VERSION_STRING "v1.3.3"

//#include "bcm_host.h"
//#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include "RaspiCamControl.h"
#include "RaspiPreview.h"
#include "RaspiCLI.h"

#include <semaphore.h>

/// Camera number to use - we only have one camera, indexed from 0.
#define CAMERA_NUMBER 0

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Video format information
#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3

// Max bitrate we allow for recording
const int MAX_BITRATE = 30000000; // 30Mbits/s

/// Interval at which we check for an failure abort during capture
const int ABORT_INTERVAL = 100; // ms


int mmal_status_to_int(MMAL_STATUS_T status);
static void signal_handler(int signal_number);

// Forward
typedef struct RASPIVID_STATE_S RASPIVID_STATE;

/** Struct used to pass information in encoder port userdata to callback
 */
typedef struct
{
   RASPIVID_STATE *pstate;              /// pointer to our state in case required in callback
   int abort;                           /// Set to 1 in callback if an error occurs to attempt to abort the capture
} PORT_USERDATA;

/** Structure containing all state information for the current run
 */
struct RASPIVID_STATE_S
{
   int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   int width;                          /// Requested width of image
   int height;                         /// requested height of image
   int bitrate;                        /// Requested bitrate
   int framerate;                      /// Requested frame rate (fps)
   int verbose;                        /// !0 if want detailed run information
   
   int framescaptured;

   RASPIPREVIEW_PARAMETERS preview_parameters;   /// Preview setup parameters
   RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters

   MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
   MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera to preview

   MMAL_POOL_T *video_pool; /// Pointer to the pool of buffers used by video output port

   PORT_USERDATA callback_data;        /// Used to move data to the encoder callback
};

/**
 * Assign a default set of parameters to the state passed in
 *
 * @param state Pointer to state structure to assign defaults to
 */
static void default_status(RASPIVID_STATE *state)
{
   if (!state)
   {
      //vcos_assert(0);
      return;
   }

   // Default everything to zero
   memset(state, 0, sizeof(RASPIVID_STATE));

   // Now set anything non-zero
   state->timeout = 30000;     // 5s delay before take image
   state->width = 1280;       // Default to 1080p
   state->height = 720;
   state->bitrate = 17000000; // This is a decent default bitrate for 1080p
   state->framerate = VIDEO_FRAME_RATE_NUM;
   state->verbose = 0;
   state->framescaptured = 0;

   // Setup preview window defaults
   raspipreview_set_defaults(&state->preview_parameters);

   // Set up the camera_parameters to default
   raspicamcontrol_set_defaults(&state->camera_parameters);
}


/**
 *  buffer header callback function for camera control
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
   {
   }
   else
   {
      //vcos_log_error("Received unexpected camera control callback event, 0x%08x", buffer->cmd);
   }

   mmal_buffer_header_release(buffer);
}

/**
 *  buffer header callback function for vieo
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
static void video_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   MMAL_BUFFER_HEADER_T *new_buffer;

   // We pass our file handle and other stuff in via the userdata field.

   PORT_USERDATA *pData = (PORT_USERDATA *)port->userdata;

   if (pData)
   {
      int bytes_written = buffer->length;

      if (buffer->length)
      {
         mmal_buffer_header_mem_lock(buffer);

         //bytes_written = fwrite(buffer->data, 1, buffer->length, pData->file_handle);
		 pData->pstate->framescaptured++;
		 if((pData->pstate->framescaptured % 30) == 0)
		 {
			printf("Got data of %u bytes\n",buffer->length);
			if(pData->pstate->framescaptured == 30)
			{
				FILE* f = fopen("foo.raw","wb");
				fwrite(buffer->data,1,buffer->length,f);
				fclose(f);
				printf("Written %u bytes to disk\n",buffer->length);
			}
		 }

         mmal_buffer_header_mem_unlock(buffer);
      }

      if (bytes_written != buffer->length)
      {
         //vcos_log_error("Failed to write buffer data (%d from %d)- aborting", bytes_written, buffer->length);
         pData->abort = 1;
      }
   }
   else
   {
      //vcos_log_error("Received a encoder buffer callback with no state");
   }

   // release buffer back to the pool
   mmal_buffer_header_release(buffer);

   // and send one back to the port (if still open)
   if (port->is_enabled)
   {
      MMAL_STATUS_T status;

      new_buffer = mmal_queue_get(pData->pstate->video_pool->queue);

      if (new_buffer)
         status = mmal_port_send_buffer(port, new_buffer);

      if (!new_buffer || status != MMAL_SUCCESS)
         //vcos_log_error("Unable to return a buffer to the video port");
   }
}


/**
 * Create the camera component, set up its ports
 *
 * @param state Pointer to state control struct
 *
 * @return MMAL_SUCCESS if all OK, something else otherwise
 *
 */
static MMAL_STATUS_T create_camera_component(RASPIVID_STATE *state)
{
   MMAL_COMPONENT_T *camera = 0;
   MMAL_ES_FORMAT_T *format;
   MMAL_PORT_T *preview_port = NULL, *video_port = NULL, *still_port = NULL;
   MMAL_STATUS_T status;

   /* Create the component */
   status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("Failed to create camera component");
      goto error;
   }

   if (!camera->output_num)
   {
      status = MMAL_ENOSYS;
      //vcos_log_error("Camera doesn't have output ports");
      goto error;
   }

   preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
   video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
   still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

   // Enable the camera, and tell it its control callback function
   status = mmal_port_enable(camera->control, camera_control_callback);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("Unable to enable control port : error %d", status);
      goto error;
   }

   //  set up the camera configuration
   {
      MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
      {
         { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
         .max_stills_w = state->width,
         .max_stills_h = state->height,
         .stills_yuv422 = 0,
         .one_shot_stills = 0,
         .max_preview_video_w = state->width,
         .max_preview_video_h = state->height,
         .num_preview_video_frames = 3,
         .stills_capture_circular_buffer_height = 0,
         .fast_preview_resume = 0,
         .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
      };
      mmal_port_parameter_set(camera->control, &cam_config.hdr);
   }

   // Now set up the port formats

   // Set the encode format on the Preview port
   // HW limitations mean we need the preview to be the same size as the required recorded output

   format = preview_port->format;

   format->encoding = MMAL_ENCODING_OPAQUE;
   format->encoding_variant = MMAL_ENCODING_I420;

   format->es->video.width = state->width;
   format->es->video.height = state->height;
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = state->width;
   format->es->video.crop.height = state->height;
   format->es->video.frame_rate.num = state->framerate;
   format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

   status = mmal_port_format_commit(preview_port);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("camera viewfinder format couldn't be set");
      goto error;
   }

   // Set the encode format on the video  port

   format = video_port->format;

   format->encoding = MMAL_ENCODING_I420;
   format->encoding_variant = MMAL_ENCODING_I420;

   format->es->video.width = state->width;
   format->es->video.height = state->height;
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = state->width;
   format->es->video.crop.height = state->height;
   format->es->video.frame_rate.num = state->framerate;
   format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

   status = mmal_port_format_commit(video_port);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("camera video format couldn't be set");
      goto error;
   }

   // Ensure there are enough buffers to avoid dropping frames
   if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;


   // Set the encode format on the still  port

   format = still_port->format;

   format->encoding = MMAL_ENCODING_OPAQUE;
   format->encoding_variant = MMAL_ENCODING_I420;

   format->es->video.width = state->width;
   format->es->video.height = state->height;
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = state->width;
   format->es->video.crop.height = state->height;
   format->es->video.frame_rate.num = 1;
   format->es->video.frame_rate.den = 1;

   status = mmal_port_format_commit(still_port);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("camera still format couldn't be set");
      goto error;
   }

	//PR : create pool of message on video port
	MMAL_POOL_T *pool;
	//video_port->buffer_size = video_port->buffer_size_recommended;
	//video_port->buffer_num = video_port->buffer_num_recommended;
    // Make a buffer big enough for ARGB data to come back
	video_port->buffer_size = video_port->buffer_size_recommended;
	printf("Creating video port pool with %d buffers of size %d\n", video_port->buffer_num, video_port->buffer_size);
	pool = mmal_port_pool_create(video_port, video_port->buffer_num, video_port->buffer_size);
	if (!pool)
	{
	   //vcos_log_error("Failed to create buffer header pool for video output port");
	}
	state->video_pool = pool;

   /* Ensure there are enough buffers to avoid dropping frames */
   if (still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

   /* Enable component */
   status = mmal_component_enable(camera);

   if (status != MMAL_SUCCESS)
   {
      //vcos_log_error("camera component couldn't be enabled");
      goto error;
   }

   raspicamcontrol_set_all_parameters(camera, &state->camera_parameters);

   state->camera_component = camera;

   if (state->verbose)
      fprintf(stderr, "Camera component done\n");

   return status;

error:

   if (camera)
      mmal_component_destroy(camera);

   return status;
}

/**
 * Destroy the camera component
 *
 * @param state Pointer to state control struct
 *
 */
static void destroy_camera_component(RASPIVID_STATE *state)
{
   if (state->camera_component)
   {
      mmal_component_destroy(state->camera_component);
      state->camera_component = NULL;
   }
}

/**
 * Connect two specific ports together
 *
 * @param output_port Pointer the output port
 * @param input_port Pointer the input port
 * @param Pointer to a mmal connection pointer, reassigned if function successful
 * @return Returns a MMAL_STATUS_T giving result of operation
 *
 */
static MMAL_STATUS_T connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection)
{
   MMAL_STATUS_T status;

   status =  mmal_connection_create(connection, output_port, input_port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);

   if (status == MMAL_SUCCESS)
   {
      status =  mmal_connection_enable(*connection);
      if (status != MMAL_SUCCESS)
         mmal_connection_destroy(*connection);
   }

   return status;
}

/**
 * Checks if specified port is valid and enabled, then disables it
 *
 * @param port  Pointer the port
 *
 */
static void check_disable_port(MMAL_PORT_T *port)
{
   if (port && port->is_enabled)
      mmal_port_disable(port);
}

/**
 * Handler for sigint signals
 *
 * @param signal_number ID of incoming signal.
 *
 */
static void signal_handler(int signal_number)
{
   if (signal_number == SIGUSR1)
   {
      // Handle but ignore - prevents us dropping out if started in none-signal mode
      // and someone sends us the USR1 signal anyway
   }
   else
   {
      // Going to abort on all other signals
      //vcos_log_error("Aborting program\n");
      exit(130);
   }

}

/**
 * main
 */
int test_main(int argc, const char **argv)
{
    /*
   // Our main data storage vessel..
   RASPIVID_STATE state;
   int exit_code = EX_OK;

   MMAL_STATUS_T status = MMAL_SUCCESS;
   MMAL_PORT_T *camera_preview_port = NULL;
   MMAL_PORT_T *camera_video_port = NULL;
   MMAL_PORT_T *camera_still_port = NULL;
   MMAL_PORT_T *preview_input_port = NULL;
//   MMAL_PORT_T *encoder_input_port = NULL;
//   MMAL_PORT_T *encoder_output_port = NULL;
   FILE *output_file = NULL;

   bcm_host_init();

   // Register our application with the logging system
   //vcos_log_register("RaspiVid", //vcos_LOG_CATEGORY);

   signal(SIGINT, signal_handler);

   // Disable USR1 for the moment - may be reenabled if go in to signal capture mode
   signal(SIGUSR1, SIG_IGN);

   default_status(&state);

   // OK, we have a nice set of parameters. Now set up our components
   // We have three components. Camera, Preview and encoder.

   if ((status = create_camera_component(&state)) != MMAL_SUCCESS)
   {
      //vcos_log_error("%s: Failed to create camera component", __func__);
      exit_code = EX_SOFTWARE;
   }
   else if ((status = raspipreview_create(&state.preview_parameters)) != MMAL_SUCCESS)
   {
      //vcos_log_error("%s: Failed to create preview component", __func__);
      destroy_camera_component(&state);
      exit_code = EX_SOFTWARE;
   }
   else
   {
      if (state.verbose)
         fprintf(stderr, "Starting component connection stage\n");

      camera_preview_port = state.camera_component->output[MMAL_CAMERA_PREVIEW_PORT];
      camera_video_port   = state.camera_component->output[MMAL_CAMERA_VIDEO_PORT];
      camera_still_port   = state.camera_component->output[MMAL_CAMERA_CAPTURE_PORT];
      preview_input_port  = state.preview_parameters.preview_component->input[0];

      if (state.preview_parameters.wantPreview )
      {
         if (state.verbose)
         {
            fprintf(stderr, "Connecting camera preview port to preview input port\n");
            fprintf(stderr, "Starting video preview\n");
         }

         // Connect camera to preview
         status = connect_ports(camera_preview_port, preview_input_port, &state.preview_connection);
      }
      else
      {
         status = MMAL_SUCCESS;
      }

      if (status == MMAL_SUCCESS)
      {
         if (state.verbose)
            fprintf(stderr, "Connecting camera stills port to encoder input port\n");

         // Set up our userdata - this is passed though to the callback where we need the information.
         state.callback_data.pstate = &state;
         state.callback_data.abort = 0;
		 camera_video_port->userdata = (struct MMAL_PORT_USERDATA_T *)&state.callback_data;

		 //setup the video buffer callback
         status = mmal_port_enable(camera_video_port, video_buffer_callback);
         if (status != MMAL_SUCCESS)
         {
            //vcos_log_error("Failed to setup video buffer callback");
            goto error;
         }

        // Send all the buffers to the vide output port
        {
            int num = mmal_queue_length(state.video_pool->queue);
            int q;
            for (q=0;q<num;q++)
            {
                MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state.video_pool->queue);

                if (!buffer)
					vcos_log_error("Unable to get a required buffer %d from pool queue", q);

                if (mmal_port_send_buffer(camera_video_port, buffer)!= MMAL_SUCCESS)
					vcos_log_error("Unable to send a buffer to encoder output port (%d)", q);

				printf("Sent buffer %d to video port\n");
            }
        }

		//begin capture
		if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
		{
			printf("Failed to start capture\n");
		   goto error;
		}
         vcos_sleep(state.timeout);
 
      }
      else
      {
         mmal_status_to_int(status);
         vcos_log_error("%s: Failed to connect camera to preview", __func__);
      }

error:

      mmal_status_to_int(status);

      if (state.verbose)
         fprintf(stderr, "Closing down\n");

      // Disable all our ports that are not handled by connections
      check_disable_port(camera_still_port);
 
      if (state.preview_parameters.wantPreview )
         mmal_connection_destroy(state.preview_connection);

      if (state.preview_parameters.preview_component)
         mmal_component_disable(state.preview_parameters.preview_component);

      if (state.camera_component)
         mmal_component_disable(state.camera_component);

      raspipreview_destroy(&state.preview_parameters);
      destroy_camera_component(&state);

      if (state.verbose)
         fprintf(stderr, "Close down completed, all components disconnected, disabled and destroyed\n\n");
   }

   if (status != MMAL_SUCCESS)
      raspicamcontrol_check_configuration(128);

   return exit_code;
   */
   return 0;
}

