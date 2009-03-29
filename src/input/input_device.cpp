
#include "input/input_device.hpp"

InputDevice::InputDevice()
{
    for(int n=0; n<PA_COUNT; n++)
    {
        m_bindings[n].id = -1;
        m_bindings[n].type = Input::IT_NONE;
        m_bindings[n].dir = Input::AD_NEGATIVE;
    }
    
    m_player = "default";
}
// -----------------------------------------------------------------------------
KeyboardDevice::KeyboardDevice()
{
}
// -----------------------------------------------------------------------------
void KeyboardDevice::loadDefaults()
{
    m_bindings[PA_NITRO].id = SDLK_SPACE;
    m_bindings[PA_ACCEL].id = SDLK_UP;
    m_bindings[PA_BRAKE].id = SDLK_DOWN;
    m_bindings[PA_LEFT].id = SDLK_LEFT;
    m_bindings[PA_RIGHT].id = SDLK_RIGHT;
    m_bindings[PA_DRIFT].id = SDLK_LSHIFT;
    m_bindings[PA_RESCUE].id = SDLK_ESCAPE;
    m_bindings[PA_FIRE].id = SDLK_LALT;
    m_bindings[PA_LOOK_BACK].id = SDLK_b;

    m_bindings[PA_NITRO].type = Input::IT_KEYBOARD;
    m_bindings[PA_ACCEL].type = Input::IT_KEYBOARD;
    m_bindings[PA_BRAKE].type = Input::IT_KEYBOARD;
    m_bindings[PA_LEFT].type = Input::IT_KEYBOARD;
    m_bindings[PA_RIGHT].type = Input::IT_KEYBOARD;
    m_bindings[PA_DRIFT].type = Input::IT_KEYBOARD;
    m_bindings[PA_RESCUE].type = Input::IT_KEYBOARD;
    m_bindings[PA_FIRE].type = Input::IT_KEYBOARD;
    m_bindings[PA_LOOK_BACK].type = Input::IT_KEYBOARD;
}
// -----------------------------------------------------------------------------
/** checks if this key belongs to this belongs. if yes, sets action and returns true; otherwise returns false */
bool KeyboardDevice::hasBinding(const int key_id, PlayerAction* action /* out */) const
{
    for(int n=0; n<PA_COUNT; n++)
    {
        if(m_bindings[n].id == key_id)
        {
            *action = (PlayerAction)n;
            return true;
        }
    }// next device
    
    return false;
}

// -----------------------------------------------------------------------------
/** Constructor for GamePadDevice.
 *  \param sdlIndex Index of stick.
 */
GamePadDevice::GamePadDevice(int sdlIndex)
{
    m_type = DT_GAMEPAD;
    m_sdlJoystick = SDL_JoystickOpen(sdlIndex);
    
    m_id = SDL_JoystickName(sdlIndex);
    
    const int count = SDL_JoystickNumAxes(m_sdlJoystick);
    m_prevAxisDirections = new Input::AxisDirection[count];
    
    for (int i = 0; i < count; i++)
        m_prevAxisDirections[i] = Input::AD_NEUTRAL;
    
    m_deadzone = DEADZONE_JOYSTICK;
    
    //m_index = -1;
    
    loadDefaults();
}   // GamePadDevice
// -----------------------------------------------------------------------------
void GamePadDevice::loadDefaults()
{
    /*
     TODO
    m_bindings[PA_NITRO]
    m_bindings[PA_DRIFT]
    m_bindings[PA_RESCUE]
    m_bindings[PA_FIRE]
    m_bindings[PA_LOOK_BACK]
    */
     
    m_bindings[PA_ACCEL].type = Input::IT_STICKMOTION;
    m_bindings[PA_ACCEL].id = 1;
    m_bindings[PA_ACCEL].dir = Input::AD_NEGATIVE;
    
    m_bindings[PA_BRAKE].type = Input::IT_STICKMOTION;
    m_bindings[PA_BRAKE].id = 1;
    m_bindings[PA_BRAKE].dir = Input::AD_POSITIVE;
    
    m_bindings[PA_LEFT].type = Input::IT_STICKMOTION;
    m_bindings[PA_LEFT].id = 0;
    m_bindings[PA_LEFT].dir = Input::AD_NEGATIVE;

    m_bindings[PA_RIGHT].type = Input::IT_STICKMOTION;
    m_bindings[PA_RIGHT].id = 0;
    m_bindings[PA_RIGHT].dir = Input::AD_POSITIVE;
    
    
    /*
     set(GA_CURSOR_UP,
     Input(Input::IT_STICKMOTION, 0, 1, Input::AD_NEGATIVE));
     set(GA_CURSOR_DOWN,
     Input(Input::IT_STICKMOTION, 0, 1, Input::AD_POSITIVE));
     set(GA_CURSOR_LEFT,
     Input(Input::IT_STICKMOTION, 0, 0, Input::AD_NEGATIVE));
     set(GA_CURSOR_RIGHT,
     Input(Input::IT_STICKMOTION, 0, 0, Input::AD_POSITIVE));
     set(GA_CLEAR_MAPPING,
     Input(Input::IT_STICKBUTTON, 0, 2));
     set(GA_ENTER,
     Input(Input::IT_STICKBUTTON, 0, 0),
     set(GA_LEAVE,
     Input(Input::IT_STICKBUTTON, 0, 1),
     
     Input::IT_KEYBOARD
     */
}
// -----------------------------------------------------------------------------
bool GamePadDevice::hasBinding(const int axis, const int value, PlayerAction* action /* out */) const
{
    if(value > -m_deadzone && value < m_deadzone) return false; // within deadzone
    
    for(int n=0; n<PA_COUNT; n++)
    {
        if(m_bindings[n].id == axis)
        {
            if(m_bindings[n].dir == Input::AD_NEGATIVE && value < 0)
            {
                *action = (PlayerAction)n;
                return true;
            }
            else if(m_bindings[n].dir == Input::AD_POSITIVE && value > 0)
            {
                *action = (PlayerAction)n;
                return true;
            }
        }
    }// next device
    
    return false;
}
// -----------------------------------------------------------------------------
/** Destructor for GamePadDevice.
 */
GamePadDevice::~GamePadDevice()
{
    delete m_prevAxisDirections;
    
    SDL_JoystickClose(m_sdlJoystick);
}   // ~GamePadDevice
