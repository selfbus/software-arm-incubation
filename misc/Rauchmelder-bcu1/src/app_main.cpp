/*
 *    _____ ________    __________  __  _______    ____  __  ___
 *   / ___// ____/ /   / ____/ __ )/ / / / ___/   / __ \/  |/  /
 *   \__ \/ __/ / /   / /_  / __  / / / /\__ \   / /_/ / /|_/ /
 *  ___/ / /___/ /___/ __/ / /_/ / /_/ /___/ /  / _, _/ /  / /
 * /____/_____/_____/_/   /_____/\____//____/  /_/ |_/_/  /_/
 *
 *  Copyright (c) 2013-2014 Stefan Taferner <stefan.taferner@gmx.at>
 *  Copyright (c) 2017 Oliver Stefan <o.stefan252@googlemail.com>
 *  Copyright (c) 2020 Stefan Haller
 *
 *  Refactoring and bug fixes:
 *  Copyright (c) 2023 Darthyson <darth@maptrack.de>
 *  Copyright (c) 2023 Thomas Dallmair <dev@thomas-dallmair.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */

#include "app_main.h"
#include "sd_app.h"

APP_VERSION("S_RM_H6 ", "3", "10") // Don't forget to also change the build-variable sw_version

SmokeDetectorApp *app = nullptr;

/**
 * Application setup
 */
BcuBase* setup()
{
    app = new SmokeDetectorApp();
    return app->initialize();
}

/**
 * The application's main.
 */
void loop()
{
    app->loop();
}

/**
 * The loop while no application is running.
 */
void loop_noapp()
{
    app->loopNoApp();
}

void SmokeDetectorUsrCallback::Notify(UsrCallbackType type)
{
    switch (type)
    {
        case UsrCallbackType::reset : // Reset after an ETS-application download or simple @ref APCI_BASIC_RESTART_PDU
            app->end();
            break;

        default :
            break;
    }
}
