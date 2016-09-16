/*
 * Copyright (C) 2015 STMicroelectronics
 * Giuseppe Barba, Alberto Marinoni - Motion MEMS Product Div.
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef CONFIGURATION_MAGCAL_H
#define CONFIGURATION_MAGCAL_H

#define MAG_CALIBRATION_ENABLE	(1 & ST_MAG_CALIB_MODULE_PRESENT & SENSORS_MAGNETIC_FIELD_ENABLE)
#define CALIBRATION_FREQUENCY	(25)
#define CALIBRATION_PERIOD_MS	(1000.0f / CALIBRATION_FREQUENCY)

#endif /* CONFIGURATION_MAGCAL_H */
 
