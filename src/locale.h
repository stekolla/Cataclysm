#pragma once

/**
 * Initialize the process locale used for formatting and character handling.
 */
auto init_locale() -> bool;

/**
 * Reload English name data after the locale is initialized.
 */
auto load_english_names() -> void;
