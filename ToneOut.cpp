/*
  Joey Laybourn
  CS246A
  Assignment 1
  Fall 2019
*/

#include <portaudio.h>
#include <Control.h>
#include <algorithm>
#include <iostream>
#include <string>

#define RATE 44100
#define PI 3.1415926
#define BASE_FREQUENCY 220

class AudioControl : public Control
{
  public:
   AudioControl(unsigned n, const char *title) : 
    Control(n, title),
    index(0),
    indexIncrement(1),
    octave(0),
    cents(0),
    gain(1),
    frequency(BASE_FREQUENCY)
    {}
   void valueChanged(unsigned index, int value) override; 
   void setOctave(int oct) { octave = oct; }
   void setCents(int cent) { cents = cent; }
   void changePitch();

   int getIndex() { return index; }
   void setIndex(float i) { index = i; }
   void incrementIndex();

   float getFrequency() { return frequency; }

   void setGain(float volume);
   float getGain() { return gain; }
   
  private:
   double index;
   float indexIncrement;
   int octave;
   int cents;
   float frequency;
   float gain;
};

struct Test
{
  int index = 0;
};

void AudioControl::valueChanged(unsigned index, int value)
{
  std::string label;

  switch(index)
  {
    case 0:
      label = "Octave: ";
      setOctave(value);
      break;

    case 1:
      label = "Pitch Offset: ";
      setCents(value);
      break;

    case 2:
      label = "Volume: ";
      //std::cout << value << std::endl;
      setGain(value);
      break;
  }

  label += std::to_string(value);

  if(index == 2)
  {
    int decimal = std::abs(value % 10);
    label.resize(label.length() - 1);
    label += '.';
    label += std::to_string(decimal);
  }

  frequency = BASE_FREQUENCY * indexIncrement;
  setLabel(index, label.c_str());
  changePitch();
}

void AudioControl::changePitch()
{
  indexIncrement = (pow(2.0f, ((cents + (octave * 1200)) / 1200.0f)));
  //std::cout << frequency << std::endl;
}

void AudioControl::setGain(float volume)
{
  gain = pow(10, (volume / 10.0f) / 20.0f);
}

void AudioControl::incrementIndex() 
{ 
  index += indexIncrement; 

  // if (index >= RATE / (BASE_FREQUENCY * indexIncrement))
  // {
  //   index = 0;
  // }
}

/////////////////////////////////////////////////////////////////
// PortAudio callback
/////////////////////////////////////////////////////////////////
int onWrite(const void* vin, void* vout, unsigned long frames,
  const PaStreamCallbackTimeInfo* tinfo,
  PaStreamCallbackFlags flags, void* user)
{
  AudioControl* control = reinterpret_cast<AudioControl*>(user);
  float* out = reinterpret_cast<float*>(vout);
  const unsigned R = RATE;
  const float f = BASE_FREQUENCY;
  //int index = control->getIndex();
  //std::cout << "FRAMES: " << frames << std::endl;

  for (int i = 0; i < frames; ++i)
  {
    float sampleTemp;
    sampleTemp = control->getGain() * float((sin(2 * PI * f * control->getIndex() / R)));
    if(sampleTemp > 1)
    {
      sampleTemp = 1;
    }
    else if(sampleTemp < -1)
    {
      sampleTemp = -1;
    }
    
    out[i] = sampleTemp;
    control->incrementIndex();

  }

  //control->setIndex(index);
  return paContinue;
}

PaStream* initPortAudio(AudioControl *control)
{
  Pa_Initialize();
  PaStreamParameters params;
  params.device = Pa_GetDefaultOutputDevice();
  params.channelCount = 1;
  params.sampleFormat = paFloat32;
  params.suggestedLatency = max(0.02, Pa_GetDeviceInfo(params.device)
    ->defaultLowOutputLatency);
  params.hostApiSpecificStreamInfo = 0;
  PaStream* output_stream;
  Pa_OpenStream(&output_stream, 0, &params, RATE, 0, paClipOff, onWrite, control);
  Pa_StartStream(output_stream);

  return output_stream;
}

AudioControl* initControl()
{
  AudioControl *control = new AudioControl(3, "thing");

  control->setRange(0, -2, 2);
  control->setLabel(0, "Octave: 0");

  control->setRange(1, -1200, 1200);
  control->setLabel(1, "Pitch Offset: 0");

  control->setRange(2, -240, 240);
  control->setLabel(2, "Volume: 0.0");

  control->show(true);

  return control;
}

void closePortAudio(PaStream *output_stream)
{
  Pa_StopStream(output_stream);
  Pa_CloseStream(output_stream);
  Pa_Terminate();
}

int main()
{
  AudioControl* control = initControl();
  PaStream* output_stream = initPortAudio(control);

  while(1);
  
  closePortAudio(output_stream);
  return 0;
}