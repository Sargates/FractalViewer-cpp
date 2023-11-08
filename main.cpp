#include <iostream>
#include <string>
#include <cmath>
#include <numeric>
#include <complex>
#include <vector>
#include <thread>
#include <mutex>
#include <SDL.h>


std::mutex textureMutex;
SDL_Renderer* renderer;
SDL_Texture* screenTexture;
Uint32* pixels;

const int screenWidth = 1600;
const int screenHeight = 900;
const float aspectRatio = (float) screenWidth / (float) screenHeight;

int isInSet(std::complex<double> c) {
	std::complex<double> z = (0, 0);
	for (int i = 0; i <= 250; i++) {
		z = std::pow(z, 2) + c;
		if (std::norm(z) > 10) {
			return i;
		}
	}
	return 0;
}

void renderRegion(int startY, int endY) {
    for (int x = 0; x < screenWidth; ++x) {
        for (int y = startY; y < endY; ++y) {
            
			double worldX = std::lerp(-2.0*aspectRatio, 2.0*aspectRatio, (x/(float)screenWidth));
			double worldY = std::lerp(-2.0, 2.0, (y/(float)screenHeight));
            SDL_Color pixelColor;
            int iters = isInSet(std::complex<double>(worldX, worldY));
            pixelColor.r = (iters % 8) * 32;
            pixelColor.g = (iters % 16) * 16;
            pixelColor.b = (iters % 32) * 8;
            pixelColor.a = 255;

            int index = x + y * screenWidth;

            // Convert SDL_Color to Uint32 (pixel format)
            Uint32 rgbaColor = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888),
                pixelColor.r, pixelColor.g, pixelColor.b, pixelColor.a);

            pixels[index] = rgbaColor;
        }
    }
}


void combineAndDisplayFrame() {

    std::lock_guard<std::mutex> lock(textureMutex);

    SDL_UpdateTexture(screenTexture, nullptr, pixels, screenWidth * sizeof(Uint32));

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, screenTexture, nullptr, nullptr);

    // Render the frame
    SDL_RenderPresent(renderer);
}




int main(int argc, char* argv[]) {

	if (SDL_Init(SDL_INIT_EVERYTHING) > 0) {
		std::cout << "SDL could not be initialized: " << SDL_GetError();
		return 2;
	}
	SDL_Window* window = nullptr;
	float aspectRatio = (float)screenWidth / (float)screenHeight;
	SDL_CreateWindowAndRenderer(screenWidth, screenHeight, 0, &window, &renderer);
	screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, screenWidth, screenHeight);

	pixels = new Uint32[screenWidth * screenHeight];

	const int numThreads = 32;
	std::vector<std::thread> threads;

	for (int i = 0; i < numThreads; ++i) {
		int startY = i * (screenHeight / numThreads);
		int endY = (i + 1) * (screenHeight / numThreads);

		threads.emplace_back(renderRegion, startY, endY);
	}

	for (auto& thread : threads) {
		thread.join();
	}

	combineAndDisplayFrame();

	// std::cout << "Finished" << std::endl;

	int quit = 0;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
				break;
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == 27) { // If keycode == K_escape
				quit = true;
				break;
			}
		}
	}

	delete[] pixels;
	SDL_DestroyTexture(screenTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}