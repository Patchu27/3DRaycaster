#include <iostream>
#include <functional>
#include <SFML/Graphics.hpp>
#include <time.h>
#include <chrono>
#include <math.h>
#include <thread>

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

    constexpr std::uint64_t mapXYArea = mapWidth*mapHeight;//used in index function

    constexpr std::uint64_t mapSize = mapWidth*mapHeight*mapDepth;


    std::uint8_t* map = new std::uint8_t[mapSize];


    constexpr float degreeConst = 0.0174533f;//1 degree in radians

    constexpr float maxFOVDegrees = 135.f;
    constexpr float minFOVDegrees = 30.f;

    constexpr float illuminationDistance = 10000.f;

    //this can be used for the x and y const since the scale in the x and y should be kept the same
    constexpr float sepn = 64.f;

    constexpr sf::Color backgroundColor = sf::Color(10,10,10);
    constexpr sf::Color wallColor = sf::Color(50,50,200);

    
    void setMap(){
        for (size_t i = 0; i < mapSize; i++)
        {
                map[i] = (!(rand()%5));
                if (i%mapWidth==0) map[i] = 1;
                //std::cout << (int)map[i];//cout defaults to reading unsigned 8 bit ints as chars/symbols
        }
    }
    

    struct Ray{
        sf::Vector3f pos; //x,y,z
        sf::Vector2f angles; //yaw and pitch
    };
    
/*
    int snapFloat(const float input, const float snapScale){
        return floor(input/snapScale)*snapScale;
    }


    sf::Vector3i snapFloat3(const sf::Vector3f& input,const float snapScale){
        return sf::Vector3i{snapFloat(input.x,snapScale),snapFloat(input.y,snapScale),snapFloat(input.z,snapScale)};
    }
*/

    sf::Vector3i getGridPos(const sf::Vector3f& input, const float snapScale){
        return sf::Vector3i{(int)floor(input.x/snapScale),(int)floor(input.y/snapScale),(int)floor(input.z/snapScale)};
    }

    size_t coordsToN(const sf::Vector3i& coords,const sf::Vector3i& mapDimensions){//I wish SFML had 3u
        const size_t xyComp = coords.x + coords.y*mapDimensions.x;
        //old line//return xyComp + coords.z*mapDimensions.y*mapDimensions.x;
        return xyComp + coords.z*mapXYArea;
    }


    float clamp(float input, float min, float max){
        if (input>max) return max;
        if (input<min) return min;
        return input;
    }
    

    void resolvePixels(const sf::Vector3f& cameraPosition, const sf::Vector2f& cameraRotation, const float FOVDegrees, const sf::Vector3i& mapDimensions, const float snapScale, const size_t maxRaySteps, sf::Image& renderImage,sf::RenderTarget& target)
    {
                size_t pixelX;
                size_t pixelY;
                size_t resX = renderImage.getSize().x;
                size_t resY = renderImage.getSize().y;
                const float resXHalf = (float)resX/2.f;
                const float resYHalf = (float)resY/2.f;

                //this was no longer needed, and I was confusing it with degreeConst, which was causing me to have scaling problems for hours... leaving this in here for now, will be removed in the next commit
                //const float pixelAngleConst = FOVDegrees/resX;

                Ray pixelRay;

                sf::Vector3i snappedPos;

                size_t rayStepN;
                bool collision;

                //sf::CircleShape circ(2,3);
                
                float substepVal = snapScale/20.f;
                

            for (pixelY = 0; pixelY<resY; pixelY++)
            {
                        pixelRay.angles.y = (((float)pixelY-resYHalf)/resX)*FOVDegrees*degreeConst + cameraRotation.y;
                    
                    for (pixelX = 0; pixelX<resX;pixelX++)
                    {
                                
                                pixelRay.angles.x = (((float)pixelX-resXHalf)/resX)*FOVDegrees*degreeConst + cameraRotation.x;//still leaving this in for clarity since I'm still working on it//soon I'll combine these consts to be more efficient but I'm leaving it for now to make the fix clear

                                sf::Vector3f rayVec = sf::Vector3f{sin(pixelRay.angles.x),cos(pixelRay.angles.x),sin(pixelRay.angles.y)}.componentWiseMul({substepVal,substepVal,substepVal});

                                pixelRay.pos = cameraPosition + rayVec.componentWiseMul({substepVal,substepVal,substepVal});

                                collision = 0;//this is currently unutilized
                            
                            for (rayStepN = 0; rayStepN < maxRaySteps; rayStepN++)
                            {
                                        //circ.setPosition({pixelRay.pos.x,pixelRay.pos.y});
                                        //target.draw(circ);

                                         snappedPos = getGridPos(pixelRay.pos,snapScale);
                                         size_t mapIndex = coordsToN(snappedPos,mapDimensions);
                                    if (mapIndex<mapSize)
                                    {
                                            if (map[mapIndex])
                                            {
                                                        const float xdist = cameraPosition.x - pixelRay.pos.x;
                                                        const float ydist = cameraPosition.y - pixelRay.pos.y;
                                                        const float zdist = cameraPosition.z - pixelRay.pos.z;
                                
                                                        float sdist = sqrt(xdist*xdist + ydist*ydist + zdist*zdist);
                                                    
                                                        if (sdist<1.f) sdist = 1.f;

                                                        std::uint8_t tempColor = clamp(illuminationDistance/sdist,0,255);
                                                        renderImage.setPixel({(unsigned)pixelX,(unsigned)pixelY},sf::Color(tempColor,tempColor,tempColor));
                                                        collision = 1;
                                                        //std::cout << "hit";
                                                        break;
                                            }
                                            else
                                            {
                                                        //advance ray if no collision is found
                                                        pixelRay.pos += rayVec;
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
    bool fullscreenEnabled = 1;
    sf::Vector2u windowResolution{1920,1080};
    sf::RenderWindow window(sf::VideoMode({100,100}), windowName,sf::State::Windowed);
    
    if (fullscreenEnabled)
    window.create(sf::VideoMode(windowResolution), windowName,sf::State::Fullscreen);
        else
    window.create(sf::VideoMode(windowResolution), windowName,sf::State::Windowed);


    window.setFramerateLimit(63);
    window.setVerticalSyncEnabled(1);


    const float resolutionScale = 0.08f; 
   
    sf::Image outImage(sf::Vector2u(windowResolution.x*resolutionScale,windowResolution.y*resolutionScale),sf::Color(255,255,255,255));
    sf::Texture outTexture(outImage);
  

    sf::Sprite outSprite{outTexture};

    sf::Vector3f cameraPos;
    
    float FOVDegrees = 90.f;
    float mouseSensitivity = 0.001f;
    sf::Vector2f cameraRotation{0.f,0.f};

    float flySpeed = 5.f;

    /* *window defined* cursor functions from SFML site
    setMouseCursorVisible (bool visible)
 	Show or hide the mouse cursor.
void 	setMouseCursorGrabbed (bool grabbed)
 	Grab or release the mouse cursor.
void 	setMouseCursor (const Cursor &cursor)
    */

    window.setMouseCursorVisible(0);


    while (window.isOpen()){
 
     if (window.hasFocus()){
        //poll events
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            
            if (const auto* mouseMovedRaw = event->getIf<sf::Event::MouseMovedRaw>())
            {
                        cameraRotation += {(float)mouseMovedRaw->delta.x*mouseSensitivity,(float)mouseMovedRaw->delta.y*mouseSensitivity};
                    }   
                    
                }
                sf::Mouse::setPosition((sf::Vector2i)window.getSize().componentWiseDiv({2,2}));

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) window.close();//moved these out of the polling loop because they do not need to be repeated for no reason
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Tab))
        {

                if (!tabLock){
                            tabLock = 1;
                            fullscreenEnabled = !fullscreenEnabled;
                        if (fullscreenEnabled) 
                                    window.create(sf::VideoMode(window.getSize()), windowName,sf::State::Fullscreen);
                        else
                                    window.create(sf::VideoMode(window.getSize()), windowName,sf::Style::Default,sf::State::Windowed);
                        }
                        window.setFramerateLimit(63);
                        window.setVerticalSyncEnabled(1);//sfml needs a better way to track these variables I don't feel like making another wrapper right now
                        window.setMouseCursorVisible(0);
        }
        else
        {
            tabLock = 0;
        }


    window.clear(sf::Color::Black);
   

    //CONTROLS
    //cameraPos = {(float)sf::Mouse::getPosition().x,(float)sf::Mouse::getPosition().y,500};
    //cameraPos = {600,500,500};

    const std::int8_t frontDirection = (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)-sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S));
    const std::int8_t sideDirection = (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)-sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A));

    //const sf::Vector2f movementVector = cameraRotation + sf::Vector2f({(90*degreeConst*sideDirection-180.f*degreeConst*frontDirection)/(1+((!!frontDirection)&&(!!sideDirection))),0.f});

   // if ((!!frontDirection)||(!!sideDirection))
    //cameraPos += sf::Vector3f(sin(movementVector.x)*flySpeed,cos(movementVector.x)*flySpeed,sin(movementVector.y));
    cameraPos += sf::Vector3f(sin(cameraRotation.x)*flySpeed*(float)frontDirection,cos(cameraRotation.x)*flySpeed*(float)frontDirection,sin(cameraRotation.y)*flySpeed*(float)frontDirection);

    FOVDegrees = clamp(FOVDegrees*(1.f+0.05f*(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::O)-sf::Keyboard::isKeyPressed(sf::Keyboard::Key::I))),minFOVDegrees,maxFOVDegrees);

    outImage.resize(outImage.getSize(),backgroundColor);//this clears the image to black
     //snappedPos = snapFloat3(pixelRay.pos,snapScale);
                                       // snappedPos = snappedPos.componentWiseDiv({64,64,64});
    //cameraRotation += sf::Vector2f{0.01f,0.0f};
    //CONTROLS
            //scaling has to be adjusted by the square root of the previous for the FOV to be the same, which makes sense because the scaling is determined by the X width of the screen and used twice to make the square
    resolvePixels(cameraPos, cameraRotation, FOVDegrees, {mapWidth,mapHeight,mapDepth}, sepn, 200, outImage,window);

    outTexture.update(outImage);
    outSprite.setScale(((sf::Vector2f)window.getSize()).componentWiseDiv((sf::Vector2f)outImage.getSize()));
    window.draw(outSprite);


    window.display();
 
}//has focus
}//while open

delete[] map;

return 0;

}


