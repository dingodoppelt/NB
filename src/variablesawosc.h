#MIT License

#Copyright (c) 2020 Electrosmith, Corp. (https://github.com/electro-smith/DaisySP)

#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:

#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.

#include <stdint.h>

/**  
       @brief Variable Saw Oscillator. 
       @author Ben Sergentanis
       @date Dec 2020 
       Saw with variable slope or notch. \n \n
       Ported from pichenettes/eurorack/plaits/dsp/oscillator/variable_saw_oscillator.h \n 
       \n to an independent module. \n
       Original code written by Emilie Gillet in 2016. \n
*/
class VariableSawOscillator
{
  public:
    VariableSawOscillator() {}
    ~VariableSawOscillator() {}

    void Init(float sample_rate);

    /** Get the next sample */
    float Process();

    /** Set master freq.
        \param frequency Freq in Hz.
    */
    void SetFreq(float frequency);

    /** Adjust the wave depending on the shape
        \param pw Notch or slope. Works best -1 to 1.
    */
    void SetPW(float pw);

    /** Slope or notch
        \param waveshape 0 = notch, 1 = slope
    */
    void SetWaveshape(float waveshape);


  private:
    float ComputeNaiveSample(float phase,
                             float pw,
                             float slope_up,
                             float slope_down,
                             float triangle_amount,
                             float notch_amount);

    float sample_rate_;

    // Oscillator state.
    float phase_;
    float next_sample_;
    float previous_pw_;
    bool  high_;

    const float kVariableSawNotchDepth = 0.2f;

    // For interpolation of parameters.
    float frequency_;
    float pw_;
    float waveshape_;
};
