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


    constexpr std::uint64_t mapWidth = 100, mapHeight = mapWidth, mapDepth = mapWidth;

    constexpr std::uint64_t mapSize = mapWidth*mapHeight*mapDepth;

  //  std::uint8_t* map = new std::uint8_t[mapSize];

    std::uint8_t*** map = new std::uint8_t**[mapWidth];

    constexpr float degreeConst = 0.0174533f;//1 degree in radians

    //this can be used for the x and y const since the scale in the x and y should be kept the same
    constexpr float sepn = 64.f;

    constexpr sf::Color backgroundColor = sf::Color(10,10,10);
    constexpr sf::Color wallColor = sf::Color(50,50,200);

    
    void setMap(){
  for (size_t i=0; i<mapWidth; i++){
                map[i] = new std::uint8_t*[mapHeight];
                    for (size_t j=0; j<mapHeight; j++){
                        map[i][j] = new std::uint8_t[mapDepth];
                            for (size_t k=0; k<mapDepth; k++){
                                map[i][j][k] = !(rand()%10);
                            }    
                    }
            }
    }
    
   /* 
    void setMap(){
        for (size_t i = 0; i < mapSize; i++)
        {
                map[i] = (!(rand()%10));
                //std::cout << (int)map[i];//cout defaults to reading unsigned 8 bit ints as chars/symbols
        }
    }*/
    

    struct Ray{
        sf::Vector3f pos; //x,y,z
        sf::Vector2f angles; //yaw and pitch
    };
    

    int snapFloat(const float input, const float snapScale){
        return floor(input/snapScale)*snapScale;
    }


    sf::Vector3i snapFloat3(const sf::Vector3f& input,const float snapScale){
        return sf::Vector3i{snapFloat(input.x,snapScale),snapFloat(input.y,snapScale),snapFloat(input.z,snapScale)};
    }


    size_t coordsToN(const sf::Vector3i& coords,const sf::Vector3i& mapDimensions){//I wish SFML had 3u
        const size_t xyComp = coords.x + coords.y*mapDimensions.x;
        return xyComp + coords.z*mapDimensions.y*mapDimensions.x;
    }


    float clamp(float input, float min, float max){
        if (input>max) return max;
        if (input<min) return min;
        return input;
    }
    

    void resolvePixels(const sf::Vector3f& cameraPosition, const sf::Vector3i& mapDimensions, const float snapScale, const float viewAngleDegrees, const size_t maxRaySteps, sf::Image& renderImage,sf::RenderTarget& target)
    {
                size_t pixelX;
                size_t pixelY;
                size_t resX = renderImage.getSize().x;
                size_t resY = renderImage.getSize().y;
                float resXHalf = resX/2.f;
                float resYHalf = resY/2.f;
                const float pixelAngleConst = viewAngleDegrees/resX;

                Ray pixelRay;

                sf::Vector3i snappedPos;

                size_t rayStepN;
                bool collision;

                sf::CircleShape circ(2,3);

            for (pixelY = 0; pixelY<resY; pixelY++)
            {
                for (pixelX = 0; pixelX<resX;pixelX++)
                {
                        pixelRay.angles.y = (((float)(pixelY-resXHalf)/resX)*pixelAngleConst)*viewAngleDegrees;//made it resXHalf to keep the aspect ratio
                                pixelRay.pos.y = cameraPosition.y;
                   
                                pixelRay.pos.x = cameraPosition.x;
                                pixelRay.angles.x = (((float)(pixelX-resXHalf)/resX)*pixelAngleConst)*viewAngleDegrees;
                                pixelRay.pos.z = cameraPosition.z;
                                collision = 0;//this is currently unused
                            for (rayStepN = 0; rayStepN < maxRaySteps; rayStepN++)
                            {
                                        //circ.setPosition({pixelRay.pos.x,pixelRay.pos.y});
                                        //target.draw(circ);
                                        snappedPos = snapFloat3(pixelRay.pos,snapScale);
                                        snappedPos = snappedPos.componentWiseDiv({64,64,64});
                                     //    size_t mapIndex = coordsToN(snappedPos,mapDimensions);
                                   // if (mapIndex<mapSize)
                                    if (snappedPos.x<mapWidth && snappedPos.y<mapHeight && snappedPos.z<mapDepth)
                                    {
                                            //if (map[mapIndex])
                                          // std::cout << snappedPos.x << " " << snappedPos.y << " " << snappedPos.z << std::endl;
                                              if (map[snappedPos.x][snappedPos.y][snappedPos.z])
                                            {
                                                        const float xdist = cameraPosition.x - pixelRay.pos.x;
                                                        const float ydist = cameraPosition.y - pixelRay.pos.y;
                                                        const float zdist = cameraPosition.z - pixelRay.pos.z;
                                
                                                        float sdist = sqrt(xdist*xdist + ydist*ydist + zdist*zdist);
                                                    
                                                        if (sdist<1.f) sdist = 1.f;

                                                        std::uint8_t tempColor = clamp(10000.f/sdist,0,255);

                                                        renderImage.setPixel({(unsigned)pixelX,(unsigned)pixelY},sf::Color(tempColor,tempColor,tempColor));
                                                        collision = 1;
                                                        //std::cout << "hit";
                                                        break;
                                            }
                                            else
                                            {
                                                        //advance ray if no collision is found
                                                        pixelRay.pos += sf::Vector3f{sin(pixelRay.angles.x)*snapScale/20,cos(pixelRay.angles.x)*snapScale/20,sin(pixelRay.angles.y)*snapScale/20};//the z is missing something. x and y seem to be functioning properly. It doesn't appear to be an issue with the indexing function
                                            }
                                    }
                                    else
                                    {
                                    break;
                                    }
                        
                            }
                    }
            }
    }

int main(){


    setMap();

    bool tabLock = 0;
    std::string windowName = "App";
    bool fullscreenEnabled = 0;
    sf::Vector2u windowResolution{192,108};
    sf::RenderWindow window(sf::VideoMode({100,100}), windowName,sf::State::Windowed);
    
    if (fullscreenEnabled)
    window.create(sf::VideoMode(windowResolution), windowName,sf::State::Fullscreen);
        else
    window.create(sf::VideoMode(windowResolution), windowName,sf::State::Windowed);


    window.setFramerateLimit(63);
    window.setVerticalSyncEnabled(1);

    sf::Image outImage(window.getSize(),sf::Color(255,255,255,255));
    sf::Texture outTexture(outImage);
  
    sf::Sprite outSprite{outTexture};

    float viewAngleDegrees = 60.f; 

    sf::Vector3f cameraPos;

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

    window.clear(sf::Color::Black);
   

    cameraPos = {(float)sf::Mouse::getPosition().x,(float)sf::Mouse::getPosition().y,500};

    outImage.resize(outImage.getSize(),backgroundColor);//this clears the image to black

    resolvePixels(cameraPos, {mapWidth,mapHeight,mapDepth}, sepn, viewAngleDegrees, 90, outImage,window);

     outTexture.update(outImage);
    window.draw(outSprite);


    window.display();
 
}//has focus
}//while open

delete[] map;

return 0;

}


