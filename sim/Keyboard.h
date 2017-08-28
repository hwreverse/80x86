#pragma once

#include <stdint.h>
#include "Simulator.h"

#include <SDL_events.h>

#include <map>

#include "SoftwareCPU.h"
#include "../bios/bda.h"

struct keydef {
    int normal;
    int shifted;
    int ctrl;
    int alt;
};

static std::map<int, struct keydef> scancode_map = {
    {SDLK_a, {0x1E61, 0x1E41, 0x1E01, 0x1E00}},
    {SDLK_b, {0x3062, 0x3042, 0x3002, 0x3000}},
    {SDLK_c, {0x2E63, 0x2E43, 0x2E03, 0x2E00}},
    {SDLK_d, {0x2064, 0x2044, 0x2004, 0x2000}},
    {SDLK_e, {0x1265, 0x1245, 0x1205, 0x1200}},
    {SDLK_f, {0x2166, 0x2146, 0x2106, 0x2100}},
    {SDLK_g, {0x2267, 0x2247, 0x2207, 0x2200}},
    {SDLK_h, {0x2368, 0x2348, 0x2308, 0x2300}},
    {SDLK_i, {0x1769, 0x1749, 0x1709, 0x1700}},
    {SDLK_j, {0x246A, 0x244A, 0x240A, 0x2400}},
    {SDLK_k, {0x256B, 0x254B, 0x250B, 0x2500}},
    {SDLK_l, {0x266C, 0x264C, 0x260C, 0x2600}},
    {SDLK_m, {0x326D, 0x324D, 0x320D, 0x3200}},
    {SDLK_n, {0x316E, 0x314E, 0x310E, 0x3100}},
    {SDLK_o, {0x186F, 0x184F, 0x180F, 0x1800}},
    {SDLK_p, {0x1970, 0x1950, 0x1910, 0x1900}},
    {SDLK_q, {0x1071, 0x1051, 0x1011, 0x1000}},
    {SDLK_r, {0x1372, 0x1352, 0x1312, 0x1300}},
    {SDLK_s, {0x1F73, 0x1F53, 0x1F13, 0x1F00}},
    {SDLK_t, {0x1474, 0x1454, 0x1414, 0x1400}},
    {SDLK_u, {0x1675, 0x1655, 0x1615, 0x1600}},
    {SDLK_v, {0x2F76, 0x2F56, 0x2F16, 0x2F00}},
    {SDLK_w, {0x1177, 0x1157, 0x1117, 0x1100}},
    {SDLK_x, {0x2D78, 0x2D58, 0x2D18, 0x2D00}},
    {SDLK_y, {0x1579, 0x1559, 0x1519, 0x1500}},
    {SDLK_z, {0x2C7A, 0x2C5A, 0x2C1A, 0x2C00}},
    {SDLK_1, {0x0231, 0x0221, 0x0231, 0x7800}},
    {SDLK_2, {0x0332, 0x0340, 0x0300, 0x7900}},
    {SDLK_3, {0x0433, 0x0423, 0x0433, 0x7A00}},
    {SDLK_4, {0x0534, 0x0524, 0x0534, 0x7B00}},
    {SDLK_5, {0x0635, 0x0625, 0x0635, 0x7C00}},
    {SDLK_6, {0x0736, 0x075E, 0x071E, 0x7D00}},
    {SDLK_7, {0x0837, 0x0826, 0x0837, 0x7E00}},
    {SDLK_8, {0x0938, 0x092A, 0x0938, 0x7F00}},
    {SDLK_9, {0x0A39, 0x0A28, 0x0a39, 0x8000}},
    {SDLK_0, {0x0B30, 0x0B29, 0x0b30, 0x8100}},
    {SDLK_MINUS, {0x0C2D, 0x0C5F, 0x0C1F, 0x8200}},
    {SDLK_EQUALS, {0x0D3D, 0x0D2B, 0x0d2b, 0x8300}},
    {SDLK_LEFTBRACKET, {0x1A5B, 0x1A7B, 0x1A1B, 0x1A00}},
    {SDLK_RIGHTBRACKET, {0x1B5D, 0x1B7D, 0x1B1D, 0x1B00}},
    {SDLK_SEMICOLON, {0x273B, 0x273A, 0x273b, 0x2700}},
    {SDLK_QUOTE, {0x2827, 0x2822, 0x2827, 0x2827}},
    {SDLK_BACKQUOTE, {0x2960, 0x297E, 0x2960, 0x2960}},
    {SDLK_BACKSLASH, {0x2B5C, 0x2B7C, 0x2B1C, 0x2600}},
    {SDLK_COMMA, {0x332C, 0x333C, 0x332c, 0x332c}},
    {SDLK_PERIOD, {0x342E, 0x343E, 0x342e, 0x342e}},
    {SDLK_SLASH, {0x352F, 0x353F, 0x352f, 0x352f}},
    {SDLK_F1, {0x3B00, 0x5400, 0x5E00, 0x6800}},
    {SDLK_F2, {0x3C00, 0x5500, 0x5F00, 0x6900}},
    {SDLK_F3, {0x3D00, 0x5600, 0x6000, 0x6A00}},
    {SDLK_F4, {0x3E00, 0x5700, 0x6100, 0x6B00}},
    {SDLK_F5, {0x3F00, 0x5800, 0x6200, 0x6C00}},
    {SDLK_F6, {0x4000, 0x5900, 0x6300, 0x6D00}},
    {SDLK_F7, {0x4100, 0x5A00, 0x6400, 0x6E00}},
    {SDLK_F8, {0x4200, 0x5B00, 0x6500, 0x6F00}},
    {SDLK_F9, {0x4300, 0x5C00, 0x6600, 0x7000}},
    {SDLK_F10, {0x4400, 0x5D00, 0x6700, 0x7100}},
    {SDLK_F11, {0x8500, 0x8700, 0x8900, 0x8B00}},
    {SDLK_F12, {0x8600, 0x8800, 0x8A00, 0x8C00}},
    {SDLK_BACKSPACE, {0x0E08, 0x0E08, 0x0E7F, 0x0E00}},
    {SDLK_DELETE, {0x5300, 0x532E, 0x9300, 0xA300}},
    {SDLK_DOWN, {0x5000, 0x5032, 0x9100, 0xA000}},
    {SDLK_END, {0x4F00, 0x4F31, 0x7500, 0x9F00}},
    {SDLK_RETURN, {0x1C0D, 0x1C0D, 0x1C0A, 0xA600}},
    {SDLK_ESCAPE, {0x011B, 0x011B, 0x011B, 0x0100}},
    {SDLK_HOME, {0x4700, 0x4737, 0x7700, 0x9700}},
    {SDLK_INSERT, {0x5200, 0x5230, 0x9200, 0xA200}},
    {SDLK_KP_5, {0x0000, 0x4C35, 0x8F00, 0x0000}},
    {SDLK_KP_MULTIPLY, {0x372A, 0x372a, 0x9600, 0x3700}},
    {SDLK_KP_MINUS, {0x4A2D, 0x4A2D, 0x8E00, 0x4A00}},
    {SDLK_KP_PLUS, {0x4E2B, 0x4E2B, 0x4e2b, 0x4E00}},
    {SDLK_KP_DIVIDE, {0x352F, 0x352F, 0x9500, 0xA400}},
    {SDLK_LEFT, {0x4B00, 0x4B34, 0x7300, 0x9B00}},
    {SDLK_PAGEDOWN, {0x5100, 0x5133, 0x7600, 0xA100}},
    {SDLK_PAGEUP, {0x4900, 0x4939, 0x8400, 0x9900}},
    {SDLK_PRINTSCREEN, {0x0000, 0x0000, 0x7200, 0x0000}},
    {SDLK_RIGHT, {0x4D00, 0x4D36, 0x7400, 0x9D00}},
    {SDLK_SPACE, {0x3920, 0x3920, 0x3920, 0x3920}},
    {SDLK_TAB, {0x0F09, 0x0F00, 0x9400, 0xA500}},
    {SDLK_UP, {0x4800, 0x4838, 0x8D00, 0x9800}},
};

template <typename CPU_t>
class Keyboard : public IOPorts {
public:
    Keyboard(CPU_t *cpu)
        : IOPorts(0x0060, 1),
        v(0),
	cpu(cpu)
    {}

    void write8(uint16_t __unused port_num, unsigned __unused offs,
                uint8_t __unused v)
    {
        cpu->clear_irq(1);
    }
    uint8_t read8(uint16_t __unused port_num, unsigned __unused offs)
    {
        cpu->clear_irq(1);

        return v;
    }

    void set_scancode(uint8_t v)
    {
        cpu->raise_irq(1);

        this->v = v;
    }

    void process_event(SDL_Event e)
    {
        if (bda_read(cpu, kbd_buffer[0]) != 0 || !scancode_map.count(e.key.keysym.sym))
            return;

        auto keydef = scancode_map[e.key.keysym.sym];

        auto modstate = SDL_GetModState();

        uint16_t v = keydef.normal;
        if (modstate & KMOD_SHIFT)
            v = keydef.shifted;
        else if (modstate & KMOD_CTRL)
            v = keydef.ctrl;
        else if (modstate & KMOD_ALT)
            v = keydef.alt;

        set_scancode(v);
        bda_write(cpu, kbd_buffer[0], v);
        bda_write(cpu, kbd_buffer_tail, 0x20);
    }
private:
    uint8_t v;
    CPU_t *cpu;
};
