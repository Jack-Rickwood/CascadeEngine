#pragma once

#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "graphics/window/window.h"
#include "graphics/device/device.h"
#include "graphics/renderer/renderer.h"
#include "input/keyboard_input_controller.h"
#include "files/state_file.h"
#include "log/log.h"

#define CONFIG_GUI

namespace cscd {

class Application {
public:
#ifdef CONFIG_GUI
    static constexpr bool create_debug_window = true;
#else
    static constexpr bool create_debug_window = false;
#endif

    static constexpr int GAME_WIDTH = 1920;
    static constexpr int GAME_HEIGHT = 1200;
    static constexpr int CONFIG_WIDTH = 750;
    static constexpr int CONFIG_HEIGHT = 1000;

    Application() = delete;
    Application(std::string state_path);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void run();

    float mapSliderToPhi(int slider_val);
    int mapPhiToSlider(float phi_val);
    void configWindow();

    // GUI Callbacks
    void phiUpdate(tgui::Slider::Ptr& slider, tgui::Label::Ptr& slider_number, float& phi_setting);
    void sliderUpdate(tgui::Slider::Ptr& slider, tgui::Label::Ptr& slider_number, int& slider_setting);
    void enableFeatureUpdate(tgui::CheckBox::Ptr& checkbox, int& enable_setting);
    void iterationsUpdate(tgui::ComboBox::Ptr& combobox, int& iterations_setting);
    void numberBoxUpdate(tgui::EditBox::Ptr& editbox, int& number_setting);

private:
    SceneInfo scene_info{};
    
    Window window{GAME_WIDTH, GAME_HEIGHT, "Hello Vulkan!"};
    Device device{window};
    Renderer renderer;
    Logger logger;

    std::atomic_bool stop_config_ui = false;
};

}