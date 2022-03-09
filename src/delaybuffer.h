/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2022  <copyright holder> <email>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#ifndef DELAYBUFFER_H
#define DELAYBUFFER_H

#include <vector>

/**
 * @todo write docs
 */
class DelayBuffer
{
public:
    DelayBuffer() {};
    void setSize(long unsigned int size) { delayBuffer.resize(size); };
    void setDelay(float delay) { outdex = (long unsigned int)(index - delay + delayBuffer.size()) % delayBuffer.size(); };
    void put(float sample) { delayBuffer[index] = sample; };
    float get() { return delayBuffer[outdex]; };
    void tick() { index++; outdex++; if(index >= delayBuffer.size()) index = 0; if(outdex >= delayBuffer.size()) outdex = 0; };
private:
    std::vector<float> delayBuffer;
    long unsigned int index = 0;
    long unsigned int outdex = 1;
};

#endif // DELAYBUFFER_H
