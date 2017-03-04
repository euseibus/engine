#include "NoiseType.h"
#include <SDL.h>

static const char *NoiseTypeStr[] = {
	"double noise",
	"simplex noise",
	"ridged noise",
	"flow noise (rot. gradients)",
	"fbm",
	"fbm cascade",
	"fbm analytical derivatives",
	"flow noise fbm (time)",
	"ridged multi fractal (time)",
	"ridged multi fractal",
	"ridged multi fractal cascade",
	"iq noise",
	"iq noise scaled",
	"analytical derivatives",
	"noise curl noise (time)",
	"worley noise",
	"worley noise fbm",
	"voronoi",
	"swissTurbulence",
	"jordanTurbulence"
};
static_assert((int)SDL_arraysize(NoiseTypeStr) == (int)NoiseType::Max, "String array size doesn't match noise types");

const char *getNoiseTypeName(NoiseType t) {
	return NoiseTypeStr[(int)t];
}