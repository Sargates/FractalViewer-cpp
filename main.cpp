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

const int screenWidth = 500;
const int screenHeight = 400;
const float aspectRatio = (float) screenWidth / (float) screenHeight;
const double tenthRootOfTwo = std::pow(2, 1/10.0);
const double oneOverTenthRootOfTwo = std::pow(2, -1.0/10.0);

struct Camera2D {
	double x;
	double y;
	double zoom;

	// Constructor to initialize the camera
	Camera2D(double initialX = 0.0f, double initialY = 0.0f, double initialScale = 1.0f)
		: x(initialX), y(initialY), zoom(initialScale) {}

	// Function to set the camera position
	void setPosition(double newX, double newY) {
		x = newX;
		y = newY;
	}

	// Function to convert world coordinates to screen coordinates
	void worldToScreen(double& worldX, double& worldY) {
		worldX = worldX / zoom - x;
		worldY = worldY / zoom - y;
	}

	// Function to convert screen coordinates to world coordinates
	void screenToWorld(double& screenX, double& screenY) {
		screenX = screenX * zoom + x;
		screenY = screenY * zoom + y;
	}
};

Camera2D camera = Camera2D(0.0, 0.0, 1.0);


int isInSet(std::complex<double> c) {
	std::complex<double> z = (0, 0);
	for (int i = 0; i <= 500; i++) {
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
			camera.worldToScreen(worldX, worldY);
			SDL_Color pixelColor;
			int iters = isInSet(std::complex<double>(worldX, worldY));
			pixelColor.r = (iters % 7) * 32;
			pixelColor.g = (iters % 17) * 16;
			pixelColor.b = (iters % 31) * 8;
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


	// Draw crosshair so user knows where they are
	int centerX = screenWidth/2;
	int centerY = screenHeight/2;
	SDL_Rect outer = { centerX-2, centerY-2, 4, 4 };
	SDL_Rect inner = { centerX-1, centerY-1, 2, 2 };
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(renderer, &outer);
	SDL_SetRenderDrawColor(renderer, 127, 106, 79, 255);
	SDL_RenderDrawRect(renderer, &inner);

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

	// const int numThreads = 32;
	// std::vector<std::thread> threads;

	// for (int i = 0; i < numThreads; ++i) {
	// 	int startY = i * (screenHeight / numThreads);
	// 	int endY = (i + 1) * (screenHeight / numThreads);

	// 	threads.emplace_back(renderRegion, startY, endY);
	// }

	// for (auto& thread : threads) {
	// 	thread.join();
	// }

	// combineAndDisplayFrame();

	// std::cout << "Finished" << std::endl;

	int quit = 0;
	while (!quit) {
		const int numThreads = 40; // Adjust based on your system and requirements
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
			const Uint8* state = SDL_GetKeyboardState(NULL);
			if (state[SDL_SCANCODE_RIGHT]) {
				camera.x -= 0.1 / camera.zoom;
			}
			if (state[SDL_SCANCODE_LEFT]) {
				camera.x += 0.1 / camera.zoom;
			}
			if (state[SDL_SCANCODE_UP]) {
				camera.y += 0.1 / camera.zoom;
			}
			if (state[SDL_SCANCODE_DOWN]) {
				camera.y -= 0.1 / camera.zoom;
			}
			if (state[SDL_SCANCODE_A]) {
				std::cout << camera.zoom << std::endl;
			}
			if(event.type == SDL_MOUSEWHEEL) {
				if(event.wheel.y > 0) {
					camera.zoom *= tenthRootOfTwo;
				}
				else if(event.wheel.y < 0) {
					camera.zoom *= oneOverTenthRootOfTwo;
				}
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