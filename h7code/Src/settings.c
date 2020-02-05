#include "main.h"
#include "settings.h"
#include "hardware/store_to_spi_flash.h"

Settings g_settings = {
    .view_parallel = false,
    .all_frequencies = false,
    .view_mode = VM_LC,
    .single_freq_index = 18,
    .graph_min_freq = 100,
    .graph_max_freq = 10000,
    .graph_type = GRAPH_Z_RE_Z_IM,
    .correction_index = 0
};

char* g_correction_names[CORRECTION_COUNT] = {
    "LONG",
    "SHORT",
    "PCB"
};

static Settings settings_stored;
static uint32_t last_save_ms;

void LoadSettings()
{
    if(SpiFlashReadFromFlash(FLASH_SECTOR_SETTINGS, sizeof(Settings), &settings_stored))
    {
        if(settings_stored.correction_index<0 || settings_stored.correction_index>=CORRECTION_COUNT)
            settings_stored.correction_index = 0;

        g_settings = settings_stored;
    } else {
        settings_stored = g_settings;
    }

    last_save_ms = HAL_GetTick();
}

bool SaveSettingsIfChangedAndTimeUp()
{
    uint32_t dt = HAL_GetTick()-last_save_ms;
    if(dt < 10000)
        return false;
    if(memcmp(&g_settings, &settings_stored, sizeof(Settings))==0)
        return false;

    SpiFlashWriteToFlash(FLASH_SECTOR_SETTINGS, sizeof(Settings), &g_settings);

    settings_stored = g_settings;
    last_save_ms = HAL_GetTick();

    return true;
}
