//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016-2017 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifdef MOBILE_STK

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#ifdef ANDROID
#include "SDL_system.h"
#include <jni.h>
std::string g_android_main_user_agent;

extern int android_main(int argc, char *argv[]);

void override_default_params_for_mobile();
extern "C" int SDL_main(int argc, char *argv[])
{
    override_default_params_for_mobile();
    int result = android_main(argc, argv);
    // TODO: Irrlicht device is properly waiting for destroy event, but
    // some global variables are not initialized/cleared in functions and thus
    // its state is remembered when the window is restored. We will use exit
    // call to make sure that all variables are cleared until a proper fix will
    // be done.
    fflush(NULL);
    _exit(0);
    return 0;
}
#endif

void override_default_params_for_mobile()
{
    // It has an effect only on the first run, when config file is created.
    // So that we can still modify these params in STK options and user's
    // choice will be then remembered.
    
    // Set smaller texture size to avoid high RAM usage
    UserConfigParams::m_max_texture_size = 256;
    UserConfigParams::m_high_definition_textures = false;
    
    // Enable advanced lighting only for android >= 8
#ifdef ANDROID
    UserConfigParams::m_dynamic_lights = (SDL_GetAndroidSDKVersion() >= 26);
#endif

    // Disable light scattering for better performance
    UserConfigParams::m_light_scatter = false;

    // Enable multitouch race GUI
    UserConfigParams::m_multitouch_draw_gui = true;

#ifdef IOS_STK
    // Default 30 fps for battery saving (only used in iOS)
    UserConfigParams::m_swap_interval = 2;
#endif

#ifdef ANDROID
    // For usage in StringUtils::getUserAgentString
    if (SDL_IsAndroidTV())
    {
        // For some android tv sdl returns a touch screen device even it doesn't
        // have
        UserConfigParams::m_multitouch_draw_gui = false;
        g_android_main_user_agent = " (AndroidTV)";
    }
    else if (SDL_IsChromebook())
        g_android_main_user_agent = " (ChromeOS)";
    else
        g_android_main_user_agent = " (Android)";

    // Get some info about display
    const int SCREENLAYOUT_SIZE_SMALL = 1;
    const int SCREENLAYOUT_SIZE_NORMAL = 2;
    const int SCREENLAYOUT_SIZE_LARGE = 3;
    const int SCREENLAYOUT_SIZE_XLARGE = 4;
    int32_t screen_size = 0;
    int ddpi = 0;
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    assert(env);
    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (activity != NULL)
    {
        jclass clazz = env->GetObjectClass(activity);
        if (clazz != NULL)
        {
            jmethodID method_id = env->GetMethodID(clazz, "getScreenSize",
                "()I");
            if (method_id != NULL)
                screen_size = env->CallIntMethod(activity, method_id);
                
            jmethodID display_dpi_id = env->GetStaticMethodID(clazz, 
                "getDisplayDPI", "()Landroid/util/DisplayMetrics;");
                
            if (display_dpi_id != NULL)
            {
                jobject display_dpi_obj = env->CallStaticObjectMethod(clazz, 
                                                                display_dpi_id);
                jclass display_dpi_class = env->GetObjectClass(display_dpi_obj);
            
                jfieldID ddpi_field = env->GetFieldID(display_dpi_class, 
                                                      "densityDpi", "I");
                ddpi = env->GetIntField(display_dpi_obj, ddpi_field);
            
                env->DeleteLocalRef(display_dpi_obj);
                env->DeleteLocalRef(display_dpi_class);
            }
            
            env->DeleteLocalRef(activity);
            env->DeleteLocalRef(clazz);
        }
    }

    // Set multitouch device scale depending on actual screen size
    switch (screen_size)
    {
    case SCREENLAYOUT_SIZE_SMALL:
    case SCREENLAYOUT_SIZE_NORMAL:
        UserConfigParams::m_multitouch_scale.setDefaultValue(1.3f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.1f);
        UserConfigParams::m_font_size = 5.0f;
        break;
    case SCREENLAYOUT_SIZE_LARGE:
        UserConfigParams::m_multitouch_scale.setDefaultValue(1.2f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.15f);
        UserConfigParams::m_font_size = 5.0f;
        break;
    case SCREENLAYOUT_SIZE_XLARGE:
        UserConfigParams::m_multitouch_scale.setDefaultValue(1.1f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.2f);
        UserConfigParams::m_font_size = 4.0f;
        break;
    default:
        break;
    }
    
    // Update rtts scale based on display DPI
    if (ddpi < 1)
    {
        Log::warn("MainAndroid", "Failed to get display DPI.");
        UserConfigParams::m_scale_rtts_factor = 0.7f;
    }
    else
    {
        if (ddpi > 400)
            UserConfigParams::m_scale_rtts_factor = 0.6f;
        else if (ddpi > 300)
            UserConfigParams::m_scale_rtts_factor = 0.65f;
        else if (ddpi > 200)
            UserConfigParams::m_scale_rtts_factor = 0.7f;
        else if (ddpi > 150)
            UserConfigParams::m_scale_rtts_factor = 0.75f;
        else
            UserConfigParams::m_scale_rtts_factor = 0.8f;

        Log::info("MainAndroid", "Display DPI: %i", ddpi);
        Log::info("MainAndroid", "Render scale: %f", 
                  (float)UserConfigParams::m_scale_rtts_factor);
    }
#endif

    // Enable screen keyboard
    UserConfigParams::m_screen_keyboard = 1;
    
    // It shouldn't matter, but STK is always run in fullscreen on android
    UserConfigParams::m_fullscreen = true;
    
    // Make sure that user can play every track even if there are installed
    // only few tracks and it's impossible to finish overworld challenges
    UserConfigParams::m_unlock_everything = 1;
    
    // Create default user istead of showing login screen to make life easier
    UserConfigParams::m_enforce_current_player = true;
}

#ifdef IOS_STK
void getConfigForDevice(const char* dev)
{
    // Check browser.geekbench.com/ios-benchmarks metal benchmark
    // https://gist.github.com/adamawolf/3048717 for device name mapping
    std::string device = dev;
    if (device.find("iPhone") != std::string::npos)
    {
        // Normal configuration default
        UserConfigParams::m_multitouch_scale.setDefaultValue(1.3f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.1f);
        UserConfigParams::m_font_size = 5.0f;
        device.erase(0, 6);
        auto versions = StringUtils::splitToUInt(device, ',');
        if (versions.size() == 2)
        {
            // A9 GPU
            if (versions[0] >= 8)
            {
                UserConfigParams::m_dynamic_lights = true;
                UserConfigParams::m_high_definition_textures = 1;
            }
            if (versions[0] < 7 || // iPhone 5s
                (versions[0] == 7 && versions[1] == 2) || // iPhone 6
                (versions[0] == 8 && versions[1] == 1) || // iPhone 6S
                (versions[0] == 8 && versions[1] == 4) || // iPhone SE
                (versions[0] == 9 && versions[1] == 1) || // iPhone 7
                (versions[0] == 9 && versions[1] == 3) || // iPhone 7
                (versions[0] == 10 && versions[1] == 1) || // iPhone 8
                (versions[0] == 10 && versions[1] == 4) // iPhone 8
                )
            {
                // Those phones have small screen
                UserConfigParams::m_multitouch_scale.setDefaultValue(1.45f);
            }
        }
    }
    else if (device.find("iPad") != std::string::npos)
    {
        // Normal configuration default
        UserConfigParams::m_multitouch_scale.setDefaultValue(0.95f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.2f);
        UserConfigParams::m_font_size = 3.0f;
        device.erase(0, 4);
        auto versions = StringUtils::splitToUInt(device, ',');
        if (versions.size() == 2)
        {
            if (versions[0] >= 7)
            {
                UserConfigParams::m_dynamic_lights = true;
                UserConfigParams::m_high_definition_textures = 1;
            }
        }
    }
    else if (device.find("iPod") != std::string::npos)
    {
        // All iPod touch has small screen
        UserConfigParams::m_font_size = 5.0f;
        UserConfigParams::m_multitouch_scale.setDefaultValue(1.45f);
        UserConfigParams::m_multitouch_sensitivity_x.setDefaultValue(0.1f);
        device.erase(0, 4);
        auto versions = StringUtils::splitToUInt(device, ',');
        if (versions.size() == 2)
        {
            // iPod Touch 7th Generation (A10)
            if (versions[0] >= 9)
            {
                UserConfigParams::m_dynamic_lights = true;
                UserConfigParams::m_high_definition_textures = 1;
            }
        }
    }
}

#endif

#endif
