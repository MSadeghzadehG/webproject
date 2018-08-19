/*  GamepadConfiguration.cpp
 *  PCSX2 Dev Team
 *  Copyright (C) 2015
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "GamepadConfiguration.h"

// Construtor of GamepadConfiguration
GamepadConfiguration::GamepadConfiguration(int pad, wxWindow *parent)
    : wxDialog(
          parent,                      // Parent
          wxID_ANY,                    // ID
          _T("Gamepad configuration"), // Title
          wxDefaultPosition,           // Position
          wxSize(400, 230),            // Width + Lenght
          // Style
          wxSYSTEM_MENU |
              wxCAPTION |
              wxCLOSE_BOX |
              wxCLIP_CHILDREN)
{

    m_pad_id = pad;
    m_pan_gamepad_config = new wxPanel(
        this,              // Parent
        wxID_ANY,          // ID
        wxDefaultPosition, // Prosition
        wxSize(300, 200)   // Size
        );
    m_cb_rumble = new wxCheckBox(
        m_pan_gamepad_config, // Parent
        wxID_ANY,             // ID
        _T("&Enable rumble"), // Label
        wxPoint(20, 20)       // Position
        );

    m_cb_hack_sixaxis_usb = new wxCheckBox(
        m_pan_gamepad_config,                    // Parent
        wxID_ANY,                                // ID
        _T("&Hack: Sixaxis/DS3 plugged in USB"), // Label
        wxPoint(20, 40)                          // Position
        );

    m_cb_hack_sixaxis_pressure = new wxCheckBox(
        m_pan_gamepad_config,              // Parent
        wxID_ANY,                          // ID
        _T("&Hack: Sixaxis/DS3 pressure"), // Label
        wxPoint(20, 60)                    // Position
        );

    wxString txt_rumble = wxT("Rumble intensity");
    m_lbl_rumble_intensity = new wxStaticText(
        m_pan_gamepad_config, // Parent
        wxID_ANY,             // ID
        txt_rumble,           // Text which must be displayed
        wxPoint(20, 90),      // Position
        wxDefaultSize         // Size
        );

    m_sl_rumble_intensity = new wxSlider(
        m_pan_gamepad_config, // Parent
        wxID_ANY,             // ID
        0,                    // value
        0,                    // min value 0x0000
        0x7FFF,               // max value 0x7FFF
        wxPoint(150, 83),     // Position
        wxSize(200, 30)       // Size
        );

    wxString txt_joystick = wxT("Joystick sensibility");
    m_lbl_rumble_intensity = new wxStaticText(
        m_pan_gamepad_config, // Parent
        wxID_ANY,             // ID
        txt_joystick,         // Text which must be displayed
        wxPoint(20, 120),     // Position
        wxDefaultSize         // Size
        );

    m_sl_joystick_sensibility = new wxSlider(
        m_pan_gamepad_config, // Parent
        wxID_ANY,             // ID
        0,                    // value
        0,                    // min value
        100,                  // max value
        wxPoint(150, 113),    // Position
        wxSize(200, 30)       // Size
        );

    m_bt_ok = new wxButton(
        m_pan_gamepad_config, // Parent
        wxID_ANY,             // ID
        _T("&OK"),            // Label
        wxPoint(250, 160),    // Position
        wxSize(60, 25)        // Size
        );

    m_bt_cancel = new wxButton(
        m_pan_gamepad_config, // Parent
        wxID_ANY,             // ID
        _T("&Cancel"),        // Label
        wxPoint(320, 160),    // Position
        wxSize(60, 25)        // Size
        );

    Bind(wxEVT_BUTTON, &GamepadConfiguration::OnButtonClicked, this);
    Bind(wxEVT_SCROLL_THUMBRELEASE, &GamepadConfiguration::OnSliderReleased, this);
    Bind(wxEVT_CHECKBOX, &GamepadConfiguration::OnCheckboxChange, this);

    repopulate();
}

/**
    Initialize the frame
    Check if a gamepad is detected
    Check if the gamepad support rumbles
*/
void GamepadConfiguration::InitGamepadConfiguration()
{
    repopulate(); // Set label and fit simulated key array
    /*
     * Check if there exist at least one pad available
     * if the pad id is 0, you need at least 1 gamepad connected,
     * if the pad id is 1, you need at least 2 gamepad connected,
     * Prevent to use a none initialized value on s_vgamePad (core dump)
    */
    if (s_vgamePad.size() >= m_pad_id + 1) {
        /*
         * Determine if the device can use rumble
         * Use TestForce with a very low strength (can't be felt)
         * May be better to create a new function in order to check only that
        */

        // Bad idea. Some connected devices might support rumble but not all connected devices.
        //        if (!s_vgamePad[m_pad_id]->TestForce(0.001f)) {
        //            wxMessageBox(L"Rumble is not available for your device.");
        //            m_cb_rumble->Disable();           // disable the rumble checkbox
        //            m_sl_rumble_intensity->Disable(); // disable the rumble intensity slider
        //        }
    } else {
        wxMessageBox(L"No gamepad detected.");
        m_sl_joystick_sensibility->Disable(); // disable the joystick sensibility slider
        m_cb_rumble->Disable();               // disable the rumble checkbox
        m_sl_rumble_intensity->Disable();     // disable the rumble intensity slider
    }
}

/****************************************/
/*********** Events functions ***********/
/****************************************/

/**
 * Button event, called when a button is clicked
*/
void GamepadConfiguration::OnButtonClicked(wxCommandEvent &event)
{
    // Affichage d'un message à chaque clic sur le bouton
    wxButton *bt_tmp = (wxButton *)event.GetEventObject(); // get the button object
    int bt_id = bt_tmp->GetId();                           // get the real ID
    if (bt_id == m_bt_ok->GetId()) {                       // If the button ID is equals to the Ok button ID
        Close();                                           // Close the window
    } else if (bt_id == m_bt_cancel->GetId()) {            // If the button ID is equals to the cancel button ID
        reset();                                           // reinitialize the value of each parameters
        Close();                                           // Close the window
    }
}

/**
 * Slider event, called when the use release the slider button
 * @FIXME The current solution can't change the joystick sensibility and the rumble intensity
 *        for a specific gamepad. The same value is used for both
*/
void GamepadConfiguration::OnSliderReleased(wxCommandEvent &event)
{
    wxSlider *sl_tmp = (wxSlider *)event.GetEventObject(); // get the slider object
    int sl_id = sl_tmp->GetId();                           // slider id
    if (sl_id == m_sl_rumble_intensity->GetId()) {         // if this is the rumble intensity slider
        u32 intensity = m_sl_rumble_intensity->GetValue(); // get the new value
        conf->set_ff_intensity(intensity);                 // and set the force feedback intensity value with it
                                                           // get the rumble intensity
        float strength = m_sl_rumble_intensity->GetValue();
        /*
        * convert in a float value between 0 and 1, and run rumble feedback
        * 1 -> 0x7FFF
        * 0 -> 0x0000
        * x -> ?
        *
        * formula : strength = x*1/0x7FFF
        * x : intensity variable
        * 0x7FFF : maximum intensity
        * 1 : maximum value of the intensity for the sdl rumble test
        */
        s_vgamePad[m_pad_id]->TestForce(strength / 0x7FFF);
    } else if (sl_id == m_sl_joystick_sensibility->GetId()) {
        u32 sensibility = m_sl_joystick_sensibility->GetValue(); // get the new value
        conf->set_sensibility(sensibility);                      // and set the joystick sensibility
    }
}

/**
 * Checkbox event, called when the value of the checkbox change
*/
void GamepadConfiguration::OnCheckboxChange(wxCommandEvent &event)
{
    wxCheckBox *cb_tmp = (wxCheckBox *)event.GetEventObject(); // get the slider object
    int cb_id = cb_tmp->GetId();
    if (cb_id == m_cb_rumble->GetId()) {
        conf->pad_options[m_pad_id].forcefeedback = (m_cb_rumble->GetValue()) ? (u32)1 : (u32)0;
        if (m_cb_rumble->GetValue()) {
            s_vgamePad[m_pad_id]->TestForce();
            m_sl_rumble_intensity->Enable();
        } else {
            m_sl_rumble_intensity->Disable();
        }
    } else if (cb_id == m_cb_hack_sixaxis_usb->GetId()) {
        conf->pad_options[m_pad_id].sixaxis_usb = (m_cb_hack_sixaxis_usb->GetValue()) ? (u32)1 : (u32)0;
    } else if (cb_id == m_cb_hack_sixaxis_pressure->GetId()) {
        conf->pad_options[m_pad_id].sixaxis_pressure = (m_cb_hack_sixaxis_pressure->GetValue()) ? (u32)1 : (u32)0;
    }
}

/****************************************/
/*********** Methods functions **********/
/****************************************/

// Reset checkbox and slider values
void GamepadConfiguration::reset()
{
    m_cb_rumble->SetValue(m_init_rumble);
    m_cb_hack_sixaxis_usb->SetValue(m_init_hack_sixaxis);
    m_cb_hack_sixaxis_pressure->SetValue(m_init_hack_sixaxis_pressure);
    m_sl_rumble_intensity->SetValue(m_init_rumble_intensity);
    m_sl_joystick_sensibility->SetValue(m_init_joystick_sensibility);
}

// Set button values
void GamepadConfiguration::repopulate()
{
    bool val = conf->pad_options[m_pad_id].forcefeedback;
    m_init_rumble = val;
    m_cb_rumble->SetValue(val);
    val = conf->pad_options[m_pad_id].sixaxis_usb;
    m_init_hack_sixaxis = val;
    m_cb_hack_sixaxis_usb->SetValue(val);
    val = conf->pad_options[m_pad_id].sixaxis_pressure;
    m_init_hack_sixaxis_pressure = val;
    m_cb_hack_sixaxis_pressure->SetValue(val);
    int tmp = conf->get_ff_intensity();
    m_sl_rumble_intensity->SetValue(tmp);
    m_init_rumble_intensity = tmp;
    tmp = conf->get_sensibility();
    m_sl_joystick_sensibility->SetValue(tmp);
    m_init_joystick_sensibility = tmp;

    // enable rumble intensity slider if the checkbox is checked
    if (m_cb_rumble->GetValue())
        m_sl_rumble_intensity->Enable();
    else // disable otherwise
        m_sl_rumble_intensity->Disable();
}
