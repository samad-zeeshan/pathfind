// UI component implementations. One font atlas, loaded from the embedded
// JetBrains Mono so every platform renders identical text.

#include "ui.h"

#include "theme.h"
#include "font_data.h"

namespace {

Font g_font = {};
bool g_fontLoaded = false;

// Loaded at 2x the largest render size, then scaled down with bilinear
// filtering. One atlas covers the whole type scale well enough at these sizes.
constexpr int kAtlasSize = 44;

// Mono glyphs are on a fixed advance, spacing 0 keeps columns aligned.
constexpr float kSpacing = 0.0f;

float roundness(Rectangle rec, int radiusPx) {
    const float side = rec.width < rec.height ? rec.width : rec.height;
    if (side <= 0) return 0.0f;
    float r = (float)radiusPx / (side * 0.5f);
    return r > 1.0f ? 1.0f : r;
}

}  // namespace

void initUi() {
    g_font = LoadFontFromMemory(".ttf", kUiFontData, (int)kUiFontDataSize,
                                kAtlasSize, nullptr, 0);
    g_fontLoaded = g_font.texture.id != 0;
    if (g_fontLoaded) {
#if defined(PLATFORM_WEB)
        // WebGL1 cannot mipmap the NPOT atlas, bilinear is the best available.
        SetTextureFilter(g_font.texture, TEXTURE_FILTER_BILINEAR);
#else
        // Mipmaps keep the small sizes crisp, the atlas is 3-4x larger than
        // the label text it gets scaled down to.
        GenTextureMipmaps(&g_font.texture);
        SetTextureFilter(g_font.texture, TEXTURE_FILTER_TRILINEAR);
#endif
    } else {
        g_font = GetFontDefault();
    }
}

void shutdownUi() {
    if (g_fontLoaded) UnloadFont(g_font);
    g_font = {};
    g_fontLoaded = false;
}

void uiText(const char* text, int x, int y, int size, Color color) {
    DrawTextEx(g_font, text, Vector2{ (float)x, (float)y }, (float)size, kSpacing, color);
}

int uiTextWidth(const char* text, int size) {
    return (int)MeasureTextEx(g_font, text, (float)size, kSpacing).x;
}

void uiTextRight(const char* text, int rightX, int y, int size, Color color) {
    uiText(text, rightX - uiTextWidth(text, size), y, size, color);
}

void uiSectionLabel(const char* text, int x, int y) {
    DrawTextEx(g_font, text, Vector2{ (float)x, (float)y },
               (float)theme::kFontLabel, 2.0f, theme::kTextMuted);
}

void uiCard(Rectangle rec, Color fill) {
    DrawRectangleRounded(rec, roundness(rec, theme::kRadius), 8, fill);
    DrawRectangleRoundedLinesEx(rec, roundness(rec, theme::kRadius), 8, 1.0f,
                                theme::kBorder);
}

int uiBadge(const char* text, int x, int y, Color dotColor) {
    const int h = 24;
    const int textW = uiTextWidth(text, theme::kFontSmall);
    const int w = 10 + 6 + 8 + textW + 12;
    const Rectangle rec{ (float)x, (float)y, (float)w, (float)h };
    DrawRectangleRounded(rec, 1.0f, 8, theme::kPanelInset);
    DrawRectangleRoundedLinesEx(rec, 1.0f, 8, 1.0f, theme::kBorder);
    DrawCircleV(Vector2{ (float)(x + 13), (float)(y + h / 2) }, 3.5f, dotColor);
    uiText(text, x + 24, y + (h - theme::kFontSmall) / 2, theme::kFontSmall, theme::kText);
    return w;
}

void uiStatRow(const char* label, const char* value, int x, int y, int width) {
    uiText(label, x, y, theme::kFontSmall, theme::kTextDim);
    uiTextRight(value, x + width, y - 1, theme::kFontBody, theme::kText);
}

void uiListRow(const char* key, const char* name, int x, int y, int width, bool active) {
    const int h = 26;
    if (active) {
        const Rectangle rec{ (float)x, (float)y, (float)width, (float)h };
        DrawRectangleRounded(rec, roundness(rec, 6), 8, theme::kPanelInset);
        DrawRectangle(x, y + 4, 3, h - 8, theme::kAccent);
    }
    const Color keyInk  = active ? theme::kAccent : theme::kTextMuted;
    const Color nameInk = active ? theme::kText   : theme::kTextDim;
    uiText(key, x + 12, y + (h - theme::kFontSmall) / 2, theme::kFontSmall, keyInk);
    uiText(name, x + 32, y + (h - theme::kFontBody) / 2, theme::kFontBody, nameInk);
}

int uiKeyHint(const char* key, const char* desc, int x, int y) {
    const int h = 20;
    const int keyW = uiTextWidth(key, theme::kFontLabel) + 12;
    const Rectangle rec{ (float)x, (float)y, (float)keyW, (float)h };
    DrawRectangleRounded(rec, roundness(rec, 5), 8, theme::kPanel);
    DrawRectangleRoundedLinesEx(rec, roundness(rec, 5), 8, 1.0f, theme::kBorder);
    uiText(key, x + 6, y + (h - theme::kFontLabel) / 2, theme::kFontLabel, theme::kTextDim);

    const int descX = x + keyW + 6;
    uiText(desc, descX, y + (h - theme::kFontSmall) / 2, theme::kFontSmall, theme::kTextMuted);
    return keyW + 6 + uiTextWidth(desc, theme::kFontSmall) + 18;
}

void uiLegendItem(const char* label, Color swatch, bool onFloor, int x, int y) {
    const Rectangle rec{ (float)x, (float)y, 12, 12 };
    if (onFloor) DrawRectangleRounded(rec, 0.35f, 6, theme::kFloor);
    DrawRectangleRounded(rec, 0.35f, 6, swatch);
    uiText(label, x + 19, y - 1, theme::kFontSmall, theme::kTextDim);
}
