This project is no longer being worked on. See https://github.com/Nanobeef/unified


# CPU Boid Simulation

## Video
This video is too long! Please skim and skip around.
https://www.youtube.com/watch?v=yh0ttcGheHg

## About

The goal of this project is to simulate as many boid particles as possible in real-time. All of the code is written from scratch at a very low-level. A few libraries are used (FreeType2, SIMD-E).

A specific milestone is to simulate a time step with 1 Million particles in under 7 milliseconds (1/144fps), and to do so with reasonable search ranges.
To break that down:
- (0.007 seconds) / (1,000,000 particles) = Average 7 nanoseconds per particle per time step

I am using a CPU with 32 threads, so each boid needs to then be simulated in: (7*32) = 224 nanoseconds. In reality it needs to be faster because of concurrency overhead. 

Each boid at each time step needs to accumulate positions and velocities of other nearby particles. This requires 3 spatial searches to be performed by each boid:
- Seperation: 
	Turn away from the average position of neighbors within a range. vel = distance * strength + vel.
- Cohesion: 
	Turn towards the average position of neighbors within a range. vel = distance * strength + vel
- Alignment: 
	Copy the average velocity of neighbors within a range. lerp(avg_vel, vel, strength)

Eventually these searches will be combined for a possible 50% speedup.

A brute force search on 1 Million particles would require 1 Trillion comparisons, which is too many. So boids need to be sorted by position in order to save time when searching neighbors.  With concurrent simulation all CPU threads need to work on overlapping data, in sequential stages. Memory reads and writes need to happen in a specific sequence depending on physical location of a CPU core. If this is done wrong the simulation would end up running slower and slower as more threads are used. This is because CPU caches need to ensure that all threads access current data. So when a thread writes to a cache line (64 byte section), that cache line will invalidate that entire cache line at all levels of the cache, including caches on other cores. Given all of this, a multi-threaded boids simulation needs to be designed carefully. 

This is the 3rd iteration of this type of simulation that I have made.

All 3 Revision have used a fixed-grid spatial partining approach. Revision 2 and 3 use integer coordinates, or conversion, and a bitshift to find the grid cells really fast. Still the more significant issue is cache-efficiency rather than instruction latency.

1.) Floating-Point Coordinates
- float-32 position and velocity
- Started simple but fell short when multhreaded.

2.) Limited Floating-Point Coordinates
- float-32 velocity 
- float-32 position (limited)
- If a 32-bit floating point value is between 1.0 and 2.0 the exponent bits will all be the same. This means that the mantissa can be treated as a flat 23-bit unsigned integer.
- 8 Million possible positions on each axis, and the floating point positions can be treated like integers so bit-shift can maybe help with power of two division, modulus, and multiplication
- Useful Integer overflow at boundaries. 

3.) Floating-point and integer.
- The overhead of converting between integer and floating point coodinates is trivial and the loss in precision does not effect the simulation in a significant way.
- Integer overflow at boundries.

The interactions of boids can be very simple, so full real numbers are not always required, although they are useful.
	
## Status

Right now the simulation can reasonably handle 500 Thousand boids on a Ryzen 9 7950x (16c,32t). In the near future the following changes will be made.

1.) Simplify boids pipeline
- Remove 2 stages. It is best not to touch the data more than once if possible. 
- Remove Count stage and put at the end of both the Reset and Resolve stages.
- Construct stage is not needed for now.
- Only perform one addition (besides loop) per itteration of Alloc stage. Move some instructions to multithreaded parts of code.
- Possibly remove the switch statement from main update thread loop. Inline all of the stages, once there are fewer of them.

2.) Resolve Stage
- Operate on boid array rather than boids in cell array. (change again)
- Unroll the entire Reosolve stage.
- SIMD int to float conRevision
- SIMD optimization of distance calculation.
- Compact vectors (_mm512_mask_compress_ps) and remove duplicate comparisons
- Make branchless at the cost of a few extra distance comparisons.
- Compress data from 32-bit to 16-bit integer coordinates before transfer to GPU.
	
3.) GUI
- Make automatic fixed layout calculations.
- Drag and drop elements.
- Save slider state of boids and interpolate between them.


Long Term:
- More precise control of thread count (1) and boid count (1).
- Add more intellegence to boids and conditional searches to be scheduled for next update.


## Project Modules

- Boids
- Camera
- Rendering: boids, overlay, vertex buffers.
- Small GUI
- Window and Events
- Utility: Memory allocation, custom printf.


# Pictures

## First Revision
		
<img width="1770" height="1438" alt="screenshot-20250708-140338" src="https://github.com/user-attachments/assets/bf3246e7-aa8a-4d68-b848-81e38d0fcf62" />
<img width="1811" height="1440" alt="screenshot-20250708-183427" src="https://github.com/user-attachments/assets/d442fdc8-fb7f-4fc3-be4a-14901edb6fc6" />
<img width="1301" height="1307" alt="screenshot-20250807-231941" src="https://github.com/user-attachments/assets/0f6d4f47-ecfb-45cb-843b-10deeff0c374" />

## Second Revision

<img width="2561" height="1441" alt="screenshot-20251111-153905" src="https://github.com/user-attachments/assets/efae42c1-bc5f-4f05-bd05-1847ee5e115f" />

## Third Revision

<img width="2552" height="1440" alt="screenshot-20251111-154259" src="https://github.com/user-attachments/assets/e0d1f052-333f-46db-803e-72120e87ea87" />
<img width="860" height="1131" alt="screenshot-20251118-182015" src="https://github.com/user-attachments/assets/44b7f30d-939c-4e35-bc4e-04715e15b9d3" />
<img width="2560" height="1441" alt="screenshot-20251118-182738" src="https://github.com/user-attachments/assets/3b5d53b2-5802-4aee-819f-a93262cbe899" />
<img width="1246" height="1418" alt="screenshot-20251212-172511" src="https://github.com/user-attachments/assets/46ddd5a4-a239-4b0d-8f2e-4d9489352b10" />
