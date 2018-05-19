//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#include "challenges/story_mode_timer.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "states_screens/dialogs/speedrun_mode_dialog.hpp"
#include "utils/string_utils.hpp"

StoryModeTimer *story_mode_timer = 0;

StoryModeTimer::StoryModeTimer()
{
    m_player_tested           = false;
    reset();
}  // SpeedrunTimer

void StoryModeTimer::reset()
{
    PlayerProfile *player = PlayerManager::getCurrentPlayer();

    // May happen when there is no player profile at all
    if (player == NULL)
    {
        m_valid_speedrun_ended    = false;
        m_story_mode_ended        = false;
        m_stored_speedrun_milliseconds   = 0;
        m_stored_story_mode_milliseconds = 0;
    }
    else
    {
        m_valid_speedrun_ended    = player->isSpeedrunFinished();
        m_story_mode_ended        = player->isFinished();
        m_stored_speedrun_milliseconds   = player->getSpeedrunTimer();
        m_stored_story_mode_milliseconds = player->getStoryModeTimer();
    }

    m_valid_speedrun_started  = false;
    m_story_mode_started      = false;
    m_speedrun_pause_active   = false;
    m_story_mode_pause_active = false;
    m_loading_pause           = false;
    m_player_can_speedrun     = false;
    m_speedrun_milliseconds   = 0;
    m_story_mode_milliseconds = 0;
    std::chrono::time_point<std::chrono::system_clock> now(std::chrono::system_clock::now());
    m_speedrun_total_pause_time = now-now;//the zero() function doesn't work
    m_story_mode_total_pause_time = now-now;
}

void StoryModeTimer::startTimer()
{
    // The speedrun timer runs even if not enabled, as long
    // as the conditions match, as the ovrehead is minimal
    // and it thus persist if the user disable/reenable it.
	if (!m_valid_speedrun_started && m_player_can_speedrun)
    {
		m_speedrun_start = std::chrono::system_clock::now();
	    m_valid_speedrun_started = true;
    }

    // The normal story mode timer runs along the speedrun timer
    // so that if speedrun mode is disabled, its value exists
    // and is correct
    if (!m_story_mode_started)
    {
		m_story_mode_start = std::chrono::system_clock::now();
	    m_story_mode_started = true;
    }
}

void StoryModeTimer::stopTimer()
{
	if (m_valid_speedrun_started)
    {
		m_speedrun_end = std::chrono::system_clock::now();
		m_valid_speedrun_ended = true;
	}

    if (m_story_mode_started)
    {
        m_story_mode_end = std::chrono::system_clock::now();
        m_story_mode_ended = true;
    }
    updateTimer();
}

/* Pauses the story mode and speedrun timer, if applicable.
 * The speedrun timer is only paused on loading screens,
 * while the story mode timer can also be paused when
 * quitting the story mode. */
void StoryModeTimer::pauseTimer(bool loading)
{
    // Don't change the pause time if there is no run,
    // if it is finished, or if it is already set.

	if ( m_valid_speedrun_started && !m_speedrun_pause_active &&
         !m_valid_speedrun_ended && loading)
    {
        pauseSpeedrunTimer();
    }
    if ( m_story_mode_started && !m_story_mode_pause_active &&
         !m_story_mode_ended)
    {
        pauseStoryModeTimer();
        m_loading_pause = loading;
    }
} // pauseTimer

void StoryModeTimer::pauseSpeedrunTimer()
{
    m_speedrun_pause_start = std::chrono::system_clock::now();
    m_speedrun_pause_active = true;
}

void StoryModeTimer::pauseStoryModeTimer()
{
    m_story_mode_pause_start = std::chrono::system_clock::now();
    m_story_mode_pause_active = true;
}


void StoryModeTimer::unpauseTimer(bool loading)
{
    //Don't unpause if there is no run or no previous pause
	if (m_valid_speedrun_started && m_speedrun_pause_active && !m_valid_speedrun_ended && loading)
    {
        unpauseSpeedrunTimer();
    }
	if (m_story_mode_started && m_story_mode_pause_active &&
        !m_story_mode_ended && (m_loading_pause || ( !m_loading_pause && !loading)))
    {
        unpauseStoryModeTimer();
    }
} //unpauseTimer

void StoryModeTimer::unpauseSpeedrunTimer()
{
    std::chrono::time_point<std::chrono::system_clock> now(std::chrono::system_clock::now());
    m_speedrun_total_pause_time += now - m_speedrun_pause_start;
    m_speedrun_pause_active = false;

    int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(m_speedrun_total_pause_time).count();
    Log::verbose("StoryModeTimer", "Total speedrun pause time : %ims.",milliseconds);
} // unpauseSpeedrunTimer

void StoryModeTimer::unpauseStoryModeTimer()
{
    std::chrono::time_point<std::chrono::system_clock> now(std::chrono::system_clock::now());
    m_story_mode_total_pause_time += now - m_story_mode_pause_start;
    m_story_mode_pause_active = false;
} // unpauseStoryModeTimer

void StoryModeTimer::updateTimer()
{
    if (!m_player_tested)
    {
        reset();
        testPlayerRun();
    }

    updateSpeedrunTimer();
    updateStoryModeTimer();
} // updateTimer

void StoryModeTimer::updateSpeedrunTimer()
{
	std::chrono::duration<double> elapsed_time;

	if (m_valid_speedrun_ended)
    {
		elapsed_time = m_speedrun_end - m_speedrun_start - m_speedrun_total_pause_time;
    }
	else
    {
		std::chrono::time_point<std::chrono::system_clock> now(std::chrono::system_clock::now());
		elapsed_time = now - m_speedrun_start - m_speedrun_total_pause_time;
	}

	m_speedrun_milliseconds = m_stored_speedrun_milliseconds +
                              std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count();
}

void StoryModeTimer::updateStoryModeTimer()
{
	std::chrono::duration<double> elapsed_time;

	if (m_story_mode_ended)
    {
		elapsed_time = m_story_mode_end - m_story_mode_start - m_story_mode_total_pause_time;
    }
	else
    {
		std::chrono::time_point<std::chrono::system_clock> now(std::chrono::system_clock::now());
		elapsed_time = now - m_story_mode_start - m_story_mode_total_pause_time;
	}
    m_story_mode_milliseconds = m_stored_story_mode_milliseconds +
                                std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count();
}

//Check if the current player has already entered story mode or not
void StoryModeTimer::testPlayerRun()
{
    PlayerProfile *player = PlayerManager::getCurrentPlayer();

    // May happen when there is no player profile at all
    // Will be called again until a player profile is created
    if (player == NULL)
        return;

    if (player->isFirstTime())
    {
        m_player_can_speedrun = true;
    }


    if(!m_player_can_speedrun && UserConfigParams::m_speedrun_mode)
    {
        UserConfigParams::m_speedrun_mode = false;
        new SpeedrunModeDialog();
    }

    m_player_tested = true;
}

/* When the active player changes, resets and reload timer data */
void StoryModeTimer::playerHasChanged()
{
    m_player_tested = false;
} // playerHasChanged

std::string StoryModeTimer::getTimerString()
{
	if (UserConfigParams::m_speedrun_mode && !m_valid_speedrun_started && !m_valid_speedrun_ended)
    {
        return StringUtils::timeToString(/*time in seconds*/ 0,
                                         /*precision*/ 3, true, /* display hours*/ true);
    }

    if (UserConfigParams::m_speedrun_mode)
        return StringUtils::timeToString(/*time in seconds*/ m_speedrun_milliseconds/1000.0f,
                                         /*precision*/ 3, true, /* display hours*/ true);
    else
        return StringUtils::timeToString(/*time in seconds*/ m_story_mode_milliseconds/1000.0f,
                                         /*precision*/ 0, true, /* display hours*/ true);
} // getStoryModeTimerString

/* EOF */
