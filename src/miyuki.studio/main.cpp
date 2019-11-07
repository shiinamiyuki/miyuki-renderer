
#include <api/ui/ui.h>
#include <iostream>
#include <stdexcept>
int main() {
    using namespace miyuki;
    try {

        auto window = std::make_shared<ui::MainWindow>(800, 600, "miyuki.studio");
        auto docking = std::make_shared<ui::DockingSpace>("Docking Space");
        window->add(docking);
        docking->add(std::make_shared<ui::DemoWindow>());
        window->draw();
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}