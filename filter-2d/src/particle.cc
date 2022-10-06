#include <stdio.h>
#include "core-fusion.h"

#include <vector>
#include <utility>
#include <algorithm>

// Number of particles
#define NP 25
#define NWALL 2

class ParticleFilter : public CfPipe<uint32_t, int16_t> {
 public:
  void run(CfStream<uint32_t>& istream, CfStream<int16_t>& ostream) {
    uint16_t idx, li;
    uint32_t inst;

    // 1.14 fixed point
    int16_t sine[256] = {
      0, 402, 803, 1205, 1605, 2005, 2404, 2801, 3196, 3589, 3980, 4369, 4756, 5139, 5519, 5896, 6269, 6639, 7005, 7366, 7723, 8075, 8423, 8765, 9102, 9434, 9759, 10079, 10393, 10701, 11002, 11297, 11585, 11866, 12139, 12406, 12665, 12916, 13159, 13395, 13622, 13842, 14053, 14255, 14449, 14634, 14810, 14978, 15136, 15286, 15426, 15557, 15678, 15790, 15892, 15985, 16069, 16142, 16206, 16260, 16305, 16339, 16364, 16379, 16384, 16379, 16364, 16339, 16305, 16260, 16206, 16142, 16069, 15985, 15892, 15790, 15678, 15557, 15426, 15286, 15136, 14978, 14810, 14634, 14449, 14255, 14053, 13842, 13622, 13395, 13159, 12916, 12665, 12406, 12139, 11866, 11585, 11297, 11002, 10701, 10393, 10079, 9759, 9434, 9102, 8765, 8423, 8075, 7723, 7366, 7005, 6639, 6269, 5896, 5519, 5139, 4756, 4369, 3980, 3589, 3196, 2801, 2404, 2005, 1605, 1205, 803, 402, 0, -403, -804, -1206, -1606, -2006, -2405, -2802, -3197, -3590, -3981, -4370, -4757, -5140, -5520, -5897, -6270, -6640, -7006, -7367, -7724, -8076, -8424, -8766, -9103, -9435, -9760, -10080, -10394, -10702, -11003, -11298, -11586, -11867, -12140, -12407, -12666, -12917, -13160, -13396, -13623, -13843, -14054, -14256, -14450, -14635, -14811, -14979, -15137, -15287, -15427, -15558, -15679, -15791, -15893, -15986, -16070, -16143, -16207, -16261, -16306, -16340, -16365, -16380, -16384, -16380, -16365, -16340, -16306, -16261, -16207, -16143, -16070, -15986, -15893, -15791, -15679, -15558, -15427, -15287, -15137, -14979, -14811, -14635, -14450, -14256, -14054, -13843, -13623, -13396, -13160, -12917, -12666, -12407, -12140, -11867, -11586, -11298, -11003, -10702, -10394, -10080, -9760, -9435, -9103, -8766, -8424, -8076, -7724, -7367, -7006, -6640, -6270, -5897, -5520, -5140, -4757, -4370, -3981, -3590, -3197, -2802, -2405, -2006, -1606, -1206, -804, -403
    };
    int16_t cosine[256] = {
      16384, 16379, 16364, 16339, 16305, 16260, 16206, 16142, 16069, 15985, 15892, 15790, 15678, 15557, 15426, 15286, 15136, 14978, 14810, 14634, 14449, 14255, 14053, 13842, 13622, 13395, 13159, 12916, 12665, 12406, 12139, 11866, 11585, 11297, 11002, 10701, 10393, 10079, 9759, 9434, 9102, 8765, 8423, 8075, 7723, 7366, 7005, 6639, 6269, 5896, 5519, 5139, 4756, 4369, 3980, 3589, 3196, 2801, 2404, 2005, 1605, 1205, 803, 402, 0, -403, -804, -1206, -1606, -2006, -2405, -2802, -3197, -3590, -3981, -4370, -4757, -5140, -5520, -5897, -6270, -6640, -7006, -7367, -7724, -8076, -8424, -8766, -9103, -9435, -9760, -10080, -10394, -10702, -11003, -11298, -11586, -11867, -12140, -12407, -12666, -12917, -13160, -13396, -13623, -13843, -14054, -14256, -14450, -14635, -14811, -14979, -15137, -15287, -15427, -15558, -15679, -15791, -15893, -15986, -16070, -16143, -16207, -16261, -16306, -16340, -16365, -16380, -16384, -16380, -16365, -16340, -16306, -16261, -16207, -16143, -16070, -15986, -15893, -15791, -15679, -15558, -15427, -15287, -15137, -14979, -14811, -14635, -14450, -14256, -14054, -13843, -13623, -13396, -13160, -12917, -12666, -12407, -12140, -11867, -11586, -11298, -11003, -10702, -10394, -10080, -9760, -9435, -9103, -8766, -8424, -8076, -7724, -7367, -7006, -6640, -6270, -5897, -5520, -5140, -4757, -4370, -3981, -3590, -3197, -2802, -2405, -2006, -1606, -1206, -804, -403, -1, 402, 803, 1205, 1605, 2005, 2404, 2801, 3196, 3589, 3980, 4369, 4756, 5139, 5519, 5896, 6269, 6639, 7005, 7366, 7723, 8075, 8423, 8765, 9102, 9434, 9759, 10079, 10393, 10701, 11002, 11297, 11585, 11866, 12139, 12406, 12665, 12916, 13159, 13395, 13622, 13842, 14053, 14255, 14449, 14634, 14810, 14978, 15136, 15286, 15426, 15557, 15678, 15790, 15892, 15985, 16069, 16142, 16206, 16260, 16305, 16339, 16364, 16379
    };

    // 0.8 fixed point
    uint8_t gaussian[256] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 6, 7, 8, 9, 10,
      11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 27, 29, 32, 35, 37, 40, 43, 47, 50, 54, 58, 62, 66, 70, 75, 79, 84, 89, 94, 100, 105, 111,
      116, 122, 128, 134, 140, 146, 152, 158, 164, 170, 176, 182, 187, 193, 199, 204, 209, 214, 219, 224, 228, 232, 236, 239, 242, 245, 248, 250, 251, 253, 254, 254,
      255, 254, 254, 253, 251, 250, 248, 245, 242, 239, 236, 232, 228, 224, 219, 214, 209, 204, 199, 193, 187, 182, 176, 170, 164, 158, 152, 146, 140, 134, 128, 122,
      116, 111, 105, 100, 94, 89, 84, 79, 75, 70, 66, 62, 58, 54, 50, 47, 43, 40, 37, 35, 32, 29, 27, 25, 23, 21, 19, 17, 16, 14, 13, 12,
      11, 10, 9, 8, 7, 6, 6, 5, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    // Walls 7.8 fixed point
    int16_t wall_x[NWALL] = {0, 0};
    int16_t wall_y[NWALL] = {0, 0};
    int16_t wall_tx[NWALL] = {0, 0x6400};
    int16_t wall_ty[NWALL] = {0x6400, 0};

    // Particle X, Y position in 7.8 fixed point
    int16_t particle_x[NP] = {
      0x1000, 0x2000, 0x3000, 0x4000, 0x5000,
      0x1000, 0x2000, 0x3000, 0x4000, 0x5000,
      0x1000, 0x2000, 0x3000, 0x4000, 0x5000,
      0x1000, 0x2000, 0x3000, 0x4000, 0x5000,
      0x1000, 0x2000, 0x3000, 0x4000, 0x5000
    };
    int16_t particle_y[NP] = {
      0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
      0x2000, 0x2000, 0x2000, 0x2000, 0x2000,
      0x3000, 0x3000, 0x3000, 0x3000, 0x3000,
      0x4000, 0x4000, 0x4000, 0x4000, 0x4000,
      0x5000, 0x5000, 0x5000, 0x5000, 0x5000,
    };
    // Particle direction 360/256 units
    uint8_t particle_dir[NP] = {
      190, 180, 170, 160, 150,
      140, 130, 190, 180, 170,
      160, 150, 140, 130, 190,
      180, 170, 160, 150, 140,
      130, 190, 180, 170, 160};
    uint8_t particle_W[NP] = {
      255, 255, 255, 255, 255,
      255, 255, 255, 255, 255,
      255, 255, 255, 255, 255,
      255, 255, 255, 255, 255,
      255, 255, 255, 255, 255
    };

    int16_t save_x[NP];
    int16_t save_y[NP];
    uint8_t save_dir[NP];
    uint8_t resample[256];

    // Random number generator state
    uint16_t rng = 0x2437;
    uint16_t rngbit;

    // Read instructions from host
    while (istream.read(&inst)) {
      // High two bytes is command
      // Low two bytes is command argument

      uint16_t cmd = (inst>>16) & 0xffff;

      if (cmd == 0) {
        //////// Send current state

        for (idx = 0; idx < NP; idx++) {
          int16_t x = particle_x[idx];
          int16_t y = particle_y[idx];
          ostream.write(x);
          ostream.write(y);
          int16_t ex;
          ex = particle_W[idx];
          ex = (ex<<8) | particle_dir[idx];
          ostream.write(ex);
        }

      } else if (cmd == 1) {
        //////// Move particles by command argument

        // Amount to move
        int16_t incr = inst & 0xffff;

        for (idx = 0; idx < NP; idx++) {
          int16_t off = incr; // requested amount

          //// randomize offset

          // Pseudo random number generator
          rngbit = (rng >> 0) ^ (rng >> 2) ^ (rng >> 3) ^ (rng >> 5);
          rng  = (rng >> 1) | (rngbit << 15);

          // Add random offset to request
          off += ((int16_t)rng)>>12; // add signed number between 0.02734375 and -0.03125

          particle_x[idx] += off;
        }
      } else if (cmd == 2) {
        //////// Rotate particles direction

        // Amount to move
        int16_t da = inst & 0xffff;

        for (idx = 0; idx < NP; idx++) {
          //// randomize offset
          int16_t angle = da;

          // Pseudo random number generator
          rngbit = (rng >> 0) ^ (rng >> 2) ^ (rng >> 3) ^ (rng >> 5);
          rng  = (rng >> 1) | (rngbit << 15);

          // Add random offset to request
          angle += ((int16_t)rng)>>14; // add signed number between -2 and 1
          angle += particle_dir[idx];

          particle_dir[idx] = angle;
        }

      } else if (cmd == 16) {
        // Receiver range observation

        // Measured range
        int16_t observed = inst & 0xffff;

        // Update particle weights
        for (idx = 0; idx < NP; idx++) {
          // Walls at 0 and 1
          int16_t x1 = particle_x[idx];
          int16_t y1 = particle_y[idx];
          uint8_t dir = particle_dir[idx];
          int16_t tx = cosine[dir] >> 6;
          int16_t ty = sine[dir] >> 6;
          int16_t x1_x2_diff = -tx;
          int16_t y1_y2_diff = -ty;

          // particle distance to wall in front of it
          int16_t belief=-1;

          for (li = 0; li < NWALL; li++) {
            int16_t x3 = wall_x[li];
            int16_t y3 = wall_y[li];
            int16_t ux = wall_tx[li];
            int16_t uy = wall_ty[li];
            int16_t x3_x4_diff = -ux;
            int16_t y3_y4_diff = -uy;

            int16_t x1_x3_diff = x1 - x3;
            int16_t y1_y3_diff = y1 - y3;

            // denominator, 0 ==> parallel
            int32_t den1 = (int32_t)x1_x2_diff * (int32_t)y3_y4_diff;
            int32_t den2 = (int32_t)y1_y2_diff * (int32_t)x3_x4_diff;
            int32_t den = den1 - den2;

            if (den != 0) {
              int32_t unum1 = (int32_t)x1_x3_diff * (int32_t)y1_y2_diff;
              int32_t unum2 = (int32_t)y1_y3_diff * (int32_t)x1_x2_diff;
              int32_t unum = unum1 - unum2;

              // hit segment if u between 0 and 1
              if ((den < 0 && (den <= unum && unum <= 0)) ||
                  (den > 0 && (0 <= unum && unum <= den))) {
                // t is distance to line
                int32_t tnum1 = (int32_t)x1_x3_diff * (int32_t)y3_y4_diff;
                int32_t tnum2 = (int32_t)y1_y3_diff * (int32_t)x3_x4_diff;
                int32_t tnum = tnum1 - tnum2;

                int32_t dist = tnum/(den>>8);
                belief = dist;
                //printf("P%d (%f,%f,%d) -> W%d @ %f\n", idx, x1/256.0, y1/256.0, 360*dir/256, li, dist/256.0);
              }
            }
          }

          uint8_t prob = 0;
          if (belief >= 0) {
            int16_t diff = (observed - belief) >> 6;
            if (-128 <= diff && diff < 127)
              prob = gaussian[128 + diff];
          }
          uint16_t W = (uint16_t)prob * (uint16_t)particle_W[idx];
          particle_W[idx] = (uint8_t)(W>>8);
        }
      } else if (cmd == 32) {
        //////// resample particles according to weight

        // save copy of particle state and compute total weight
        uint16_t sum = 0;
        for (idx = 0; idx < NP; idx++) {
          save_x[idx] = particle_x[idx];
          save_y[idx] = particle_y[idx];
          save_dir[idx] = particle_dir[idx];
          sum += particle_W[idx];
        }

        // construct distribution from weights
        uint8_t pi = 0;
        uint16_t acc = particle_W[0];
        uint32_t thresh = sum;

        for (idx = 0; idx < 256; idx++) {
          uint16_t u = (uint16_t)(thresh>>8);
          while (acc < u) {
            pi += 1;
            acc += particle_W[pi];
          }
          resample[idx] = pi;
          thresh += sum;
        }

        // sample NP particles from distribution
        for (idx = 0; idx < NP; idx++) {
          // uniform random number
          rngbit = (rng >> 0) ^ (rng >> 2) ^ (rng >> 3) ^ (rng >> 5);
          rng  = (rng >> 1) | (rngbit << 15);

          uint8_t selected = rng&0xff;
          pi = resample[selected];

          rngbit = (rng >> 0) ^ (rng >> 2) ^ (rng >> 3) ^ (rng >> 5);
          rng  = (rng >> 1) | (rngbit << 15);
          int16_t xoff = rng & 0xfe00; // use bits 15:9 (2.14 signed fixed point)
          int16_t yoff = (rng<<7) & 0xfe00; // use bits 8:2 (2.14 signed fixed point)
          int16_t doff = (rng<<14) & 0xc000; // use bits 1:0 (2.14 signed fixed point)

          particle_x[idx] = save_x[pi] + (xoff>>6);
          particle_y[idx] = save_y[pi] + (yoff>>6);
          particle_dir[idx] = save_dir[pi] + (doff>>14);
          particle_W[idx] = 255;
        }
      }
    }
  }
};

template <typename P>
void print_state(P &pipeline) {
  // send readout instruction
  uint32_t inst = 0x00000000;
  pipeline.write(inst);

  for (int i = 0; i < NP; i++) {
    int16_t px, py, ex;
    pipeline.read(&px);
    pipeline.read(&py);
    pipeline.read(&ex);
    uint8_t pa = (uint8_t)(ex&0xff);
    uint8_t pW = (uint8_t)((ex>>8)&0xff);
    // print particle position and difference from initial position
    printf("%u (%f,%f,%d) %f\n", i, px/256.0, py/256.0, 360*pa/256, pW/256.0);
  }
}

template <typename P>
void print_distribution(P &pipeline) {
  std::vector<std::pair<int16_t,int16_t>> particles;

  // send readout instruction
  uint32_t inst = 0x00000000;
  pipeline.write(inst);

  for (int i = 0; i < NP; i++) {
    int16_t px, pW;
    pipeline.read(&px);
    pipeline.read(&pW);
    particles.push_back(std::make_pair(px, pW));
  }

  std::sort(particles.begin(), particles.end());
  for (int i = 0; i < NP; i++) {
    printf("%f\t", particles[i].first/256.0);
    for (int j = (particles[i].second>>1); j > 0; j--)
      printf("*");
    printf("\n");
  }
}

int main(int argc, char *argv[]) {
  Upduino board;
  ParticleFilter filter;

  auto pipeline = board.serialIn >> filter >> board.serialOut;

  board.start(argc, argv);

  uint32_t inst;

  // initial location (18,50) 225deg
  // observe distance measure of 25
  inst = 0x00101900;
  pipeline.write(inst);

  printf("OBSERVATION 1:\n");
  print_state(pipeline);

  inst = 0x00200000;
  pipeline.write(inst);
  printf("RESAMPLE:\n");
  print_state(pipeline);

  // rotate to 180deg
  inst = 0x0002ffe0;
  pipeline.write(inst);

  // current location (18,50) 180deg
  // observe distance measure of 18
  inst = 0x00101200;
  pipeline.write(inst);

  printf("OBSERVATION 2:\n");
  print_state(pipeline);
}