// camera_global.hpp
#ifndef CAMERA_GLOBAL_HPP
#define CAMERA_GLOBAL_HPP

#include <metavision/sdk/stream/camera.h>

// Declaração da instancia cam como global, assim, usar "extern" para qu epossa ser usada em outros arquivos .cpp
extern Metavision::Camera cam;

#endif