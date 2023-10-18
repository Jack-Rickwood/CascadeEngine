#include "application.h"

namespace cscd {

// Maps:
// 0-10  | 0.00-0.10
// 11-20 | 0.10-1.00
// 21-30 | 1.00-10.0
float Application::mapSliderToPhi(int slider_val) {
    if (slider_val >= 0 && slider_val <= 10) {
        return 0.001f * (float)slider_val;
    } else if (slider_val >= 11 && slider_val <= 20) {
        return 0.01f * (float)(slider_val - 10);
    } else if (slider_val >= 21 && slider_val <= 30) {
        return 0.1f * (float)(slider_val - 20);
    } else {
        logger.log(Logger::LOG_LEVEL_ERROR, "Slider value outside domain passed to mapSliderToPhi");
        return 0.0f;
    }
}

int Application::mapPhiToSlider(float phi_val) {
    if (phi_val >= 0.00f && phi_val <= 0.1f) {
        return 1000 * (int)phi_val;
    } else if (phi_val >= 0.10f && phi_val <= 1.00f) {
        return 10 + 100 * (int)phi_val;
    } else if (phi_val >= 1.0f && phi_val <= 10.0f) {
        return 20 + 10 * (float)phi_val;
    } else {
        logger.log(Logger::LOG_LEVEL_ERROR, "Phi value outside domain passed to mapPhiToSlider");
        return 0;
    }
}

void Application::phiUpdate(tgui::Slider::Ptr& slider, tgui::Label::Ptr& slider_number, float& phi_setting) {
    float phi_val = mapSliderToPhi(slider->getValue());
    slider_number->setText(tgui::String::fromNumberRounded(phi_val, 3));
    phi_setting = phi_val;
    renderer.invalidateAccumulatedFrames();
}

void Application::sliderUpdate(tgui::Slider::Ptr& slider, tgui::Label::Ptr& slider_number, int& slider_setting) {
    slider_number->setText(tgui::String::fromNumber(slider->getValue()));
    slider_setting = slider->getValue();
    renderer.invalidateAccumulatedFrames();
}

void Application::enableFeatureUpdate(tgui::CheckBox::Ptr& checkbox, int& enable_setting) {
    enable_setting = checkbox->isChecked();
    renderer.invalidateAccumulatedFrames();
}

void Application::iterationsUpdate(tgui::ComboBox::Ptr& combobox, int& iterations_setting) {
    iterations_setting = combobox->getSelectedItem().toInt();
    renderer.invalidateAccumulatedFrames();
}

void Application::numberBoxUpdate(tgui::EditBox::Ptr& editbox, int& number_setting) {
    number_setting = editbox->getText().toInt();
    renderer.invalidateAccumulatedFrames();
}

void Application::configWindow() {
    sf::RenderWindow config_window(sf::VideoMode(800, 600), "SFML works!");
    config_window.setPosition(sf::Vector2i(1000, 200));
    tgui::Gui config_gui(config_window);
    tgui::Theme::setDefault("/usr/share/tgui-1/themes/Black.txt");
    
    config_gui.loadWidgetsFromFile("../src/gui/config/form.txt");

    tgui::ComboBox::Ptr ray_steps_box = config_gui.get<tgui::ComboBox>("maxRayStepsComboBox");
    int ray_steps_init = renderer.getRaytraceSettings().max_ray_steps;
    ray_steps_box->setSelectedItem(tgui::String::fromNumber(ray_steps_init));
    ray_steps_box->onItemSelect([&] { iterationsUpdate(std::ref(ray_steps_box), std::ref(renderer.getRaytraceSettings().max_ray_steps)); });

    tgui::Slider::Ptr ray_bounces_slider = config_gui.get<tgui::Slider>("maxRayBouncesSlider");
    tgui::Label::Ptr ray_bounces_number = config_gui.get<tgui::Label>("maxRayBouncesNumber");
    float ray_bounces_init = renderer.getRaytraceSettings().max_bounces;
    ray_bounces_slider->setValue(ray_bounces_init);
    ray_bounces_slider->onValueChange([&] { sliderUpdate(std::ref(ray_bounces_slider), std::ref(ray_bounces_number), std::ref(renderer.getRaytraceSettings().max_bounces)); });
    ray_bounces_number->setText(tgui::String::fromNumber(ray_bounces_init));

    tgui::Slider::Ptr rpp_slider = config_gui.get<tgui::Slider>("raysPerPixelSlider");
    tgui::Label::Ptr rpp_number = config_gui.get<tgui::Label>("raysPerPixelNumber");
    float rpp_init = renderer.getRaytraceSettings().rays_per_pixel;
    rpp_slider->setValue(rpp_init);
    rpp_slider->onValueChange([&] { sliderUpdate(std::ref(rpp_slider), std::ref(rpp_number), std::ref(renderer.getRaytraceSettings().rays_per_pixel)); });
    rpp_number->setText(tgui::String::fromNumber(rpp_init));

    tgui::CheckBox::Ptr blue_noise_box = config_gui.get<tgui::CheckBox>("blueNoiseCheckBox");
    bool blue_noise_init = renderer.getRaytraceSettings().use_blue_noise;
    blue_noise_box->setChecked(blue_noise_init);
    blue_noise_box->onChange([&] { enableFeatureUpdate(std::ref(blue_noise_box), std::ref(renderer.getRaytraceSettings().use_blue_noise)); });

    tgui::CheckBox::Ptr temp_accum_box = config_gui.get<tgui::CheckBox>("tempAccumCheckBox");
    bool temp_accum_init = renderer.getRaytraceSettings().use_temp_accumulation;
    temp_accum_box->setChecked(temp_accum_init);
    temp_accum_box->onChange([&] { enableFeatureUpdate(std::ref(temp_accum_box), std::ref(renderer.getRaytraceSettings().use_temp_accumulation)); });

    tgui::Slider::Ptr c_phi_slider = config_gui.get<tgui::Slider>("cPhiSlider");
    tgui::Label::Ptr c_phi_number = config_gui.get<tgui::Label>("cPhiNumber");
    float c_phi_init = renderer.getPostprocessSettings().c_phi;
    c_phi_slider->setValue(mapPhiToSlider(c_phi_init));
    c_phi_slider->onValueChange([&] { phiUpdate(std::ref(c_phi_slider), std::ref(c_phi_number), std::ref(renderer.getPostprocessSettings().c_phi)); });
    c_phi_number->setText(tgui::String::fromNumberRounded(c_phi_init, 3));

    tgui::Slider::Ptr n_phi_slider = config_gui.get<tgui::Slider>("nPhiSlider");
    tgui::Label::Ptr n_phi_number = config_gui.get<tgui::Label>("nPhiNumber");
    float n_phi_init = renderer.getPostprocessSettings().n_phi;
    n_phi_slider->setValue(mapPhiToSlider(n_phi_init));
    n_phi_slider->onValueChange([&] { phiUpdate(std::ref(n_phi_slider), std::ref(n_phi_number), std::ref(renderer.getPostprocessSettings().n_phi)); });
    n_phi_number->setText(tgui::String::fromNumberRounded(n_phi_init, 3));

    tgui::Slider::Ptr p_phi_slider = config_gui.get<tgui::Slider>("pPhiSlider");
    tgui::Label::Ptr p_phi_number = config_gui.get<tgui::Label>("pPhiNumber");
    float p_phi_init = renderer.getPostprocessSettings().p_phi;
    p_phi_slider->setValue(mapPhiToSlider(p_phi_init));
    p_phi_slider->onValueChange([&] { phiUpdate(std::ref(p_phi_slider), std::ref(p_phi_number), std::ref(renderer.getPostprocessSettings().p_phi)); });
    p_phi_number->setText(tgui::String::fromNumberRounded(p_phi_init, 3));

    tgui::CheckBox::Ptr atrous_enable_box = config_gui.get<tgui::CheckBox>("atrousEnableCheckBox");
    bool atrous_enable_init = renderer.getPostprocessSettings().use_atrous_denoise;
    atrous_enable_box->setChecked(atrous_enable_init);
    atrous_enable_box->onChange([&] { enableFeatureUpdate(std::ref(atrous_enable_box), std::ref(renderer.getPostprocessSettings().use_atrous_denoise)); });

    tgui::ComboBox::Ptr atrous_iterations_box = config_gui.get<tgui::ComboBox>("atrousIterationsComboBox");
    int atrous_iterations_init = renderer.getRendererSettings().denoise_iterations;
    atrous_iterations_box->setSelectedItem(tgui::String::fromNumber(atrous_iterations_init));
    atrous_iterations_box->onItemSelect([&] { iterationsUpdate(std::ref(atrous_iterations_box), std::ref(renderer.getRendererSettings().denoise_iterations)); });

    while (config_window.isOpen()){
        sf::Event event;
        while (config_window.pollEvent(event)) {
            config_gui.handleEvent(event);
            if (event.type == sf::Event::Closed) {
                config_window.close();
            }
        }
        if (stop_config_ui) {
            config_window.close();
        }

        config_window.clear();
        config_gui.draw();
        config_window.display();
    }
}

}