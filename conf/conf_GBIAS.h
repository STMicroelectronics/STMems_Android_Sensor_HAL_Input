/*
 * Copyright (C) 2014 STMicroelectronics
 * Matteo Dameno, Ciocca Denis, Giuseppe Barba - Motion MEMS Product Div.
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


#ifndef CONFIGURATION_GBIAS_H
#define CONFIGURATION_GBIAS_H

#define GYROSCOPE_GBIAS_CALIBRATION (1 & GYROSCOPE_GBIAS_MODULE_PRESENT & SENSORS_GYROSCOPE_ENABLE)

/* GYROSCOPE BIAS ESTIMATION */
#define GYROSCOPE_GBIAS_ESTIMATION_FUSION (0 & GYROSCOPE_GBIAS_CALIBRATION & SENSOR_FUSION_ENABLE)
#define GYROSCOPE_GBIAS_ESTIMATION_STANDALONE (1 & GYROSCOPE_GBIAS_CALIBRATION & !GYROSCOPE_GBIAS_ESTIMATION_FUSION)

#endif /* CONFIGURATION_GBIAS_H */
