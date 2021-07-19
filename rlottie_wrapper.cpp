#include <stdlib.h>
#include <string>
#include <map>

#include <stdio.h> // comment

#include "rlottie.h"

#include <emscripten.h>

typedef unsigned char uint8_t;

typedef struct {
  rlottie::Animation* player;
  uint8_t  *buffer;
  int       width;
  int       height;
} LottieHandle;

extern "C" {

EMSCRIPTEN_KEEPALIVE
LottieHandle* lottie_init() {
  //LottieHandle *obj = (LottieHandle *)malloc(sizeof(LottieHandle));
  LottieHandle *obj = (LottieHandle *)calloc(1, sizeof(LottieHandle));

  //printf("LOTTIE_INIT, CREATED HANDLE: 0x%08X\n", (unsigned int)obj);
  return obj;
}

EMSCRIPTEN_KEEPALIVE
void lottie_resize(LottieHandle* handle, int width, int height) {
	if(width == handle->width && height == handle->height) {
		return;
  }

	if(handle->buffer != nullptr) {
    //printf("LOTTIE_RESIZE: BUFFER IS EXISTS, FREEING: 0x%08X\n", (unsigned int)handle->buffer);
    free(handle->buffer);
  }
	
  handle->width  = width;
  handle->height = height;
  handle->buffer = (uint8_t *)calloc(width * height * 4, sizeof(uint8_t));
  //handle->buffer = (uint8_t *) malloc(width * height * 4 * sizeof(uint8_t));
}

EMSCRIPTEN_KEEPALIVE
int lottie_load_from_data(LottieHandle* handle, char *data) {
  //printf("LOTTIE_LOAD_FROM_DATA: 0x%08X\n", (unsigned int)handle);
  
  handle->player = rlottie::Animation::loadFromData(data, "", "", false).release();
  free(data);

  //printf("LOTTIE_LOAD_FROM_DATA: CREATED PLAYER: 0x%08X, STRUCTURE: 0x%08X\n", (unsigned int)handle->player, *(unsigned int*)(handle->player));

  if(!handle->player) {
    return 0;
  }

	return handle->player->totalFrame();
}

EMSCRIPTEN_KEEPALIVE
uint8_t* lottie_buffer(LottieHandle* handle) {
  return handle->buffer;
}
/* 
EMSCRIPTEN_KEEPALIVE
int lottie_frame_count(LottieHandle *handle) {
  if(!handle->player) {
    return 0;
  }
  return handle->player->totalFrame();
} */

EMSCRIPTEN_KEEPALIVE
void lottie_destroy(LottieHandle* handle) {
  //printf("LOTTIE_DESTROY: handle: 0x%08X\n", (unsigned int)handle);

  if(handle->player != nullptr) {
    //printf("LOTTIE_DESTROY: PLAYER IS NOT EMPTY\n");
    delete handle->player;
  } else {
    //printf("LOTTIE_DESTROY: PLAYER IS EMPTY\n");
  }
  
  if(handle->buffer != nullptr) {
    free(handle->buffer);
  }

  //free(handle);

  delete handle;
}

// canvas pixel pix[0] pix[1] pix[2] pix[3] {B G R A}
// lottie pixel pix[0] pix[1] pix[2] pix[3] {R G B A}

EMSCRIPTEN_KEEPALIVE
void lottie_render(LottieHandle* handle, int frameNo) {
  if(handle->player == nullptr) return;

  auto surface = new rlottie::Surface((uint32_t *)handle->buffer, handle->width, handle->height, handle->width * 4);

  //printf("LOTTIE_RENDER: HANDLE: 0x%08X, SURFACE: 0x%08X\n", (unsigned int)handle, (unsigned int)surface);
  //printf("LOTTIE_RENDER: HANDLE: 0x%08X\n", (unsigned int)handle);

  handle->player->renderSync(frameNo, *surface);
  //handle->player->renderSync(frameNo, rlottie::Surface((uint32_t *)handle->buffer, handle->width, handle->height, handle->width * 4));

  //delete surface;

  //return;

 
  int totalBytes = handle->width * handle->height * 4;
  for(int i = 0; i < totalBytes; i += 4) {

    unsigned char a = handle->buffer[i + 3];
    // compute only if alpha is non zero
    if(a) {
	    unsigned char r = handle->buffer[i + 2];
	    unsigned char g = handle->buffer[i + 1];
	    unsigned char b = handle->buffer[i];

	    if(a != 255) { //un premultiply
	       r = (r * 255) / a;
	       g = (g * 255) / a;
	       b = (b * 255) / a;

	       handle->buffer[i] = r;
	    	 handle->buffer[i + 1] = g;
	    	 handle->buffer[i + 2] = b;

	    } else {
	      // only swizzle r and b
	      handle->buffer[i] = r;
	    	handle->buffer[i + 2] = b;
	    }
    }
  }

  delete surface;
}

}
