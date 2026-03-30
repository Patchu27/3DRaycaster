#include <iostream>
#include <functional>
#include <SFML/Graphics.hpp>
#include <time.h>
#include <chrono>
#include <math.h>

#include <string>
#include <source_location> // https://en.cppreference.com/w/cpp/utility/source_location
#include <filesystem>



//For slightly enhanced debugging
std::ostream& dump_location(std::source_location loc = std::source_location::current())
{
    return std::cout << std::filesystem::path( loc.file_name() ).filename() << ", function: "
                     << loc.function_name() << ", line: " << loc.line() << '\n' ;
}

namespace Timer{

static std::chrono::time_point<std::chrono::high_resolution_clock> chronoTime;
static inline std::source_location beginLocation = std::source_location::current();

//call this function to reset the (wall clock) timer and record the line it's on
void resetTimer(std::source_location loc = std::source_location::current()){
   beginLocation = std::forward<std::source_location>(loc);
   chronoTime =  std::chrono::high_resolution_clock::now();
}

//call this function to get the duration between now and when the timer was reset. Outputs start and end lines, and the duration
void getDuration(std::source_location loc = std::source_location::current()){
    std::cout << "Duration from Lines: (start) " <<  beginLocation.line() << " to (end) " << loc.line() << " -- "; 
    std::cout << std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - chronoTime) << '\n';
}

}




int main(){

    bool tabLock = 0;
    std::string windowName = "App";
    bool fullscreenEnabled = 1;
    sf::Vector2u windowResolution{1920,1080};
    sf::RenderWindow window(sf::VideoMode({100,100}), windowName,sf::State::Windowed);
    
    if (fullscreenEnabled)
    window.create(sf::VideoMode(windowResolution), windowName,sf::State::Fullscreen);
        else
    window.create(sf::VideoMode(windowResolution), windowName,sf::State::Windowed);


    window.setFramerateLimit(63);
    window.setVerticalSyncEnabled(1);

    sf::RenderTexture outRenderTexture(window.getSize());
    sf::Sprite outSprite{outRenderTexture.getTexture()};

    while (window.isOpen()){
 
     if (window.hasFocus()){
        
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()||sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                window.close();
                
              
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Tab)){
                if (!tabLock){
                tabLock = 1;
                
                fullscreenEnabled = !fullscreenEnabled;

                if (fullscreenEnabled) 
                    window.create(sf::VideoMode(window.getSize()), windowName,sf::State::Fullscreen);
                    else
                    window.create(sf::VideoMode(window.getSize()), windowName,sf::Style::Default,sf::State::Windowed);
                }
                }
                else{
                tabLock = 0;
                }
            }//poll events

    window.clear(sf::Color::White);
    outRenderTexture.clear(sf::Color::White);


    

    outRenderTexture.display();    

    window.draw(outSprite);


    window.display();
 
}//has focus
}//while open


}


