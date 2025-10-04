#pragma once
#include "IPanel.hpp"

class StyleEditorPanel : public IPanel {
public:
    const char* Name() const override { return "StyleEditor"; }
    void Draw(PanelContext& ctx) override;
private:
    void SaveStyle(const char* filename);
    void LoadStyle(const char* filename);
};
