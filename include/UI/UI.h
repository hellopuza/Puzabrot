#ifndef UI_UI_H
#define UI_UI_H

#include "UI/Button.h"
#include "UI/Label.h"
#include "UI/InputBox.h"
#include "UI/SwitchButton.h"

#include <memory>

class UI : public Vidget
{
public:
    UI();

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    bool handleEvent(const sf::Event& event) override;

    void addVidget(const std::string& name, Vidget* vidget);
    const Vidget* getVidget(const std::string& name) const;
    Vidget* getVidget(const std::string& name);

private:
    void update() override;

    std::unordered_map<std::string, std::unique_ptr<Vidget>> vidgets_;
};

#endif // UI_UI_H