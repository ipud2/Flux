#include "Application.h"

//#include "../../Editor/SceneConverter.h" // Temp
//#include "../../Editor/SceneLoader.h" // Temp
#include "SceneLoader.h"
#include "Path.h"

#include <ctime>
#include <iostream>

namespace Flux {
    void Application::startGame() {
        //Scene scene;
        //SceneLoader::loadScene(Path("res/Indoors.json"), scene);
        //SceneConverter::convert(Path("res/Indoors.json"), Path("res/Indoors.scene"));
        SceneLoader::loadScene(Path("res/Indoors.scene"), currentScene);
        bool created = renderer.create();
        if (!created)
            return;

        update();
    }

    void Application::update() {
        int frames = 0;

        clock_t nextUpdate = clock();
        clock_t lastFpsCount = clock();

        while (!window.isClosed()) {
            int skipped = 0;

            double elapsed = double(clock() - lastFpsCount) / CLOCKS_PER_SEC;

            if (elapsed > 1) {
                lastFpsCount = clock();
                framesPerSecond = frames;
                std::cout << frames << std::endl;
                frames = 0;
            }

            while (clock() > nextUpdate && skipped < maxSkip) {
                currentScene.update();
                nextUpdate += skipTime;
                skipped++;
            }

            renderer.update(currentScene);

            window.update();
            frames++;
        }
    }
}

int main() {
    Flux::Application app;
    app.startGame();
}
