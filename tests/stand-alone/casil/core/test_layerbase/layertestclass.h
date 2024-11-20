/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of Casil, a reimplementation of the data acquisition framework basil in C++.
//  Copyright (C) 2024 M. Frohne
//
//  Casil is free software: you can redistribute it and/or modify it
//  under the terms of the GNU Affero General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Casil is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty
//  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with Casil. If not, see <https://www.gnu.org/licenses/>.
//
//////////////////////////////////////////////////////////////////////////////////////////////////
*/

#ifndef CASILTESTS_LAYERTESTCLASS_H
#define CASILTESTS_LAYERTESTCLASS_H

#include <casil/layerbase.h>

#include <casil/layerconfig.h>

class LayerTestClass final : public casil::LayerBase
{
public:
    LayerTestClass(const casil::LayerConfig& pConfig, const casil::LayerConfig& pRequiredConfig,
                   bool pFailInit = false, bool pFailClose = false);
    ~LayerTestClass() override = default;

private:
    bool initImpl() override;
    bool closeImpl() override;

private:
    const bool failInit;
    const bool failClose;
};

#endif // CASILTESTS_LAYERTESTCLASS_H
