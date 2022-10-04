#include <stdio.h>
#include "core-fusion.h"

#include <vector>
#include <utility>
#include <algorithm>

// Number of particles
#define NP 15

class ParticleFilter : public CfPipe<uint32_t, int16_t> {
 public:
  void run(CfStream<uint32_t>& istream, CfStream<int16_t>& ostream) {
    uint16_t idx;
    uint32_t inst;

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

    // Particle X position in 7.8 fixed point
    // Initial values uniformly distributed between 0.0 and 1.0
    int16_t particle_x[NP] = {16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240};
    int8_t particle_dir[NP] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
    uint8_t particle_W[NP] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

    int16_t save_x[NP];
    int8_t save_dir[NP];
    uint8_t resample[256];

    // Random number generator state
    uint16_t rng = 0x2437;

    // Read instructions from host
    while (istream.read(&inst)) {
      // High two bytes is command
      // Low two bytes is command argument

      uint16_t cmd = (inst>>16) & 0xffff;

      if (cmd == 0) {
        //////// Send current state

        for (idx = 0; idx < NP; idx++) {
          int16_t x = particle_x[idx];
          ostream.write(x);
          x = particle_W[idx];
          ostream.write(x);
        }

      } else if (cmd == 1) {
        //////// Move particles by command argument

        // Amount to move
        int16_t incr = inst & 0xffff;

        for (idx = 0; idx < NP; idx++) {
          int16_t off = incr; // requested amount

          //// randomize offset

          // Pseudo random number generator
          uint16_t bit = (rng >> 0) ^ (rng >> 2) ^ (rng >> 3) ^ (rng >> 5);
          rng  = (rng >> 1) | (bit << 15);

          // Add random offset to request
          off += ((int16_t)rng)>>12; // add signed number between 0.02734375 and -0.03125

          particle_x[idx] += off;
        }
      } else if (cmd == 2) {
        //////// Flip particles direction

        for (idx = 0; idx < NP; idx++)
          particle_dir[idx] = 1 - particle_dir[idx];

      } else if (cmd == 16) {
        // Receiver range observation

        // Measured range
        int16_t observed = inst & 0xffff;

        // Update particle weights
        for (idx = 0; idx < NP; idx++) {
          // Walls at 0 and 1

          // particle distance to wall in front of it
          int16_t belief;
          if (particle_dir[idx] == 0) {
            // Facing left, expect wall at 0
            belief = particle_x[idx];
          } else {
            // Facing right, expect wall at 1
            belief = 256 - particle_x[idx]; // 1.0 - x
          }

          uint8_t prob = 0;
          if (belief >= 0) {
            int16_t diff = observed - belief;
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
          uint16_t bit2 = (rng >> 0) ^ (rng >> 2) ^ (rng >> 3) ^ (rng >> 5);
          rng  = (rng >> 1) | (bit2 << 15);

          uint8_t selected = rng&0xff;
          pi = resample[selected];
          particle_x[idx] = save_x[pi];
          particle_dir[idx] = save_dir[pi];
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
    int16_t px, pW;
    pipeline.read(&px);
    pipeline.read(&pW);

    // print particle position and difference from initial position
    printf("%u %f %f\n", i, px/256.0, pW/256.0);
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

  // initial location 0.3554
  // observe distance measure of 0.3516
  inst = 0x0010005a;
  pipeline.write(inst);

  printf("OBSERVATION 1:\n");
  print_state(pipeline);

  // request resample
  inst = 0x00200000;
  pipeline.write(inst);

  printf("RESAMPLE:\n");
  print_state(pipeline);

  // send move -0.0625 instruction
  // actual move -0.0586
  inst = 0x0001fff0;
  pipeline.write(inst);

  // actual position 0.2969
  // observe distance measure of 0.2929
  inst = 0x0010004b;
  pipeline.write(inst);

  printf("OBSERVATION 2:\n");
  print_state(pipeline);

  print_distribution(pipeline);
}