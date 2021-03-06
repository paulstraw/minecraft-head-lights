// NeoPixelFunFadeInOut
// This example will randomly pick a color and fade all pixels to that color, then
// it will fade them to black and restart over
//
// This example demonstrates the use of a single animation channel to animate all
// the pixels at once.
//
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

const uint16_t PixelCount = 19;
const uint8_t PixelPin = 10;
const uint8_t AnimationChannels = 1; // we only need one as all the pixels are animated at once

// creeper = 114
// enderman = 300
const float hue = 300.0f / 360.0f;

NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
// For Esp8266, the Pin is omitted and it uses GPIO3 due to DMA hardware use.
// There are other Esp8266 alternative methods that provide more pin options, but also have
// other side effects.
// for details see wiki linked here https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods

NeoPixelAnimator animations(AnimationChannels); // NeoPixel animation management object

boolean fadeToColor = true; // general purpose variable used to store effect state

// what is stored for state is specific to the need, in this case, the colors.
// basically what ever you need inside the animation update function
struct MyAnimationState
{
  RgbwColor StartingColor;
  RgbwColor EndingColor;
};

// one entry per pixel to match the animation timing manager
MyAnimationState animationState[AnimationChannels];

void SetRandomSeed()
{
  uint32_t seed;

  // random works best with a seed that can use 31 bits
  // analogRead on a unconnected pin tends toward less than four bits
  seed = analogRead(0);
  delay(1);

  for (int shifts = 3; shifts < 31; shifts += 3)
  {
    seed ^= analogRead(0) << shifts;
    delay(1);
  }

  randomSeed(seed);
}

// simple blend function
void BlendAnimUpdate(const AnimationParam &param)
{
  // this gets called for each animation on every time step
  // progress will start at 0.0 and end at 1.0
  // we use the blend function on the RgbColor to mix
  // color based on the progress given to us in the animation
  RgbwColor updatedColor = RgbwColor::LinearBlend(
      animationState[param.index].StartingColor,
      animationState[param.index].EndingColor,
      param.progress);

  // apply the color to the strip
  for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
  {
    strip.SetPixelColor(pixel, updatedColor);
  }
}

void FadeInFadeOutRinseRepeat()
{
  if (fadeToColor)
  {
    // Fade upto a random color
    // we use HslColor object as it allows us to easily pick a hue
    // with the same saturation and luminance so the colors picked
    // will have similiar overall brightness
    float luminance = random(0, 50) / 100.0f;

    RgbColor target = HslColor(hue, 1.0f, luminance);
    uint16_t time = random(300, 1200);

    animationState[0].StartingColor = strip.GetPixelColor(0);
    animationState[0].EndingColor = target;

    animations.StartAnimation(0, time, BlendAnimUpdate);
  }
  else
  {
    // delay
    uint16_t time = random(100, 2500);

    animationState[0].StartingColor = strip.GetPixelColor(0);
    animationState[0].EndingColor = strip.GetPixelColor(0);

    animations.StartAnimation(0, time, BlendAnimUpdate);
  }

  // toggle to the next effect state
  fadeToColor = !fadeToColor;
}

void setup()
{
  strip.Begin();
  strip.Show();

  SetRandomSeed();
}

void loop()
{
  if (animations.IsAnimating())
  {
    animations.UpdateAnimations();
    strip.Show();
  }
  else
  {
    FadeInFadeOutRinseRepeat();
  }
}
