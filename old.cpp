#include <iostream>
#include <string>
#include <cmath>
#include <numeric>
#include <complex>
#include <SDL.h>


int isInSet(std::complex<double> c) {
	std::complex<double> z = (0, 0);
	for (int i = 0; i <= 25; i++) {
		z = std::pow(z, 2) + c;
		if (std::norm(z) > 10) {
			return i;
		}
	}
	return 0;
}

int main(int argc, char* argv[]) {

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << "SDL could not be initialized: " << SDL_GetError();
		return 2;
	}
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	int width = 1600;
	int height = 900;
	float aspectRatio = (float)width / (float)height;
	SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
	// SDL_RenderSetScale(renderer, 2, 2);

	std::cout << "" << (float)width / (float)height;
	for (double i = 0.0; i < 1.0; i += 0.0005) {
		for (double j = 0.0; j < 1.0; j += 0.0005) {
			double x = std::lerp(-2.0 * aspectRatio, 2.0 * aspectRatio, i);
			double y = std::lerp(-2.0, 2.0, j);
			int iters = isInSet(std::complex<double>(x, y));
			if (iters == 0) {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderDrawPointF(renderer, i * width, j * height);
			}
			else {
				SDL_SetRenderDrawColor(
					renderer,
					(iters % 8) * 32,
					(iters % 16) * 16,
					(iters % 32) * 8,
					255
				);
				SDL_RenderDrawPointF(renderer, i * width, j * height);
			}
		}
	}

	SDL_RenderPresent(renderer);

	int quit = 0;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
				break;
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 27) { // If keycode = K_escape
				quit = true;
				break;
			}
		}
	}


	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}