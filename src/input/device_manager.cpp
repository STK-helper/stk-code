

#include "input/device_manager.hpp"

DeviceManager::DeviceManager()
{
    m_keyboard_amount = 0;
    m_gamepad_amount = 0;
}

void DeviceManager::add(KeyboardDevice* d)
{
    m_keyboards.push_back(d);
    m_keyboard_amount = m_keyboards.size();
}

void DeviceManager::add(GamePadDevice* d)
{
    m_gamepads.push_back(d);
    m_gamepad_amount = m_gamepads.size();
}

// TODO
bool DeviceManager::mapInputToPlayerAndAction( Input::InputType type, int id0, int id1, int id2, int value,
                                              int* player /* out */, PlayerAction* action /* out */ )
{
    // TODO - auto-detect player ID from device
    *player = 0;

    if(type == Input::IT_KEYBOARD)
    {
        for(unsigned int n=0; n<m_keyboard_amount; n++)
        {
            if( m_keyboards[n].hasBinding(id0, action) ) return true;
        }
        return false;
    }
    else if(type == Input::IT_MOUSEBUTTON)
    {
        return false;
    }
    else if(type == Input::IT_STICKBUTTON)
    {
        return false;
    }
    else if(type == Input::IT_STICKMOTION)
    {
        // std::cout << "stick motion, ID=" <<id0 << " axis=" << id1 << " value=" << value  << std::endl;
        for(unsigned int n=0; n<m_gamepad_amount; n++)
        {
            if( /*m_gamepads[n].m_index == id0 &&*/ m_gamepads[n].hasBinding(id1 /* axis */, value, action) ) return true;
        }
        return false;
    }
    else
    {
        return false;
    }
    
    return false;
}