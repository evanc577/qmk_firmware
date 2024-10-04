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
#include "backlight.h"
#include QMK_KEYBOARD_H

enum layers {
    _QWERTY,
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
    [_FL] = LAYOUT_ansi(
        QK_BOOT,  _______, _______, _______, _______,   KC_MSTP, KC_MPRV, KC_MPLY, KC_MNXT,   _______, KC_MUTE, KC_VOLD, KC_VOLU,    KC_SYRQ,    _______,
        KC_ESC,  KC_KP_1, KC_KP_2, KC_KP_3, KC_KP_4, KC_KP_5, KC_KP_6, KC_KP_7, KC_KP_8, KC_KP_9, KC_KP_0, KC_KP_MINUS, KC_KP_PLUS, KC_DEL,      KC_HOME,
        _______,     RGB_TOG, _______, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD, RGB_VAI, RGB_VAD, _______,   KC_UP, _______, _______,        KC_END,
        _______,       _______, _______, _______, _______, _______, KC_LEFT, KC_DOWN, KC_UP, KC_RGHT, KC_LEFT, KC_DOWN, KC_RGHT,
        _______,            _______, _______, _______,  _______,  KC_SPC,  KC_F13, KC_F14, KC_F15, KC_F16, KC_F17, KC_F18,              BL_UP,
        _______,   KC_RGUI,   _______,                       _______,                             _______,     KC_ALGR,        BL_TOGG, BL_DOWN, BL_STEP
    ),
};


// Lighting timeout feature
#ifdef IDLE_TIMEOUT_SEC
static uint16_t key_timer;
static uint16_t secs_to_idle = IDLE_TIMEOUT_SEC;
static uint8_t bl_level = 0;
static void refresh_timeout(void);
static void check_timeout(void);

void refresh_timeout() {
    key_timer = timer_read();
    if (secs_to_idle == 0) {
        rgblight_wakeup();
        backlight_set(bl_level);
        secs_to_idle = IDLE_TIMEOUT_SEC;
    }
}

void check_timeout() {
    if (secs_to_idle > 0 && timer_elapsed(key_timer) > 1000) {
        --secs_to_idle;
        key_timer = timer_read();
    }
    if (secs_to_idle == 0) {
        rgblight_suspend();
        bl_level = get_backlight_level();
        backlight_set(0);
    }
}
#endif //IDLE_TIMEOUT_SEC

void housekeeping_task_user() {
#ifdef IDLE_TIMEOUT_SEC
    check_timeout();
#endif //IDLE_TIMEOUT_SEC
}

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
#ifdef IDLE_TIMEOUT_SEC
    if (record->event.pressed) refresh_timeout();
#endif //IDLE_TIMEOUT_SEC
}

bool led_update_user(led_t led_state) {
    // Change underglow color around caps lock key if caps lock is active
    static uint8_t caps_state = 0;
    if (caps_state != led_state.caps_lock) {
        const uint8_t index = 13;
        const uint8_t count = 3;
        if (led_state.caps_lock) {
            rgblight_sethsv_range(
                CAPS_LOCK_RGBLIGHT_HSV,
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
