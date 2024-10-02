/* Copyright 2021 Evan Chang <evanc577@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H

enum layers {
    _QWERTY,
#ifdef SPACEFN
    _SPACEFN,
#endif
    _FL
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_QWERTY] = LAYOUT_ansi(
        KC_ESC,    KC_F1,   KC_F2,   KC_F3,   KC_F4,     KC_F5,   KC_F6,   KC_F7,   KC_F8,     KC_F9,   KC_F10,  KC_F11,  KC_F12,     KC_PSCR,   KC_DEL,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,            KC_PGUP,
        KC_TAB,      KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,        KC_PGDN,
        KC_CAPS,       KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,
        KC_LSFT,            KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,
        KC_LCTL,   KC_LGUI,   KC_LALT,                       KC_SPC,                              MO(_FL),     KC_RCTL,        KC_LEFT, KC_DOWN, KC_RGHT
    ),
#ifdef SPACEFN
    [_SPACEFN] = LAYOUT_ansi(
        _______,   _______, _______, _______, _______,   _______, _______, _______, _______,   _______, _______, _______, _______,    _______,   _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,            _______,
        _______,     _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,        _______,
        _______,       _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______,            _______, _______, _______,  _______, _______,  _______, _______, _______, _______, _______, _______,       _______,
        _______,   _______,   _______,                   LT(_FL, KC_SPC),                        _______,     _______,        _______, _______,  _______
    ),
#endif // SPACEFN
    [_FL] = LAYOUT_ansi(
        QK_BOOT,  _______, _______, _______, _______,   KC_MSTP, KC_MPRV, KC_MPLY, KC_MNXT,   _______, KC_MUTE, KC_VOLD, KC_VOLU,    KC_SYRQ,   _______,
        KC_ESC,  KC_KP_1, KC_KP_2, KC_KP_3, KC_KP_4, KC_KP_5, KC_KP_6, KC_KP_7, KC_KP_8, KC_KP_9, KC_KP_0, KC_KP_MINUS, KC_KP_PLUS, KC_DEL,      KC_HOME,
        _______,     RGB_TOG, _______, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD, RGB_VAI, RGB_VAD, _______,   KC_UP, _______, _______,        KC_END,
        _______,       _______, _______, _______, _______, _______, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_LEFT, KC_DOWN, KC_RGHT,
        _______,            _______, _______, _______,  _______,  KC_SPC,  KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18,              BL_UP,
        _______,   KC_RGUI,
#ifdef SPACEFN
                            TG(_SPACEFN),
#else
                            _______,
#endif
                                                                  _______,                        _______,     KC_ALGR,        BL_TOGG, BL_DOWN, BL_STEP
    ),
};


// Lighting timeout feature
#ifdef IDLE_TIMEOUT_MINS
#if defined(BACKLIGHT_ENABLE) || defined(RGB_DI_PIN)
#define IDLE_TIMEOUT
#endif
#endif

#ifdef IDLE_TIMEOUT
static uint16_t idle_timer          = 0;
static uint8_t  halfmin_counter     = 0;
static uint8_t  led_level           = 0;
static bool     led_on              = true;
static bool     rgb_on              = true;
static bool     idle                = false;

void matrix_scan_user(void) {
    if (!idle && timer_elapsed(idle_timer) > 30000 && (led_on || rgb_on)) {
        halfmin_counter++;
        idle_timer = timer_read();
    }

    bool do_idle = false;
    if (halfmin_counter >= IDLE_TIMEOUT_MINS * 2) {
#ifdef BACKLIGHT_ENABLE
        if (led_on) {
            led_level       = get_backlight_level();
            do_idle         = true;
            backlight_set(0);
        }
#endif
#ifdef RGB_DI_PIN
        if (rgb_on) {
            do_idle         = true;
            rgblight_disable_noeeprom();
        }
#endif
        if (do_idle) {
            halfmin_counter = 0;
            idle            = true;
        }
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        // Restore lighting if coming out of idle
        if (idle) {
#ifdef BACKLIGHT_ENABLE
            if (led_on) {
                backlight_set(led_level);
            }
#endif
#if RGB_DI_PIN
            if (rgb_on) {
                rgblight_enable_noeeprom();
            }
#endif
            idle = false;
        }

        // Record current lighting status
#ifdef BACKLIGHT_ENABLE
        led_on = is_backlight_enabled();
#endif
#ifdef RGB_DI_PIN
        rgb_on = rgblight_is_enabled();
#endif

        // Reset timer
        idle_timer      = timer_read();
        halfmin_counter = 0;
    }
    return true;
}
#endif // IDLE_TIMEOUT


// SpaceFn status lighting
#ifdef SPACEFN
const rgblight_segment_t PROGMEM spacefn_layer[] = RGBLIGHT_LAYER_SEGMENTS(
        {11, 2, HSV_RED}
        );
const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(
        spacefn_layer
        );

layer_state_t layer_state_set_user(layer_state_t state) {
    rgblight_set_layer_state(0, layer_state_cmp(state, _SPACEFN));
    return state;
}
#endif // SPACEFN

bool led_update_user(led_t led_state) {
    static uint8_t caps_state = 0;
    if (caps_state != led_state.caps_lock) {
        const uint8_t index = 13;
        const uint8_t count = 3;
        if (led_state.caps_lock) {
            rgblight_sethsv_range(
                (CAPS_LOCK_RGBLIGHT_HSV >> 16) & 0xFF,
                (CAPS_LOCK_RGBLIGHT_HSV >>  8) & 0xFF,
                (CAPS_LOCK_RGBLIGHT_HSV >>  0) & 0xFF,
                index,
                index + count
            );
        } else {
            HSV hsv = rgblight_get_hsv();
            rgblight_sethsv_range(hsv.h, hsv.s, hsv.v, index, index + count);
        }
        caps_state = led_state.caps_lock;
    }
    return true;
}

void keyboard_post_init_user(void) {
#ifdef IDLE_TIMEOUT
    // Init backlight and rgb status
    led_on = is_backlight_enabled();
    rgb_on = rgblight_is_enabled();

    // idle_timer needs to be set one time
    if (idle_timer == 0) {
        idle_timer = timer_read();
    }
#endif

#ifdef SPACEFN
    rgblight_layers = my_rgb_layers;
#endif
}
