//
///
//// I wrote this. I think it's pretty neat  :)
///
//

uint8_t shrink_pixels();
static Color prev_lava_color = Color();
static Color prev_base_color = Color();
static uint16_t lava[256];
static uint8_t lavaFade = 0;
static uint8_t baseFade = 0;
static uint8_t fps; // This is really "frames per transition" but the default transition length is 1s anyway, so...
static uint8_t spawn = 0;
static uint8_t num_pixels;

void update_lava(AddressableLight &strip, Color lava_color) {
  // Set up the spawning "algorithm"
  float spawnrateF = id(spawnrate).state;
  uint8_t spawnrate = (fps - (spawnrateF * fps));
  if(!spawnrate) spawnrate = 1;
  
  // Set up the base color
  float br, bg, bb, bw;
  id(base).current_values_as_rgbw(&br, &bg, &bb, &bw);
  Color base_color = Color(int(br * 255.0), int(bg * 255.0), int(bb * 255.0), int(bw * 255.0));
  
  // fade colors when you change them in HA (instead of snapping immediately)
  if(lava_color != prev_lava_color) {
    lava_color = prev_lava_color.gradient(lava_color, ((lavaFade + 1) / (float)fps) * 255);
    prev_lava_color = lava_color;
    lavaFade++;
    lavaFade %= fps;
  }

  if(base_color != prev_base_color) {
    base_color = prev_base_color.gradient(base_color, ((baseFade + 1) / (float)fps) * 255);
    prev_base_color = base_color;
    baseFade++;
    baseFade %= fps;
  }

  // The strip is always the max 256, we're just only sending data to the set pixel_count
  uint8_t num_pixel_target = id(pixel_count).state;
  if(num_pixel_target < num_pixels) {
    // shrink
    uint8_t s = shrink_pixels();
    if(s < num_pixels - 1) {
      strip[num_pixels - 1] = Color();
      num_pixels--;
    }
  } else if(num_pixel_target > num_pixels) {
    // expand
    num_pixels++;
  }

  // Spawn a new lava...
  if(spawn == 0) lava[random(0, num_pixel_target)]++;
  spawn++;
  spawn %= spawnrate;

  // Step the lava
  for(uint8_t i = 0; i < num_pixels; i++) {
    uint16_t l = lava[i];
    l = (l + (l ? 1 : 0)) % 512;
    lava[i] = l;

    l = l <= 255 ? l : 511 - l;
    Color c = lava_color.gradient(base_color, 255 - l);
    strip[i] = c;
  }
}

// This just checks if there's still data at the tail end of the strip, when shrinking. Because.
uint8_t shrink_pixels() {
  for(uint8_t i = num_pixels - 1; i >= id(pixel_count).state; i--) {
    if(lava[i]) return i;
  }
  return 0;
}