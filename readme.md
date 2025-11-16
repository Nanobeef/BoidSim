


# CPU Boid Simulation

## About

The goal of this project is to simulate as many boid particles as possible in real-time. 

A specific milestone was to simulate a time step with 1 Million particles in under 7 milliseconds (1/144fps), and to do so with reasonable search ranges.
To break that down:
- (0.007 seconds) / (1,000,000 particles) = Average 7 nanoseconds per particle per time step

I am using a CPU with 32 threads, so each boid needs to then be simulated in: (7*32) = 224 nanoseconds. In reality it needs to be faster because of concurrency overhead.

Each boid at each time step needs to accumulate positions and velocities of other nearby particles. This requires 3 spatial searches to be performed by each boid:
- Seperation: 
	Turn away from the average position of neighbors within a range.
- Cohesion: 
	Turn towards the average position of neighbors within a range.
- Alignment: 
	Copy the average velocity of neighbors within a range.

A brute force search on 1 Million particles would require 1 Trillion comparisons, which is too many. So boids need to be sorted by position in order to save time when searching neighbors.  With concurrent simulation all CPU threads need to work on overlapping data, in sequential stages. Memory reads and writes need to happen in a specific sequence depending on physical location of a CPU core. If this is done wrong the simulation would end up running slower and slower as more threads are used. This is because CPU caches need to ensure that all threads access current data. So when a thread writes to a cache line (64 byte section), that cache line will invalidate that entire cache line at all levels of the cache, including caches on other cores. Given all of this, a multi-threaded boids simulation needs to be designed carefully. 


This is the 3rd iteration of this type of simulation that I have made.

1.) Floating-Point Coordinates
- float-32 position and velocity

2.) Limited Floating-Point Coordinates
- float-32 velocity 
- float-32 position (limited)
-- If a 32-bit floating point value is between 1.0 and 2.0 the exponent bits will all be the same. This means that the mantissa can be treated as a flat 23-bit unsigned integer.
-- 8 Million possible positions on each axis, and the floating point positions can be treated like integers so bit-shift can maybe help with power of two division, modulus, and multiplication
-- Useful Integer overflow at boundaries. 

3.) Integer Coordinates
- signed-32 velocity
- unsigned-32 position


The interactions of boids can be very simple, so full real numbers are not always required, although they are useful.

The most complete and fastest simulation uses integer coordinates. This is because many values and constants could be easily aligned to a power of two.


	
...

## Project Modules

- Boids
- Camera
- Rendering: boids, overlay, vertex buffers.
- Small GUI
- Window and Events
- Utility: Memory allocation, custom printf.





# Pictures
		
		
<img width="1770" height="1438" alt="screenshot-20250708-140338" src="https://github.com/user-attachments/assets/bf3246e7-aa8a-4d68-b848-81e38d0fcf62" />

	


	


